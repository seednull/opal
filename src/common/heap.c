#include "heap.h"
#include "intrinsics.h"

#include <stdlib.h>
#include <string.h>

/*
 */
static Opal_BinIndex opal_toBinIndexRoundUp(uint32_t value)
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

static uint32_t opal_toBinSize(Opal_BinIndex value)
{
	uint32_t mantissa = value & OPAL_MANTISSA_MASK;
	uint32_t exponent = value >> OPAL_MANTISSA_BITS;

	if (exponent == 0)
		return mantissa;

	return (mantissa | OPAL_MANTISSA_MAX) << (exponent - 1);
}

static Opal_Result opal_findBinFromIndex(uint32_t bins, uint8_t index, uint8_t *value)
{
	assert(value);

	uint32_t mask = (1 << index) - 1;
	mask = ~mask;

	uint32_t masked_bins = bins & mask;
	if (masked_bins == 0)
		return OPAL_NO_MEMORY;

	*value = tzcnt(masked_bins);
	return OPAL_SUCCESS;
}

/*
 */
static void opal_heapAddNodeToBin(Opal_Heap *heap, Opal_NodeIndex index, uint32_t size, uint32_t offset)
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
	node->next_neighbour = OPAL_NODE_INDEX_NULL;
	node->prev_neighbour = OPAL_NODE_INDEX_NULL;
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

static void opal_heapRemoveNodeFromBin(Opal_Heap *heap, Opal_NodeIndex index)
{
	assert(heap);
	assert(index != OPAL_NODE_INDEX_NULL);

	Opal_HeapNode *node = &heap->nodes[index];
	assert(node);

	Opal_BinIndex bin_index = opal_toBinIndexRoundUp(node->size);
	Opal_NodeIndex bin_head_index = heap->bins[bin_index];

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

	uint8_t sparse_bin_index = bin_index >> OPAL_MANTISSA_BITS;
	uint8_t linear_bin_index = bin_index & OPAL_MANTISSA_MASK;

	uint8_t linear_bin_mask = heap->used_linear_bins[sparse_bin_index];
	uint32_t sparse_bin_mask = heap->used_sparse_bins;

	if (heap->bins[bin_index] == OPAL_NODE_INDEX_NULL)
	{
		linear_bin_mask &= ~(1 << linear_bin_index);
		if (linear_bin_mask == 0)
			sparse_bin_mask &= ~(1 << sparse_bin_index);

		heap->used_linear_bins[sparse_bin_index] = linear_bin_mask;
		heap->used_sparse_bins = sparse_bin_mask;
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

static Opal_Result opal_heapFindBinForSize(const Opal_Heap *heap, uint32_t size, Opal_BinIndex *result)
{
	assert(heap);
	assert(result);

	Opal_BinIndex index = opal_toBinIndexRoundUp(size);

	uint8_t sparse_bin_index = index >> OPAL_MANTISSA_BITS;
	uint8_t linear_bin_index = OPAL_LINEAR_BIN_INDEX_NULL;

	uint8_t used_linear_bins = heap->used_linear_bins[sparse_bin_index];

	if (used_linear_bins != 0)
	{
		uint32_t min_linear_bin = index & OPAL_MANTISSA_BITS;
		opal_findBinFromIndex(used_linear_bins, min_linear_bin, &linear_bin_index);
	}

	if (linear_bin_index == OPAL_LINEAR_BIN_INDEX_NULL)
	{
		Opal_Result result = opal_findBinFromIndex(heap->used_sparse_bins, sparse_bin_index + 1, &sparse_bin_index);
		if (result != OPAL_SUCCESS)
			return result;

		assert(sparse_bin_index != 0);

		used_linear_bins = heap->used_linear_bins[sparse_bin_index];
		assert(used_linear_bins != 0);

		linear_bin_index = (uint8_t)tzcnt(used_linear_bins);
	}

	assert(linear_bin_index != OPAL_LINEAR_BIN_INDEX_NULL);

	*result = (sparse_bin_index << OPAL_MANTISSA_BITS) | (linear_bin_index & OPAL_MANTISSA_MASK);
	return OPAL_SUCCESS;
}

/*
 */
Opal_Result opal_heapInitialize(Opal_Heap *heap, uint32_t size, uint32_t max_allocations)
{
	assert(heap);

	memset(heap, 0, sizeof(Opal_Heap));
	heap->num_nodes = max_allocations;
	heap->num_free_nodes = max_allocations;
	heap->size = size;

	heap->nodes = (Opal_HeapNode *)malloc(sizeof(Opal_HeapNode) * max_allocations);
	heap->free_indices = (Opal_NodeIndex *)malloc(sizeof(Opal_NodeIndex) * max_allocations);

	for (uint32_t i = 0; i < max_allocations; ++i)
		heap->free_indices[i] = max_allocations - i - 1;

	for (uint32_t i = 0; i < OPAL_NUM_BINS; ++i)
		heap->bins[i] = OPAL_NODE_INDEX_NULL;

	Opal_NodeIndex index = opal_heapGrabNodeIndex(heap);

	opal_heapAddNodeToBin(heap, index, size, 0);
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
	assert(heap);
	assert(allocation);
	assert(size > 0);

	Opal_NodeIndex node_index = OPAL_NODE_INDEX_NULL;
	uint32_t offset = 0;

	Opal_Result result = opal_heapStageAlloc(heap, size, &node_index, &offset);
	if (result != OPAL_SUCCESS)
		return result;

	result = opal_heapCommitAlloc(heap, node_index, offset, size);
	if (result != OPAL_SUCCESS)
		return result;

	allocation->offset = offset;
	allocation->metadata = node_index;

	return OPAL_SUCCESS;
}

Opal_Result opal_heapAllocAligned(Opal_Heap *heap, uint32_t size, uint32_t alignment, Opal_HeapAllocation *allocation)
{
	assert(heap);
	assert(allocation);
	assert(size > 0);
	assert(isPow2(alignment));

	Opal_NodeIndex node_index = OPAL_NODE_INDEX_NULL;
	uint32_t offset = 0;

	Opal_Result result = opal_heapStageAllocAligned(heap, size, alignment, &node_index, &offset);
	if (result != OPAL_SUCCESS)
		return result;

	result = opal_heapCommitAlloc(heap, node_index, offset, size);
	if (result != OPAL_SUCCESS)
		return result;

	allocation->offset = offset;
	allocation->metadata = node_index;

	return OPAL_SUCCESS;
}

Opal_Result opal_heapStageAlloc(const Opal_Heap *heap, uint32_t size, Opal_NodeIndex *node_index, uint32_t *offset)
{
	assert(heap);
	assert(node_index);
	assert(offset);
	assert(size > 0);

	Opal_BinIndex bin_index = 0;
	
	Opal_Result result = opal_heapFindBinForSize(heap, size, &bin_index);
	if (result != OPAL_SUCCESS)
		return result;

	assert(bin_index != 0);
	*node_index = heap->bins[bin_index];

	assert(*node_index != OPAL_NODE_INDEX_NULL);
	Opal_HeapNode *node = &heap->nodes[*node_index];

	assert(node);
	*offset = node->offset;

	return OPAL_SUCCESS;
}

Opal_Result opal_heapStageAllocAligned(const Opal_Heap *heap, uint32_t size, uint32_t alignment, Opal_NodeIndex *node_index, uint32_t *offset)
{
	assert(heap);
	assert(node_index);
	assert(offset);
	assert(size > 0);
	assert(isPow2(alignment));

	Opal_BinIndex bin_index = 0;
	
	Opal_Result result = opal_heapFindBinForSize(heap, size, &bin_index);
	if (result != OPAL_SUCCESS)
		return result;

	assert(bin_index != 0);
	*node_index = heap->bins[bin_index];

	assert(*node_index != OPAL_NODE_INDEX_NULL);
	Opal_HeapNode *node = &heap->nodes[*node_index];

	assert(node);
	*offset = alignUp(node->offset, alignment);

	uint32_t remainder_begin_size = *offset - node->offset;
	if (remainder_begin_size + size > node->size)
	{
		uint32_t max_size = size + alignment - 1;
		result = opal_heapFindBinForSize(heap, max_size, &bin_index);
		if (result != OPAL_SUCCESS)
			return result;

		assert(bin_index != 0);
		*node_index = heap->bins[bin_index];

		assert(*node_index != OPAL_NODE_INDEX_NULL);
		node = &heap->nodes[*node_index];

		assert(node);
		*offset = alignUp(node->offset, alignment);
	}

	return OPAL_SUCCESS;
}

Opal_Result opal_heapCommitAlloc(Opal_Heap *heap, Opal_NodeIndex node_index, uint32_t offset, uint32_t size)
{
	assert(heap);
	assert(node_index != OPAL_NODE_INDEX_NULL);
	assert(size > 0);

	Opal_HeapNode *node = &heap->nodes[node_index];
	assert(node);
	assert(node->used == 0);
	assert(offset >= node->offset);

	uint32_t remainder_begin_size = offset - node->offset;
	uint32_t remainder_begin_offset = node->offset;
	uint32_t remainder_end_size = node->size - remainder_begin_size - size;
	uint32_t remainder_end_offset = offset + size;

	assert(remainder_begin_size + size <= node->size);

	Opal_NodeIndex prev_index = node->prev_neighbour;
	Opal_NodeIndex next_index = node->next_neighbour;

	opal_heapRemoveNodeFromBin(heap, node_index);

	node->offset = offset;
	node->used = 1;
	node->size = size;

	if (remainder_begin_size > 0)
	{
		Opal_HeapNode *prev_node = (prev_index != OPAL_NODE_INDEX_NULL) ? &heap->nodes[prev_index] : NULL;

		// try merge with previous free node
		if (prev_node != NULL && prev_node->used == 0)
		{
			remainder_begin_offset = prev_node->offset;
			remainder_begin_size += prev_node->size;

			Opal_NodeIndex prev_prev_index = prev_node->prev_neighbour;

			opal_heapRemoveNodeFromBin(heap, prev_index);
			opal_heapReleaseNodeIndex(heap, prev_index);

			prev_index = prev_prev_index;
		}

		Opal_NodeIndex new_index = opal_heapGrabNodeIndex(heap);
		opal_heapAddNodeToBin(heap, new_index, remainder_begin_size, remainder_begin_offset);

		Opal_HeapNode *new_node = &heap->nodes[new_index];
		assert(new_node);

		node->prev_neighbour = new_index;
		new_node->next_neighbour = node_index;

		new_node->prev_neighbour = prev_index;
		if (prev_node)
			prev_node->next_neighbour = new_index;
	}

	if (remainder_end_size > 0)
	{
		Opal_HeapNode *next_node = (next_index != OPAL_NODE_INDEX_NULL) ? &heap->nodes[next_index] : NULL;

		// try merge with previous free node
		if (next_node != NULL && next_node->used == 0)
		{
			remainder_end_size += next_node->size;

			Opal_NodeIndex next_next_index = next_node->next_neighbour;

			opal_heapRemoveNodeFromBin(heap, next_index);
			opal_heapReleaseNodeIndex(heap, next_index);

			next_index = next_next_index;
		}

		Opal_NodeIndex new_index = opal_heapGrabNodeIndex(heap);
		opal_heapAddNodeToBin(heap, new_index, remainder_end_size, remainder_end_offset);

		Opal_HeapNode *new_node = &heap->nodes[new_index];
		assert(new_node);

		node->next_neighbour = new_index;
		new_node->prev_neighbour = node_index;

		new_node->next_neighbour = next_index;
		if (next_node)
			next_node->prev_neighbour = new_index;
	}

	return OPAL_SUCCESS;
}

uint32_t opal_heapCanAlloc(const Opal_Heap *heap, uint32_t size)
{
	assert(heap);
	assert(size > 0);

	Opal_NodeIndex node_index = OPAL_NODE_INDEX_NULL;
	uint32_t offset = 0;

	Opal_Result result = opal_heapStageAlloc(heap, size, &node_index, &offset);
	if (result != OPAL_SUCCESS)
		return 0;

	assert(node_index != OPAL_NODE_INDEX_NULL);
	return 1;
}

uint32_t opal_heapCanAllocAligned(const Opal_Heap *heap, uint32_t size, uint32_t alignment)
{
	Opal_NodeIndex node_index = OPAL_NODE_INDEX_NULL;
	uint32_t offset = 0;

	Opal_Result result = opal_heapStageAllocAligned(heap, size, alignment, &node_index, &offset);
	if (result != OPAL_SUCCESS)
		return 0;

	assert(node_index != OPAL_NODE_INDEX_NULL);
	return 1;
}

Opal_Result opal_heapFree(Opal_Heap *heap, Opal_HeapAllocation allocation)
{
	assert(allocation.metadata != OPAL_NODE_INDEX_NULL);

	Opal_HeapNode *node = &heap->nodes[allocation.metadata];
	assert(node);
	assert(node->used);

	Opal_NodeIndex prev_index = node->prev_neighbour;
	Opal_NodeIndex next_index = node->next_neighbour;

	Opal_HeapNode *next_node = (next_index != OPAL_NODE_INDEX_NULL) ? &heap->nodes[next_index] : NULL;
	Opal_HeapNode *prev_node = (prev_index != OPAL_NODE_INDEX_NULL) ? &heap->nodes[prev_index] : NULL;

	Opal_NodeIndex new_prev_index = prev_index;
	Opal_NodeIndex new_next_index = next_index;

	// try merge with previous free node
	if (prev_node != NULL && prev_node->used == 0)
	{
		node->offset = prev_node->offset;
		node->size += prev_node->size;

		new_prev_index = prev_node->prev_neighbour;

		opal_heapRemoveNodeFromBin(heap, prev_index);
		opal_heapReleaseNodeIndex(heap, prev_index);
	}

	// try merge with previous next node
	if (next_node != NULL && next_node->used == 0)
	{
		node->size += next_node->size;

		new_next_index = next_node->next_neighbour;

		opal_heapRemoveNodeFromBin(heap, next_index);
		opal_heapReleaseNodeIndex(heap, next_index);
	}

	opal_heapReleaseNodeIndex(heap, allocation.metadata);

	Opal_NodeIndex new_index = opal_heapGrabNodeIndex(heap);
	opal_heapAddNodeToBin(heap, new_index, node->size, node->offset);

	node = &heap->nodes[new_index];
	node->prev_neighbour = new_prev_index;
	node->next_neighbour = new_next_index;

	if (new_prev_index != OPAL_NODE_INDEX_NULL)
	{
		Opal_HeapNode *prev_prev_node = &heap->nodes[new_prev_index];
		prev_prev_node->next_neighbour = new_index;
	}

	if (new_next_index != OPAL_NODE_INDEX_NULL)
	{
		Opal_HeapNode *next_next_node = &heap->nodes[new_next_index];
		next_next_node->prev_neighbour = new_index;
	}

	return OPAL_SUCCESS;
}
