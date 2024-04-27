#pragma once

#include <opal.h>
#include <assert.h>

#define OPAL_MANTISSA_BITS			3
#define OPAL_MANTISSA_MAX 			0x00000008
#define OPAL_MANTISSA_MASK			0x00000007

#define OPAL_EXPONENT_BITS			5
#define OPAL_EXPONENT_MAX			0x00000020

#define OPAL_NUM_SPARSE_BINS		OPAL_EXPONENT_MAX
#define OPAL_NUM_LINEAR_BINS		OPAL_MANTISSA_MAX
#define OPAL_NUM_BINS				OPAL_NUM_SPARSE_BINS * OPAL_NUM_LINEAR_BINS

#define OPAL_NODE_INDEX_NULL		0xFFFFFFFF
#define OPAL_LINEAR_BIN_INDEX_NULL	0xFF

typedef uint8_t Opal_BinIndex;
typedef uint32_t Opal_NodeIndex;

typedef struct Opal_HeapAllocation_t
{
	uint32_t offset;
	Opal_NodeIndex metadata;
} Opal_HeapAllocation;

typedef struct Opal_HeapNode_t
{
	uint32_t offset;
	uint32_t size;
	Opal_NodeIndex next_bin;
	Opal_NodeIndex prev_bin;
	Opal_NodeIndex next_neighbour;
	Opal_NodeIndex prev_neighbour;
	uint8_t used;
} Opal_HeapNode;

typedef struct Opal_Heap_t
{
	uint32_t used_sparse_bins;
	uint8_t used_linear_bins[OPAL_NUM_SPARSE_BINS];
	Opal_NodeIndex bins[OPAL_NUM_BINS];
	Opal_HeapNode *nodes;
	Opal_NodeIndex *free_indices;
	uint32_t num_nodes;
	uint32_t num_free_nodes;
	uint32_t size;
} Opal_Heap;

extern Opal_Result opal_heapInitialize(Opal_Heap *heap, uint32_t size, uint32_t max_allocations);
extern Opal_Result opal_heapShutdown(Opal_Heap *heap);

extern Opal_Result opal_heapAlloc(Opal_Heap *heap, uint32_t size, Opal_HeapAllocation *allocation);
extern Opal_Result opal_heapAllocAligned(Opal_Heap *heap, uint32_t size, uint32_t alignment, Opal_HeapAllocation *allocation);

extern Opal_Result opal_heapStageAlloc(const Opal_Heap *heap, uint32_t size, Opal_NodeIndex *node_index, uint32_t *offset);
extern Opal_Result opal_heapStageAllocAligned(const Opal_Heap *heap, uint32_t size, uint32_t alignment, Opal_NodeIndex *node_index, uint32_t *offset);
extern Opal_Result opal_heapCommitAlloc(Opal_Heap *heap, Opal_NodeIndex node_index, uint32_t offset, uint32_t size);

extern uint32_t opal_heapCanAlloc(const Opal_Heap *heap, uint32_t size);
extern uint32_t opal_heapCanAllocAligned(const Opal_Heap *heap, uint32_t size, uint32_t alignment);

extern Opal_Result opal_heapFree(Opal_Heap *heap, Opal_HeapAllocation allocation);

OPAL_INLINE uint32_t isPow2(uint32_t value)
{
	return (value & (value - 1)) == 0;
}

OPAL_INLINE uint32_t alignDown(uint32_t value, uint32_t alignment)
{
	assert(isPow2(alignment));

	uint32_t mask = alignment - 1;
	return value & ~mask;
}

OPAL_INLINE uint32_t alignUp(uint32_t value, uint32_t alignment)
{
	assert(isPow2(alignment));

	uint32_t mask = alignment - 1;
	return (value + mask) & ~mask;
}
