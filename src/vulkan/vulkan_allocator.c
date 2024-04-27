#include "vulkan_internal.h"

/*
 */
static Opal_Result vulkan_allocatorStageAllocationInternal(const Vulkan_Allocator *allocator, uint32_t heap_id, uint32_t size, uint32_t alignment, uint32_t resource_type, Opal_NodeIndex *node_index, uint32_t *offset)
{
	assert(allocator);
	assert(heap_id < allocator->num_heaps);

	assert(node_index);
	assert(offset);
	assert(size > 0);
	assert(isPow2(alignment));

	const Vulkan_MemoryHeap *heap = &allocator->heaps[heap_id];
	assert(heap);

	Opal_Result opal_result = opal_heapStageAllocAligned(&heap->heap, size, alignment, node_index, offset);

	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	if (allocator->buffer_image_granularity == 1)
		return OPAL_SUCCESS;

	// TODO: check page conflicts
	// if (isPageConflict(heap, *offset, resource_type) == 1)
	// 	*offset = alignUp(*offset, allocator->buffer_image_granularity);
	
	// if (isPageConflict(heap, *offset + size, resource_type) == 1)
	// 	return OPAL_NO_MEMORY;

	assert(*node_index != OPAL_NODE_INDEX_NULL);
	Opal_HeapNode *node = &heap->heap.nodes[*node_index];

	assert(node);
	uint32_t remainder_begin_size = *offset - node->offset;
	if (remainder_begin_size + size > node->size)
		return OPAL_NO_MEMORY;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_allocatorStageAllocation(const Vulkan_Allocator *allocator, uint32_t heap_id, uint32_t size, uint32_t alignment, uint32_t resource_type, Opal_NodeIndex *node_index, uint32_t *offset)
{
	assert(allocator);
	assert(heap_id < allocator->num_heaps);

	assert(node_index);
	assert(offset);
	assert(size > 0);
	assert(isPow2(alignment));

	Opal_Result opal_result = vulkan_allocatorStageAllocationInternal(allocator, heap_id, size, alignment, resource_type, node_index, offset);
	if (opal_result == OPAL_SUCCESS)
		return OPAL_SUCCESS;

	size += allocator->buffer_image_granularity; // is this the worst case?
	return vulkan_allocatorStageAllocationInternal(allocator, heap_id, size, alignment, resource_type, node_index, offset);
}

static Opal_Result vulkan_allocatorCommitAllocation(Vulkan_Allocator *allocator, uint32_t heap_id, Opal_NodeIndex node_index, uint32_t offset, uint32_t size, uint32_t resource_type)
{
	assert(allocator);
	assert(heap_id < allocator->num_heaps);

	Vulkan_MemoryHeap *heap = &allocator->heaps[heap_id];
	assert(heap);

	Opal_Result result = opal_heapCommitAlloc(&heap->heap, node_index, offset, size);
	assert(result == OPAL_SUCCESS);

	// TODO: mark pages

	return result;
}

/*
 */
Opal_Result vulkan_allocatorInitialize(Vulkan_Allocator *allocator, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps, uint32_t buffer_image_granularity)
{
	assert(allocator);

	memset(allocator, 0, sizeof(Vulkan_Allocator));

	allocator->heaps = (Vulkan_MemoryHeap *)malloc(sizeof(Vulkan_MemoryHeap) * max_heaps);
	allocator->num_heaps = 0;
	allocator->heap_size = heap_size;
	allocator->max_heap_allocations = max_heap_allocations;
	allocator->max_heaps = max_heaps;
	allocator->buffer_image_granularity = buffer_image_granularity;

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
Opal_Result vulkan_allocatorAllocateMemory(Vulkan_Allocator *allocator, VkDevice device, VkDeviceSize size, VkDeviceSize alignment, uint32_t memory_type, uint32_t dedicated, Vulkan_Allocation *allocation)
{
	assert(allocator);
	assert(device != VK_NULL_HANDLE);
	assert(size > 0);
	assert(memory_type < VK_MAX_MEMORY_TYPES);
	assert(allocation);

	// TODO: check budgets

	// create dedicated block
	if (dedicated)
	{
		Vulkan_MemoryBlock block = {0};
		block.size = size;
		block.heap = OPAL_VULKAN_HEAP_NULL;

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

		// TODO: update budgets

		return OPAL_SUCCESS;
	}

	// TODO: check if allocation can fit in heap

	// find or create heap block
	uint32_t heap_id = allocator->last_used_heaps[memory_type];
	Opal_PoolHandle block_handle = OPAL_POOL_HANDLE_NULL;
	Opal_NodeIndex node_index = OPAL_NODE_INDEX_NULL;
	uint32_t offset = 0;
	uint32_t resource_type = 0;

	if (heap_id != OPAL_VULKAN_HEAP_NULL)
	{
		Vulkan_MemoryHeap *heap = &allocator->heaps[heap_id];
		block_handle = heap->block;

		Opal_Result opal_result = vulkan_allocatorStageAllocation(allocator, heap_id, size, alignment, resource_type, &node_index, &offset);
		if (opal_result != OPAL_SUCCESS)
		{
			heap_id = OPAL_VULKAN_HEAP_NULL;
			block_handle = OPAL_POOL_HANDLE_NULL;
		}
	}

	uint32_t pending_heap_id = allocator->first_heap[memory_type];
	while (pending_heap_id != OPAL_VULKAN_HEAP_NULL)
	{
		Vulkan_MemoryHeap *heap = &allocator->heaps[pending_heap_id];

		Opal_Result opal_result = vulkan_allocatorStageAllocation(allocator, heap_id, size, alignment, resource_type, &node_index, &offset);
		if (opal_result == OPAL_SUCCESS)
		{
			heap_id = pending_heap_id;
			block_handle = heap->block;
			break;
		}

		pending_heap_id = heap->next_heap;
	}
	
	if (heap_id == OPAL_VULKAN_HEAP_NULL)
	{
		heap_id = allocator->num_heaps++;

		Vulkan_MemoryBlock block = {0};
		block.size = allocator->heap_size;
		block.heap = heap_id;

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

		uint32_t first_heap_id = allocator->first_heap[memory_type];
		if (first_heap_id != OPAL_VULKAN_HEAP_NULL)
		{
			Vulkan_MemoryHeap *first_heap = &allocator->heaps[first_heap_id];
			assert(first_heap);

			first_heap->prev_heap = heap_id;
		}

		heap->prev_heap = OPAL_VULKAN_HEAP_NULL;
		heap->next_heap = first_heap_id;
		heap->block = block_handle;

		allocator->first_heap[memory_type] = heap_id;

		Opal_Result opal_result = vulkan_allocatorStageAllocation(allocator, heap_id, size, alignment, resource_type, &node_index, &offset);
		assert(opal_result == OPAL_SUCCESS);
	}

	// heap alloc VRAM
	Vulkan_MemoryBlock *block = opal_poolGetElement(&allocator->blocks, block_handle);
	assert(block);

	Opal_Result result = vulkan_allocatorCommitAllocation(allocator, heap_id, node_index, offset, size, resource_type);
	assert(result == OPAL_SUCCESS);

	allocation->block = block_handle;
	allocation->memory = block->memory;
	allocation->offset = offset;
	allocation->heap_metadata = node_index;

	allocator->last_used_heaps[memory_type] = heap_id;

	// TODO: update budgets

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

	Vulkan_MemoryHeap *heap = NULL;

	if (block->heap != OPAL_VULKAN_HEAP_NULL)
		heap = &allocator->heaps[block->heap];

	if (heap)
	{
		Opal_HeapAllocation heap_allocation = {0};
		heap_allocation.offset = allocation.offset;
		heap_allocation.metadata = allocation.heap_metadata;

		return opal_heapFree(&heap->heap, heap_allocation);
	}

	vkFreeMemory(device, block->memory, NULL);
	return opal_poolRemoveElement(&allocator->blocks, allocation.block);
}
