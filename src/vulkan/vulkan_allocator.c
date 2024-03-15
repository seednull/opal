#include "vulkan_internal.h"

/*
 */
Opal_Result vulkan_allocatorInitialize(Vulkan_Allocator *allocator, uint32_t memory_type, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps)
{
	assert(allocator);

	allocator->heaps = (Vulkan_MemoryHeap *)malloc(sizeof(Vulkan_MemoryHeap) * max_heaps);
	allocator->last_used_heap = -1;
	allocator->num_heaps = 0;
	allocator->heap_size = heap_size;
	allocator->max_heap_allocations = max_heap_allocations;
	allocator->max_heaps = max_heaps;
	allocator->memory_type = memory_type;

	return OPAL_SUCCESS;
}

Opal_Result vulkan_allocatorShutdown(Vulkan_Allocator *allocator, VkDevice device)
{
	assert(allocator);
	assert(allocator->heaps);

	for (uint32_t i = 0; i < allocator->num_heaps; ++i)
	{
		VkDeviceMemory memory = allocator->heaps[i].memory;
		Opal_Heap *heap = &allocator->heaps[i].heap;

		vkFreeMemory(device, memory, NULL);
		opal_heapShutdown(heap);
	}

	free(allocator->heaps);

	return OPAL_SUCCESS;
}

/*
 */
Opal_Result vulkan_allocatorAllocateMemory(Vulkan_Allocator *allocator, VkDevice device, VkDeviceSize size, VkDeviceSize alignment, Vulkan_Allocation *allocation)
{
	assert(allocator);
	assert(device != VK_NULL_HANDLE);
	assert(size > 0);
	assert(allocation);

	// TODO: create dedicated allocations
	if (size > allocator->heap_size)
		return OPAL_NO_MEMORY;

	uint32_t suballocation_size = (uint32_t)size;
	uint32_t suballocation_alignment = (uint32_t)alignment;

	int32_t heap_index = -1;

	if (allocator->last_used_heap >= 0 && allocator->last_used_heap < allocator->num_heaps)
	{
		uint32_t index = (uint32_t)allocator->last_used_heap;
		Opal_Heap *heap = &allocator->heaps[index].heap;

		if (opal_heapCanAllocAligned(heap, suballocation_size, suballocation_alignment))
			heap_index = index;
	}

	if (heap_index == -1 && allocator->num_heaps > 0)
	{
		for (uint32_t i = allocator->num_heaps - 1; i >= 0; ++i)
		{
			Opal_Heap *heap = &allocator->heaps[i].heap;

			if (opal_heapCanAllocAligned(heap, suballocation_size, suballocation_alignment))
			{
				heap_index = i;
				break;
			}
		}
	}

	if (heap_index == -1)
	{
		if (allocator->num_heaps == allocator->max_heaps)
			return OPAL_NO_MEMORY;

		VkDeviceMemory memory = VK_NULL_HANDLE;

		VkMemoryAllocateInfo info = {0};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.memoryTypeIndex = allocator->memory_type;
		info.allocationSize = allocator->heap_size;

		VkResult result = vkAllocateMemory(device, &info, NULL, &memory);
		if (result != VK_SUCCESS)
			return OPAL_NO_MEMORY;

		allocator->num_heaps++;
		heap_index = allocator->num_heaps - 1;

		Opal_Result opal_result = opal_heapInitialize(&allocator->heaps[heap_index].heap, allocator->heap_size, allocator->max_heap_allocations);
		assert(opal_result == VK_SUCCESS);

		allocator->heaps[heap_index].memory = memory;
	}

	assert(heap_index >= 0 && heap_index < allocator->num_heaps);

	VkDeviceMemory memory = allocator->heaps[heap_index].memory;
	Opal_Heap *heap = &allocator->heaps[heap_index].heap;
	Opal_HeapAllocation heap_allocation = {0};

	Opal_Result opal_result = opal_heapAllocAligned(heap, suballocation_size, suballocation_alignment, &heap_allocation);
	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	allocation->memory = memory;
	allocation->index = heap_index;
	allocation->offset = heap_allocation.offset;
	allocation->metadata = heap_allocation.metadata;

	allocator->last_used_heap = heap_index;

	return OPAL_SUCCESS;
}

Opal_Result vulkan_allocatorFreeMemory(Vulkan_Allocator *allocator, Vulkan_Allocation allocation)
{
	assert(allocator);
	assert(allocation.index < allocator->num_heaps);

	Opal_Heap *heap = &allocator->heaps[allocation.index].heap;
	Opal_HeapAllocation heap_allocation = {0};
	heap_allocation.offset = allocation.offset;
	heap_allocation.metadata = allocation.metadata;

	return opal_heapFree(heap, heap_allocation);
}
