#include "vulkan_internal.h"

/*
 */
static OPAL_INLINE uint16_t vulkan_granularityPagePack(uint32_t resource_type, uint32_t usage_count)
{
	uint16_t mask = 0x03FF;
	return (uint16_t)((resource_type << 10) | (usage_count & mask));
}

static OPAL_INLINE uint32_t vulkan_granularityPageGetResourceType(uint16_t page)
{
	return (uint32_t)(page >> 10);
}

static OPAL_INLINE uint32_t vulkan_granularityPageGetUsageCount(uint16_t page)
{
	return (uint32_t)(page & 0x03FF);
}

static OPAL_INLINE uint32_t vulkan_granularityPageIsValidResource(uint32_t page, uint32_t resource_type)
{
	uint32_t page_resource_type = vulkan_granularityPageGetResourceType(page);

	return page_resource_type == 0 || page_resource_type == resource_type;
}

static OPAL_INLINE uint32_t vulkan_granularityPageIncrementUsage(uint32_t page, uint32_t resource_type)
{
	uint32_t page_resource_type = vulkan_granularityPageGetResourceType(page);
	uint32_t page_usage_count = vulkan_granularityPageGetUsageCount(page);

	assert(page_resource_type == 0 || page_resource_type == resource_type);

	return vulkan_granularityPagePack(resource_type, page_usage_count + 1);
}

static OPAL_INLINE uint32_t vulkan_granularityPageDecrementUsage(uint32_t page)
{
	uint32_t page_resource_type = vulkan_granularityPageGetResourceType(page);
	uint32_t page_usage_count = vulkan_granularityPageGetUsageCount(page);

	assert(page_resource_type != 0);
	assert(page_usage_count > 0);

	if (page_usage_count == 1)
		page_resource_type = 0;

	return vulkan_granularityPagePack(page_resource_type, page_usage_count - 1);
}

/*
 */
static Opal_Result vulkan_allocatorStageHeapAlloc(const Vulkan_Allocator *allocator, uint32_t heap_id, uint32_t size, uint32_t alignment, uint32_t resource_type, Opal_NodeIndex *node_index, uint32_t *offset)
{
	assert(allocator);
	assert(heap_id < allocator->num_heaps);
	assert(heap_id != OPAL_VULKAN_HEAP_NULL);

	assert(node_index);
	assert(offset);
	assert(size > 0);
	assert(resource_type > 0);
	assert(isPow2(alignment));

	const Vulkan_MemoryHeap *heap = &allocator->heaps[heap_id];
	assert(heap);

	if (allocator->buffer_image_granularity == 1)
		return opal_heapStageAllocAligned(&heap->heap, size, alignment, node_index, offset);

	assert(heap->granularity_pages);

	uint32_t extra_size = (uint32_t)(allocator->buffer_image_granularity - 1);
	uint32_t wanted_size = size;

	for (uint32_t i = 0; i < 3; ++i, wanted_size += extra_size)
	{
		Opal_Result result = opal_heapStageAllocAligned(&heap->heap, wanted_size, alignment, node_index, offset);
		if (result != OPAL_SUCCESS)
			return result;

		assert(*node_index != OPAL_NODE_INDEX_NULL);

		uint32_t page_begin_index = alignDown(*offset, allocator->buffer_image_granularity) / allocator->buffer_image_granularity;
		uint32_t page_begin = heap->granularity_pages[page_begin_index];

		if (vulkan_granularityPageIsValidResource(page_begin, resource_type) == 0)
			*offset = alignUp(*offset, allocator->buffer_image_granularity);

		uint32_t page_end_index = alignUp(*offset + size, allocator->buffer_image_granularity) / allocator->buffer_image_granularity - 1;
		uint32_t page_end = heap->granularity_pages[page_end_index];

		assert(page_end_index >= page_begin_index);

		if (vulkan_granularityPageIsValidResource(page_end, resource_type) == 0)
			continue;

		Opal_HeapNode *node = &heap->heap.nodes[*node_index];
		assert(node);
		assert(node->used == 0);

		uint32_t remainder_begin_size = *offset - node->offset;
		if (remainder_begin_size + size > node->size)
			continue;

		break;
	}

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_allocatorCommitHeapAlloc(Vulkan_Allocator *allocator, uint32_t heap_id, Opal_NodeIndex node_index, uint32_t offset, uint32_t size, uint32_t resource_type)
{
	assert(allocator);
	assert(heap_id < allocator->num_heaps);
	assert(heap_id != OPAL_VULKAN_HEAP_NULL);

	assert(node_index != OPAL_NODE_INDEX_NULL);
	assert(size > 0);
	assert(resource_type > 0);

	Vulkan_MemoryHeap *heap = &allocator->heaps[heap_id];
	assert(heap);

	Opal_Result result = opal_heapCommitAlloc(&heap->heap, node_index, offset, size);
	assert(result == OPAL_SUCCESS);

	if (allocator->buffer_image_granularity > 1)
	{
		assert(heap->granularity_pages);

		uint32_t page_begin_index = alignDown(offset, allocator->buffer_image_granularity) / allocator->buffer_image_granularity;
		uint32_t page_begin = heap->granularity_pages[page_begin_index];

		uint32_t page_end_index = alignUp(offset + size, allocator->buffer_image_granularity) / allocator->buffer_image_granularity - 1;
		uint32_t page_end = heap->granularity_pages[page_end_index];

		assert(page_end_index >= page_begin_index);

		heap->granularity_pages[page_begin_index] = vulkan_granularityPageIncrementUsage(page_begin, resource_type);
		heap->granularity_pages[page_end_index] = vulkan_granularityPageIncrementUsage(page_end, resource_type);
	}

	return result;
}

static Opal_Result vulkan_allocatorFreeHeapAlloc(Vulkan_Allocator *allocator, uint32_t heap_id, Opal_NodeIndex node_index, uint32_t offset)
{
	assert(allocator);
	assert(heap_id < allocator->num_heaps);
	assert(heap_id != OPAL_VULKAN_HEAP_NULL);

	assert(node_index != OPAL_NODE_INDEX_NULL);

	Vulkan_MemoryHeap *heap = &allocator->heaps[heap_id];
	assert(heap);

	Opal_HeapAllocation heap_allocation = {0};
	heap_allocation.offset = offset;
	heap_allocation.metadata = node_index;

	if (allocator->buffer_image_granularity > 1)
	{
		assert(heap->granularity_pages);

		Opal_HeapNode *node = &heap->heap.nodes[node_index];
		assert(node);
		assert(node->used != 0);

		uint32_t page_begin_index = alignDown(offset, allocator->buffer_image_granularity) / allocator->buffer_image_granularity;
		uint32_t page_begin = heap->granularity_pages[page_begin_index];

		uint32_t page_end_index = alignUp(offset + node->size, allocator->buffer_image_granularity) / allocator->buffer_image_granularity - 1;
		uint32_t page_end = heap->granularity_pages[page_end_index];

		assert(page_end_index >= page_begin_index);

		heap->granularity_pages[page_begin_index] = vulkan_granularityPageDecrementUsage(page_begin);
		heap->granularity_pages[page_end_index] = vulkan_granularityPageDecrementUsage(page_end);
	}

	return opal_heapFree(&heap->heap, heap_allocation);
}

/*
 */
Opal_Result vulkan_allocatorInitialize(Vulkan_Allocator *allocator, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps, VkDeviceSize buffer_image_granularity)
{
	assert(allocator);

	memset(allocator, 0, sizeof(Vulkan_Allocator));

	allocator->heaps = (Vulkan_MemoryHeap *)malloc(sizeof(Vulkan_MemoryHeap) * max_heaps);
	allocator->num_heaps = 0;
	allocator->heap_size = heap_size;
	allocator->max_heap_allocations = max_heap_allocations;
	allocator->max_heaps = max_heaps;
	allocator->buffer_image_granularity = buffer_image_granularity;

	memset(allocator->heaps, 0, sizeof(Vulkan_MemoryHeap) * max_heaps);
	memset(allocator->first_heap, OPAL_VULKAN_HEAP_NULL, sizeof(uint32_t) * VK_MAX_MEMORY_TYPES);
	memset(allocator->last_used_heaps, OPAL_VULKAN_HEAP_NULL, sizeof(uint32_t) * VK_MAX_MEMORY_TYPES);

	opal_poolInitialize(&allocator->blocks, sizeof(Vulkan_MemoryBlock), 0);

	return OPAL_SUCCESS;
}

Opal_Result vulkan_allocatorShutdown(Vulkan_Allocator *allocator, VkDevice device)
{
	assert(allocator);
	assert(allocator->heaps);

	for (uint32_t i = 0; i < allocator->num_heaps; ++i)
	{
		Opal_Heap *heap = &allocator->heaps[i].heap;
		opal_heapShutdown(heap);

		free(allocator->heaps[i].granularity_pages);
	}
	free(allocator->heaps);

	Vulkan_MemoryBlock *blocks_ptr = (Vulkan_MemoryBlock *)allocator->blocks.data;
	for (uint32_t i = 0; i < allocator->blocks.size; ++i)
	{
		VkDeviceMemory memory = blocks_ptr[i].memory;
		vkFreeMemory(device, memory, NULL);
	}
	opal_poolShutdown(&allocator->blocks);

	return OPAL_SUCCESS;
}

/*
 */
Opal_Result vulkan_allocatorAllocateMemory(Vulkan_Allocator *allocator, VkDevice device, VkDeviceSize size, VkDeviceSize alignment, VkDeviceSize budget, VkDeviceSize usage, uint32_t resource_type, uint32_t memory_type, uint32_t dedicated, Vulkan_Allocation *allocation)
{
	assert(allocator);
	assert(device != VK_NULL_HANDLE);
	assert(size > 0);
	assert(memory_type < VK_MAX_MEMORY_TYPES);
	assert(allocation);

	// create dedicated block
	if (dedicated)
	{
		if (usage + size > budget)
			return OPAL_NO_MEMORY;

		Vulkan_MemoryBlock block = {0};
		block.size = size;
		block.heap = OPAL_VULKAN_HEAP_NULL;
		block.memory_type = memory_type;

		VkMemoryAllocateInfo info = {0};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.memoryTypeIndex = memory_type;
		info.allocationSize = block.size;

		VkResult result = vkAllocateMemory(device, &info, NULL, &block.memory);
		if (result != VK_SUCCESS)
			return OPAL_NO_MEMORY;

		Opal_PoolHandle handle = opal_poolAddElement(&allocator->blocks, &block);

		allocation->block = handle;
		allocation->memory = block.memory;
		allocation->offset = 0;
		allocation->heap_metadata = OPAL_NODE_INDEX_NULL;

		return OPAL_SUCCESS;
	}

	// find or create heap block
	uint32_t heap_id = allocator->last_used_heaps[memory_type];
	Opal_PoolHandle block_handle = OPAL_POOL_HANDLE_NULL;
	Opal_NodeIndex node_index = OPAL_NODE_INDEX_NULL;
	uint32_t offset = 0;

	if (heap_id != OPAL_VULKAN_HEAP_NULL)
	{
		Vulkan_MemoryHeap *heap = &allocator->heaps[heap_id];
		block_handle = heap->block;

		Opal_Result opal_result = vulkan_allocatorStageHeapAlloc(allocator, heap_id, size, alignment, resource_type, &node_index, &offset);
		if (opal_result != OPAL_SUCCESS)
		{
			heap_id = OPAL_VULKAN_HEAP_NULL;
			block_handle = OPAL_POOL_HANDLE_NULL;
		}
	}

	if (heap_id == OPAL_VULKAN_HEAP_NULL)
	{
		uint32_t pending_heap_id = allocator->first_heap[memory_type];
		while (pending_heap_id != OPAL_VULKAN_HEAP_NULL)
		{
			Vulkan_MemoryHeap *heap = &allocator->heaps[pending_heap_id];

			Opal_Result opal_result = vulkan_allocatorStageHeapAlloc(allocator, heap_id, size, alignment, resource_type, &node_index, &offset);
			if (opal_result == OPAL_SUCCESS)
			{
				heap_id = pending_heap_id;
				block_handle = heap->block;
				break;
			}

			pending_heap_id = heap->next_heap;
		}
	}

	if (allocator->num_heaps == allocator->max_heaps)
		return OPAL_NO_MEMORY;

	if (heap_id == OPAL_VULKAN_HEAP_NULL)
	{
		if (usage + allocator->heap_size > budget)
			return OPAL_NO_MEMORY;

		heap_id = allocator->num_heaps;

		Vulkan_MemoryBlock block = {0};
		block.size = allocator->heap_size;
		block.heap = heap_id;
		block.memory_type = memory_type;

		VkMemoryAllocateInfo info = {0};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.memoryTypeIndex = memory_type;
		info.allocationSize = allocator->heap_size;

		VkResult result = vkAllocateMemory(device, &info, NULL, &block.memory);
		if (result != VK_SUCCESS)
			return OPAL_NO_MEMORY;

		block_handle = opal_poolAddElement(&allocator->blocks, &block);

		Vulkan_MemoryHeap *heap = &allocator->heaps[heap_id];
		opal_heapInitialize(&heap->heap, allocator->heap_size, allocator->max_heap_allocations);

		if (allocator->buffer_image_granularity > 1)
		{
			uint32_t num_pages = alignUp(allocator->heap_size, allocator->buffer_image_granularity) / allocator->buffer_image_granularity;

			heap->granularity_pages = (uint16_t *)malloc(sizeof(uint16_t) * num_pages);
			memset(heap->granularity_pages, 0, sizeof(uint16_t) * num_pages);
		}

		heap->next_heap = allocator->first_heap[memory_type];
		heap->block = block_handle;

		allocator->first_heap[memory_type] = heap_id;
		allocator->num_heaps++;

		assert(size <= allocator->heap_size);

		Opal_Result opal_result = vulkan_allocatorStageHeapAlloc(allocator, heap_id, size, alignment, resource_type, &node_index, &offset);
		assert(opal_result == OPAL_SUCCESS);
	}

	// heap alloc VRAM
	Vulkan_MemoryBlock *block = opal_poolGetElement(&allocator->blocks, block_handle);
	assert(block);

	Opal_Result result = vulkan_allocatorCommitHeapAlloc(allocator, heap_id, node_index, offset, size, resource_type);
	assert(result == OPAL_SUCCESS);

	allocation->block = block_handle;
	allocation->memory = block->memory;
	allocation->offset = offset;
	allocation->heap_metadata = node_index;

	allocator->last_used_heaps[memory_type] = heap_id;

	return OPAL_SUCCESS;
}

Opal_Result vulkan_allocatorMapMemory(Vulkan_Allocator *allocator, VkDevice device, Vulkan_Allocation allocation, void **ptr)
{
	assert(allocator);
	assert(allocation.block != OPAL_POOL_HANDLE_NULL);

	Vulkan_MemoryBlock *block = opal_poolGetElement(&allocator->blocks, allocation.block);
	assert(block);

	if (block->map_count == 0)
	{
		VkResult result = vkMapMemory(device, block->memory, 0, block->size, 0, &block->mapped_ptr);
		if (result != VK_SUCCESS)
			return OPAL_VULKAN_ERROR;
	}

	block->map_count++;
	*ptr = block->mapped_ptr;

	return OPAL_SUCCESS;
}

Opal_Result vulkan_allocatorUnmapMemory(Vulkan_Allocator *allocator, VkDevice device, Vulkan_Allocation allocation)
{
	assert(allocator);
	assert(allocation.block != OPAL_POOL_HANDLE_NULL);

	Vulkan_MemoryBlock *block = opal_poolGetElement(&allocator->blocks, allocation.block);
	assert(block);
	assert(block->map_count != 0);
	assert(block->mapped_ptr != NULL);

	block->map_count--;
	if (block->map_count == 0)
	{
		vkUnmapMemory(device, block->memory);
		block->mapped_ptr = NULL;
	}

	return OPAL_SUCCESS;
}

Opal_Result vulkan_allocatorFreeMemory(Vulkan_Allocator *allocator, VkDevice device, Vulkan_Allocation allocation)
{
	assert(allocator);
	assert(allocation.block != OPAL_POOL_HANDLE_NULL);

	Vulkan_MemoryBlock *block = opal_poolGetElement(&allocator->blocks, allocation.block);
	assert(block);

	if (block->heap != OPAL_VULKAN_HEAP_NULL)
	{
		allocator->last_used_heaps[block->memory_type] = block->heap;
		return vulkan_allocatorFreeHeapAlloc(allocator, block->heap, allocation.heap_metadata, allocation.offset);
	}

	vkFreeMemory(device, block->memory, NULL);
	return opal_poolRemoveElement(&allocator->blocks, allocation.block);
}
