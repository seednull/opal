#include "metal_internal.h"

#include "common/intrinsics.h"

#include <stdlib.h>
#include <string.h>

#define OPAL_HEAP_NULL 0xFFFFFFFF

/*
 */
static MTLStorageMode metal_storage_modes[METAL_MAX_MEMORY_TYPES] =
{
	MTLStorageModePrivate,
	MTLStorageModeShared,
	MTLStorageModeShared,
};

static MTLCPUCacheMode metal_cpu_cache_modes[METAL_MAX_MEMORY_TYPES] =
{
	MTLCPUCacheModeDefaultCache,
	MTLCPUCacheModeWriteCombined,
	MTLCPUCacheModeDefaultCache,
};

/*
 */
uint32_t metal_getMemoryType(Opal_AllocationMemoryType allocation_type)
{
	switch (allocation_type)
	{
		case OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL: return 0;
		case OPAL_ALLOCATION_MEMORY_TYPE_STREAM: return 1;
		case OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD:
		case OPAL_ALLOCATION_MEMORY_TYPE_READBACK: return 2;

		default: assert(false); return 0xFFFFFFFF;
	}
}

/*
 */
static Opal_Result metal_allocatorStageHeapAlloc(const Metal_Allocator *allocator, uint32_t heap_id, const Metal_AllocationDesc *desc, Opal_NodeIndex *node_index, uint32_t *offset)
{
	assert(allocator);
	assert(desc);
	assert(heap_id < allocator->num_heaps);
	assert(heap_id != OPAL_HEAP_NULL);

	assert(node_index);
	assert(offset);

	uint32_t size = (uint32_t)desc->size;
	assert(size > 0);
	assert(size <= allocator->heap_size);

	uint32_t alignment = (uint32_t)desc->alignment;
	assert(alignment <= allocator->heap_size);
	assert(isPow2ul(alignment));

	const Metal_MemoryHeap *heap = &allocator->heaps[heap_id];
	assert(heap);

	return opal_heapStageAllocAligned(&heap->heap, size, alignment, node_index, offset);
}

static Opal_Result metal_allocatorCommitHeapAlloc(Metal_Allocator *allocator, uint32_t heap_id, Opal_NodeIndex node_index, uint32_t offset, const Metal_AllocationDesc *desc)
{
	assert(allocator);
	assert(desc);
	assert(heap_id < allocator->num_heaps);
	assert(heap_id != OPAL_HEAP_NULL);

	assert(node_index != OPAL_NODE_INDEX_NULL);

	uint32_t size = (uint32_t)desc->size;
	assert(size > 0);
	assert(size <= allocator->heap_size);

	uint32_t alignment = (uint32_t)desc->alignment;
	assert(alignment <= allocator->heap_size);
	assert(isPow2ul(alignment));

	Metal_MemoryHeap *heap = &allocator->heaps[heap_id];
	assert(heap);

	return opal_heapCommitAlloc(&heap->heap, node_index, offset, size);
}

static Opal_Result metal_allocatorFreeHeapAlloc(Metal_Allocator *allocator, uint32_t heap_id, Opal_NodeIndex node_index, uint32_t offset)
{
	assert(allocator);
	assert(heap_id < allocator->num_heaps);
	assert(heap_id != OPAL_HEAP_NULL);

	assert(node_index != OPAL_NODE_INDEX_NULL);

	Metal_MemoryHeap *heap = &allocator->heaps[heap_id];
	assert(heap);

	Opal_HeapAllocation heap_allocation = {0};
	heap_allocation.offset = offset;
	heap_allocation.metadata = node_index;

	return opal_heapFree(&heap->heap, heap_allocation);
}

static Opal_Result metal_allocatorBlockAlloc(Metal_Device *device, Opal_AllocationMemoryType allocation_type, uint32_t heap_id, uint64_t size, Opal_PoolHandle *handle)
{
	assert(device);
	assert(size > 0);
	assert(handle);

	uint32_t memory_type = metal_getMemoryType(allocation_type);
	assert(memory_type < METAL_MAX_MEMORY_TYPES);

	Metal_Allocator *allocator = &device->allocator;

	// TODO: budges & usages

	Metal_MemoryBlock block = {0};
	block.size = size;
	block.heap = heap_id;
	block.memory_type = memory_type;

	MTLHeapDescriptor *info = [[MTLHeapDescriptor alloc] init];

	info.type = MTLHeapTypePlacement;
	info.storageMode = metal_storage_modes[memory_type];
	info.cpuCacheMode = metal_cpu_cache_modes[memory_type];
	info.size = block.size;

	block.memory = [device->device newHeapWithDescriptor: info];
	[info release];

	if (!block.memory)
		return OPAL_NO_MEMORY;

	*handle = opal_poolAddElement(&allocator->blocks, &block);
	return OPAL_SUCCESS;
}

/*
 */
Opal_Result metal_allocatorInitialize(Metal_Device *device, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps)
{
	assert(device);

	Metal_Allocator *allocator = &device->allocator;
	memset(allocator, 0, sizeof(Metal_Allocator));

	allocator->heaps = (Metal_MemoryHeap *)malloc(sizeof(Metal_MemoryHeap) * max_heaps);
	allocator->num_heaps = 0;
	allocator->heap_size = heap_size;
	allocator->max_heap_allocations = max_heap_allocations;
	allocator->max_heaps = max_heaps;

	memset(allocator->heaps, 0, sizeof(Metal_MemoryHeap) * max_heaps);
	memset(allocator->first_heap, OPAL_HEAP_NULL, sizeof(uint32_t) * METAL_MAX_MEMORY_TYPES);
	memset(allocator->last_used_heap, OPAL_HEAP_NULL, sizeof(uint32_t) * METAL_MAX_MEMORY_TYPES);

	opal_poolInitialize(&allocator->blocks, sizeof(Metal_MemoryBlock), max_heaps);

	return OPAL_SUCCESS;
}

Opal_Result metal_allocatorShutdown(Metal_Device *device)
{
	assert(device);

	Metal_Allocator *allocator = &device->allocator;
	assert(allocator->heaps);

	for (uint32_t i = 0; i < allocator->num_heaps; ++i)
	{
		Metal_MemoryHeap *heap = &allocator->heaps[i];
		opal_heapShutdown(&heap->heap);
	}
	free(allocator->heaps);

	uint32_t head = opal_poolGetHeadIndex(&allocator->blocks);
	while (head != OPAL_POOL_HANDLE_NULL)
	{
		Metal_MemoryBlock *block = (Metal_MemoryBlock *)opal_poolGetElementByIndex(&allocator->blocks, head);

		block->memory = nil;
		head = opal_poolGetNextIndex(&allocator->blocks, head);
	}

	opal_poolShutdown(&allocator->blocks);

	return OPAL_SUCCESS;
}

/*
 */
Opal_Result metal_allocatorAllocateMemory(Metal_Device *device, const Metal_AllocationDesc *desc, uint32_t dedicated, Metal_Allocation *allocation)
{
	assert(device);
	assert(desc);
	assert(desc->size > 0);
	assert(allocation);

	uint32_t memory_type = metal_getMemoryType(desc->allocation_type);
	assert(memory_type < METAL_MAX_MEMORY_TYPES);

	Metal_Allocator *allocator = &device->allocator;

	// create dedicated block
	if (dedicated)
	{
		Opal_PoolHandle handle = OPAL_POOL_HANDLE_NULL;

		Opal_Result opal_result = metal_allocatorBlockAlloc(device, desc->allocation_type, OPAL_HEAP_NULL, desc->size, &handle);
		if (opal_result != OPAL_SUCCESS)
			return opal_result;

		Metal_MemoryBlock *block = opal_poolGetElement(&allocator->blocks, handle);
		assert(block);

		allocation->block = handle;
		allocation->memory = block->memory;
		allocation->offset = 0;
		allocation->heap_metadata = OPAL_NODE_INDEX_NULL;

		return OPAL_SUCCESS;
	}

	// find or create heap block
	if (desc->size > allocator->heap_size)
		return OPAL_NO_MEMORY;

	uint32_t heap_id = allocator->last_used_heap[memory_type];
	Opal_PoolHandle block_handle = OPAL_POOL_HANDLE_NULL;
	Opal_NodeIndex node_index = OPAL_NODE_INDEX_NULL;
	uint32_t offset = 0;

	if (heap_id != OPAL_HEAP_NULL)
	{
		Metal_MemoryHeap *heap = &allocator->heaps[heap_id];
		block_handle = heap->block;

		Opal_Result opal_result = metal_allocatorStageHeapAlloc(allocator, heap_id, desc, &node_index, &offset);
		if (opal_result != OPAL_SUCCESS)
		{
			heap_id = OPAL_HEAP_NULL;
			block_handle = OPAL_POOL_HANDLE_NULL;
		}
	}

	if (heap_id == OPAL_HEAP_NULL)
	{
		uint32_t pending_heap_id = allocator->first_heap[memory_type];
		while (pending_heap_id != OPAL_HEAP_NULL)
		{
			Metal_MemoryHeap *heap = &allocator->heaps[pending_heap_id];

			Opal_Result opal_result = metal_allocatorStageHeapAlloc(allocator, pending_heap_id, desc, &node_index, &offset);
			if (opal_result == OPAL_SUCCESS)
			{
				heap_id = pending_heap_id;
				block_handle = heap->block;
				break;
			}

			pending_heap_id = heap->next_heap;
		}
	}

	if (heap_id == OPAL_HEAP_NULL)
	{
		if (allocator->num_heaps == allocator->max_heaps)
			return OPAL_NO_MEMORY;

		heap_id = allocator->num_heaps;

		Opal_Result opal_result = metal_allocatorBlockAlloc(device, desc->allocation_type, heap_id, allocator->heap_size, &block_handle);
		if (opal_result != OPAL_SUCCESS)
			return opal_result;

		Metal_MemoryHeap *heap = &allocator->heaps[heap_id];

		opal_result = opal_heapInitialize(&heap->heap, allocator->heap_size, allocator->max_heap_allocations);
		assert(opal_result == OPAL_SUCCESS);

		heap->next_heap = allocator->first_heap[memory_type];
		heap->block = block_handle;

		allocator->first_heap[memory_type] = heap_id;
		allocator->num_heaps++;

		opal_result = metal_allocatorStageHeapAlloc(allocator, heap_id, desc, &node_index, &offset);
		assert(opal_result == OPAL_SUCCESS);
	}

	// heap alloc VRAM
	Metal_MemoryBlock *block = opal_poolGetElement(&allocator->blocks, block_handle);
	assert(block);

	Opal_Result result = metal_allocatorCommitHeapAlloc(allocator, heap_id, node_index, offset, desc);
	assert(result == OPAL_SUCCESS);

	allocation->block = block_handle;
	allocation->memory = block->memory;
	allocation->offset = offset;
	allocation->heap_metadata = node_index;

	allocator->last_used_heap[memory_type] = heap_id;

	return result;
}

Opal_Result metal_allocatorFreeMemory(Metal_Device *device, Metal_Allocation allocation)
{
	assert(device);

	Metal_Allocator *allocator = &device->allocator;
	assert(allocation.block != OPAL_POOL_HANDLE_NULL);

	Metal_MemoryBlock *block = opal_poolGetElement(&allocator->blocks, allocation.block);
	assert(block);

	if (block->heap != OPAL_HEAP_NULL)
	{
		allocator->last_used_heap[block->memory_type] = block->heap;
		return metal_allocatorFreeHeapAlloc(allocator, block->heap, allocation.heap_metadata, allocation.offset);
	}

	[block->memory release];
	return opal_poolRemoveElement(&allocator->blocks, allocation.block);
}
