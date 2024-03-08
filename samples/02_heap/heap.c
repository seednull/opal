#include "heap.h"

#include <stdlib.h>
#include <assert.h>

// FIXME: implement for other compilers / OS

/*
 */
static uint32_t lzcnt(uint32_t value)
{
	uint32_t result = 0;
	_BitScanReverse(&result, value);
	return 31 - result;
}

static uint32_t tzcnt(uint32_t value)
{
	uint32_t result = 0;
	_BitScanForward(&result, value);
	return result;
}

Opal_BinIndex opal_toBinIndexRoundUp(uint32_t value)
{
	if (value < OPAL_MANTISSA_MAX)
		return value;

	uint32_t leading_zeroes = lzcnt(value);
	uint32_t highest_bit = 31 - leading_zeroes;

	uint32_t mantissa_start_bit = highest_bit - OPAL_MANTISSA_BITS;

	uint32_t exponent = mantissa_start_bit + 1;
	uint32_t mantissa = (value >> mantissa_start_bit) & OPAL_MANTISSA_MASK;

	uint32_t lower_bits_mask = (1 << mantissa_start_bit) - 1;

	if ((value & lower_bits_mask) != 0)
		mantissa++;

	return (Opal_BinIndex)((exponent << OPAL_MANTISSA_BITS) + mantissa);
}

uint32_t opal_toBinSize(Opal_BinIndex value)
{
	uint32_t mantissa = value & OPAL_MANTISSA_MASK;
	uint32_t exponent = value >> OPAL_MANTISSA_BITS;

	if (exponent == 0)
		return mantissa;

	return (mantissa | OPAL_MANTISSA_MAX) << (exponent - 1);
}

/*
 */
static void opal_heapInsertNode(Opal_Heap *heap, Opal_NodeIndex index, uint32_t size, uint32_t offset)
{
	assert(heap);
	assert(index != OPAL_NODE_INDEX_NULL);

	Opal_BinIndex bin_index = opal_toBinIndexRoundUp(size);

	uint8_t sparse_bin_index = bin_index >> OPAL_MANTISSA_BITS;
	uint8_t linear_bin_index = bin_index & OPAL_MANTISSA_MASK;

	heap->used_sparse_bins |= 1 << sparse_bin_index;
	heap->used_linear_bins[sparse_bin_index] |= 1 << linear_bin_index;

	Opal_HeapNode *node = &heap->nodes[index];

	node->offset = offset;
	node->size = size;
	node->next_bin = OPAL_NODE_INDEX_NULL;
	node->prev_bin = OPAL_NODE_INDEX_NULL;
	node->used = 0;

	Opal_NodeIndex bin_head_index = heap->bins[bin_index];
	
	if (bin_head_index != OPAL_NODE_INDEX_NULL)
	{
		Opal_HeapNode *bin_head_node = &heap->nodes[bin_head_index];
		bin_head_node->prev_bin = index;
		node->next_bin = bin_head_index;
	}

	heap->bins[bin_index] = index;
}

static void opal_heapRemoveNode(Opal_Heap *heap, Opal_NodeIndex index)
{
	assert(heap);
	assert(index != OPAL_NODE_INDEX_NULL);

	Opal_HeapNode *node = &heap->nodes[index];
	assert(node);

	Opal_BinIndex bin_index = opal_toBinIndexRoundUp(node->size);
	Opal_NodeIndex bin_head_index = heap->bins[bin_index];

	uint8_t sparse_bin_index = bin_index >> OPAL_MANTISSA_BITS;
	uint8_t linear_bin_index = bin_index & OPAL_MANTISSA_MASK;

	uint8_t linear_bin_mask = heap->used_linear_bins[sparse_bin_index];
	uint32_t sparse_bin_mask = heap->used_sparse_bins;

	linear_bin_mask &= ~(1 << linear_bin_index);
	if (linear_bin_mask == 0)
		sparse_bin_mask &= ~(1 << sparse_bin_index);

	heap->used_linear_bins[sparse_bin_index] = linear_bin_mask;
	heap->used_sparse_bins = sparse_bin_mask;

	if (index == bin_head_index)
		heap->bins[bin_index] = node->next_bin;

	if (node->prev_bin != OPAL_NODE_INDEX_NULL)
	{
		Opal_HeapNode *prev_node = &heap->nodes[node->prev_bin];
		prev_node->next_bin = node->next_bin;
	}

	if (node->next_bin != OPAL_NODE_INDEX_NULL)
	{
		Opal_HeapNode *next_node = &heap->nodes[node->next_bin];
		next_node->prev_bin = node->prev_bin;
	}
}

static Opal_NodeIndex opal_heapGrabNodeIndex(Opal_Heap *heap)
{
	assert(heap);
	assert(heap->num_free_nodes > 0);

	Opal_NodeIndex result = heap->free_indices[heap->num_free_nodes - 1];
	heap->num_free_nodes--;

	return result;
}

static void opal_heapReleaseNodeIndex(Opal_Heap *heap, Opal_NodeIndex index)
{
	assert(heap);
	assert(heap->num_free_nodes < heap->num_nodes);

	heap->num_free_nodes++;
	heap->free_indices[heap->num_free_nodes - 1] = index;
}

/*
 */
Opal_Result opal_heapInitialize(Opal_Heap *heap, uint32_t size, uint32_t num_allocations)
{
	assert(heap);

	memset(heap, 0, sizeof(Opal_Heap));
	heap->num_nodes = num_allocations;
	heap->num_free_nodes = num_allocations;
	heap->size = size;

	heap->nodes = (Opal_HeapNode *)malloc(sizeof(Opal_HeapNode) * num_allocations);
	heap->free_indices = (Opal_NodeIndex *)malloc(sizeof(Opal_NodeIndex) * num_allocations);

	for (uint32_t i = 0; i < num_allocations; ++i)
		heap->free_indices[i] = num_allocations - i - 1;

	for (uint32_t i = 0; i < OPAL_NUM_BINS; ++i)
		heap->bins[i] = OPAL_NODE_INDEX_NULL;

	Opal_NodeIndex index = opal_heapGrabNodeIndex(heap);

	opal_heapInsertNode(heap, index, size, 0);
	return OPAL_SUCCESS;
}

Opal_Result opal_heapShutdown(Opal_Heap *heap)
{
	assert(heap);
	assert(heap->free_indices);
	assert(heap->nodes);

	free(heap->free_indices);
	free(heap->nodes);

	memset(heap, 0, sizeof(Opal_Heap));
	return OPAL_SUCCESS;
}

Opal_Result opal_heapAlloc(Opal_Heap *heap, uint32_t size, Opal_HeapAllocation *allocation)
{
	Opal_BinIndex index = opal_toBinIndexRoundUp(size);

	uint8_t sparse_bin_index = index >> OPAL_MANTISSA_BITS;
	uint8_t linear_bin_index = index & OPAL_MANTISSA_MASK;
	uint8_t linear_bin_mask = (1 << linear_bin_index) - 1;

	uint8_t used_linear_bin_mask = heap->used_linear_bins[sparse_bin_index];
	if ((used_linear_bin_mask & linear_bin_mask) == 0)
	{
		uint32_t sparse_bin_mask = (1 << sparse_bin_index) - 1;
		sparse_bin_mask |= (1 << sparse_bin_index);
		sparse_bin_mask = ~sparse_bin_mask;

		sparse_bin_index = tzcnt(heap->used_sparse_bins & sparse_bin_mask);
		if (sparse_bin_index == 0)
			return OPAL_NO_MEMORY;

		used_linear_bin_mask = heap->used_linear_bins[sparse_bin_index];
		assert(used_linear_bin_mask != 0);

		linear_bin_index = (uint8_t)tzcnt(used_linear_bin_mask);

		index = (sparse_bin_index << OPAL_MANTISSA_BITS);
		index |= (linear_bin_index & OPAL_MANTISSA_MASK);
	}

	Opal_NodeIndex node_index = heap->bins[index];
	assert(node_index != OPAL_NODE_INDEX_NULL);

	Opal_HeapNode *node = &heap->nodes[node_index];
	uint32_t remainder = node->size - size;

	opal_heapRemoveNode(heap, node_index);

	node->used = 1;
	node->size = size;

	if (remainder > 0)
	{
		// TODO: neighbours

		Opal_NodeIndex new_index = opal_heapGrabNodeIndex(heap);
		opal_heapInsertNode(heap, new_index, remainder, node->offset + size);
	}

	allocation->offset = node->offset;
	allocation->metadata = node_index;

	return OPAL_SUCCESS;
}

Opal_Result opal_heapFree(Opal_Heap *heap, Opal_HeapAllocation allocation)
{
	assert(allocation.metadata != OPAL_NODE_INDEX_NULL);

	Opal_HeapNode *node = &heap->nodes[allocation.metadata];
	assert(node);
	assert(node->used);

	node->used = 0;
	opal_heapReleaseNodeIndex(heap, allocation.metadata);

	// TODO: merge neighbours

	Opal_NodeIndex new_index = opal_heapGrabNodeIndex(heap);
	opal_heapInsertNode(heap, new_index, node->size, node->offset);

	return OPAL_SUCCESS;
}
