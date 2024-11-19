#include "directx12_internal.h"

#undef min
#undef max
#include "common/intrinsics.h"

#include <stdlib.h>
#include <string.h>

#define OPAL_HEAP_NULL 0xFFFFFFFF

/*
 */
static UINT64 directx12_heap_alignments[DIRECTX12_RESOURCE_TYPE_ENUM_MAX] =
{
	D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
	D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
	D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
	D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT
};

static D3D12_HEAP_FLAGS directx12_heap_flags[DIRECTX12_RESOURCE_TYPE_ENUM_MAX] =
{
	D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS,
	D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES,
	D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES,
	D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES,
};

static D3D12_HEAP_TYPE directx12_heap_types[OPAL_ALLOCATION_MEMORY_TYPE_ENUM_MAX] =
{
	D3D12_HEAP_TYPE_DEFAULT,
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_HEAP_TYPE_READBACK,
};

/*
 */
static OPAL_INLINE uint32_t directx12_memoryTypePack(DirectX12_ResourceType resource_type, Opal_AllocationMemoryType allocation_type)
{
	return DIRECTX12_RESOURCE_TYPE_ENUM_MAX * allocation_type + resource_type;
}

/*
 */
static Opal_Result directx12_allocatorStageHeapAlloc(const DirectX12_Allocator *allocator, uint32_t heap_id, const DirectX12_AllocationDesc *desc, Opal_NodeIndex *node_index, uint32_t *offset)
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

	uint32_t alignment = (uint32_t)directx12_heap_alignments[desc->resource_type];
	assert(alignment <= allocator->heap_size);
	assert(isPow2ul(alignment));

	const DirectX12_MemoryHeap *heap = &allocator->heaps[heap_id];
	assert(heap);

	return opal_heapStageAllocAligned(&heap->heap, size, alignment, node_index, offset);
}

static Opal_Result directx12_allocatorCommitHeapAlloc(DirectX12_Allocator *allocator, uint32_t heap_id, Opal_NodeIndex node_index, uint32_t offset, const DirectX12_AllocationDesc *desc)
{
	assert(allocator);
	assert(desc);
	assert(heap_id < allocator->num_heaps);
	assert(heap_id != OPAL_HEAP_NULL);

	assert(node_index != OPAL_NODE_INDEX_NULL);

	uint32_t size = (uint32_t)desc->size;
	assert(size > 0);
	assert(size <= allocator->heap_size);

	uint32_t alignment = (uint32_t)directx12_heap_alignments[desc->resource_type];
	assert(alignment <= allocator->heap_size);
	assert(isPow2ul(alignment));

	DirectX12_MemoryHeap *heap = &allocator->heaps[heap_id];
	assert(heap);

	return opal_heapCommitAlloc(&heap->heap, node_index, offset, size);
}

static Opal_Result directx12_allocatorFreeHeapAlloc(DirectX12_Allocator *allocator, uint32_t heap_id, Opal_NodeIndex node_index, uint32_t offset)
{
	assert(allocator);
	assert(heap_id < allocator->num_heaps);
	assert(heap_id != OPAL_HEAP_NULL);

	assert(node_index != OPAL_NODE_INDEX_NULL);

	DirectX12_MemoryHeap *heap = &allocator->heaps[heap_id];
	assert(heap);

	Opal_HeapAllocation heap_allocation = {0};
	heap_allocation.offset = offset;
	heap_allocation.metadata = node_index;

	return opal_heapFree(&heap->heap, heap_allocation);
}

static Opal_Result directx12_allocatorBlockAlloc(DirectX12_Device *device, DirectX12_ResourceType resource_type, Opal_AllocationMemoryType allocation_type, uint32_t heap_id, UINT64 size, Opal_PoolHandle *handle)
{
	assert(device);
	assert(size > 0);
	assert(handle);

	uint32_t memory_type = directx12_memoryTypePack(resource_type, allocation_type);
	assert(memory_type < D3D12_MAX_MEMORY_TYPES);

	DirectX12_Allocator *allocator = &device->allocator;

	// TODO: budges & usages

	DirectX12_MemoryBlock block = {0};
	block.size = size;
	block.heap = heap_id;
	block.memory_type = memory_type;

	D3D12_HEAP_DESC info = {0};
	info.SizeInBytes = block.size;
	info.Alignment = directx12_heap_alignments[resource_type];
	info.Flags = directx12_heap_flags[resource_type];
	info.Properties.Type = directx12_heap_types[allocation_type];

	HRESULT hr = ID3D12Device_CreateHeap(device->device, &info, &IID_ID3D12Heap, &block.memory);
	if (!SUCCEEDED(hr))
		return OPAL_NO_MEMORY;

	*handle = opal_poolAddElement(&allocator->blocks, &block);
	return OPAL_SUCCESS;
}

/*
 */
Opal_Result directx12_allocatorInitialize(DirectX12_Device *device, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps)
{
	assert(device);

	DirectX12_Allocator *allocator = &device->allocator;
	memset(allocator, 0, sizeof(DirectX12_Allocator));

	allocator->heaps = (DirectX12_MemoryHeap *)malloc(sizeof(DirectX12_MemoryHeap) * max_heaps);
	allocator->num_heaps = 0;
	allocator->heap_size = heap_size;
	allocator->max_heap_allocations = max_heap_allocations;
	allocator->max_heaps = max_heaps;

	memset(allocator->heaps, 0, sizeof(DirectX12_MemoryHeap) * max_heaps);
	memset(allocator->first_heap, OPAL_HEAP_NULL, sizeof(uint32_t) * D3D12_MAX_MEMORY_TYPES);
	memset(allocator->last_used_heap, OPAL_HEAP_NULL, sizeof(uint32_t) * D3D12_MAX_MEMORY_TYPES);

	opal_poolInitialize(&allocator->blocks, sizeof(DirectX12_MemoryBlock), max_heaps);


	return OPAL_SUCCESS;
}

Opal_Result directx12_allocatorShutdown(DirectX12_Device *device)
{
	assert(device);

	DirectX12_Allocator *allocator = &device->allocator;
	assert(allocator->heaps);

	for (uint32_t i = 0; i < allocator->num_heaps; ++i)
	{
		DirectX12_MemoryHeap *heap = &allocator->heaps[i];
		opal_heapShutdown(&heap->heap);
	}
	free(allocator->heaps);

	uint32_t head = opal_poolGetHeadIndex(&allocator->blocks);
	while (head != OPAL_POOL_HANDLE_NULL)
	{
		DirectX12_MemoryBlock *block = (DirectX12_MemoryBlock *)opal_poolGetElementByIndex(&allocator->blocks, head);

		ID3D12Heap_Release(block->memory);
		head = opal_poolGetNextIndex(&allocator->blocks, head);
	}

	opal_poolShutdown(&allocator->blocks);

	return OPAL_SUCCESS;
}

/*
 */
Opal_Result directx12_allocatorAllocateMemory(DirectX12_Device *device, const DirectX12_AllocationDesc *desc, uint32_t dedicated, DirectX12_Allocation *allocation)
{
	assert(device);
	assert(desc);
	assert(desc->size > 0);
	assert(allocation);

	uint32_t memory_type = directx12_memoryTypePack(desc->resource_type, desc->allocation_type);
	assert(memory_type < D3D12_MAX_MEMORY_TYPES);

	DirectX12_Allocator *allocator = &device->allocator;

	// create dedicated block
	if (dedicated)
	{
		Opal_PoolHandle handle = OPAL_POOL_HANDLE_NULL;

		Opal_Result opal_result = directx12_allocatorBlockAlloc(device, desc->resource_type, desc->allocation_type, OPAL_HEAP_NULL, desc->size, &handle);
		if (opal_result != OPAL_SUCCESS)
			return opal_result;

		DirectX12_MemoryBlock *block = opal_poolGetElement(&allocator->blocks, handle);
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
		DirectX12_MemoryHeap *heap = &allocator->heaps[heap_id];
		block_handle = heap->block;

		Opal_Result opal_result = directx12_allocatorStageHeapAlloc(allocator, heap_id, desc, &node_index, &offset);
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
			DirectX12_MemoryHeap *heap = &allocator->heaps[pending_heap_id];

			Opal_Result opal_result = directx12_allocatorStageHeapAlloc(allocator, pending_heap_id, desc, &node_index, &offset);
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

		Opal_Result opal_result = directx12_allocatorBlockAlloc(device, desc->resource_type, desc->allocation_type, heap_id, allocator->heap_size, &block_handle);
		if (opal_result != OPAL_SUCCESS)
			return opal_result;

		DirectX12_MemoryHeap *heap = &allocator->heaps[heap_id];

		opal_result = opal_heapInitialize(&heap->heap, allocator->heap_size, allocator->max_heap_allocations);
		assert(opal_result == OPAL_SUCCESS);

		heap->next_heap = allocator->first_heap[memory_type];
		heap->block = block_handle;

		allocator->first_heap[memory_type] = heap_id;
		allocator->num_heaps++;

		opal_result = directx12_allocatorStageHeapAlloc(allocator, heap_id, desc, &node_index, &offset);
		assert(opal_result == OPAL_SUCCESS);
	}

	// heap alloc VRAM
	DirectX12_MemoryBlock *block = opal_poolGetElement(&allocator->blocks, block_handle);
	assert(block);

	Opal_Result result = directx12_allocatorCommitHeapAlloc(allocator, heap_id, node_index, offset, desc);
	assert(result == OPAL_SUCCESS);

	allocation->block = block_handle;
	allocation->memory = block->memory;
	allocation->offset = offset;
	allocation->heap_metadata = node_index;

	allocator->last_used_heap[memory_type] = heap_id;

	return result;
}

Opal_Result directx12_allocatorFreeMemory(DirectX12_Device *device, DirectX12_Allocation allocation)
{
	assert(device);

	DirectX12_Allocator *allocator = &device->allocator;
	assert(allocation.block != OPAL_POOL_HANDLE_NULL);

	DirectX12_MemoryBlock *block = opal_poolGetElement(&allocator->blocks, allocation.block);
	assert(block);

	if (block->heap != OPAL_HEAP_NULL)
	{
		allocator->last_used_heap[block->memory_type] = block->heap;
		return directx12_allocatorFreeHeapAlloc(allocator, block->heap, allocation.heap_metadata, allocation.offset);
	}

	ID3D12Heap_Release(block->memory);
	return opal_poolRemoveElement(&allocator->blocks, allocation.block);
}
