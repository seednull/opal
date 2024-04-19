#include "pool.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 */
OPAL_INLINE uint32_t opal_poolGrabIndex(Opal_Pool *pool)
{
	assert(pool);
	assert(pool->num_free_indices > 0);

	uint32_t result = pool->free_indices[pool->num_free_indices - 1];
	pool->num_free_indices--;

	uint32_t mask_index = result / 32;
	uint32_t mask_bit = result % 32;

	pool->free_masks[mask_index] &= ~(1 << mask_bit);

	return result;
}

OPAL_INLINE void opal_poolReleaseIndex(Opal_Pool *pool, uint32_t index)
{
	assert(pool);
	assert(pool->num_free_indices < pool->capacity);
	assert(index < pool->capacity);

	pool->num_free_indices++;
	pool->free_indices[pool->num_free_indices - 1] = index;

	uint32_t mask_index = index / 32;
	uint32_t mask_bit = index % 32;

	pool->free_masks[mask_index] |= 1 << mask_bit;
}

OPAL_INLINE uint32_t opal_poolIsIndexFree(const Opal_Pool *pool, uint32_t index)
{
	assert(pool);
	assert(index < pool->capacity);

	uint32_t mask_index = index / 32;
	uint32_t mask_bit = index % 32;

	uint32_t free_mask = pool->free_masks[mask_index];
	uint32_t element_mask = 1 << mask_bit;

	return free_mask & element_mask;
}

OPAL_INLINE uint32_t opal_poolGetNumMasks(const Opal_Pool *pool)
{
	assert(pool);

	uint32_t num_masks = pool->capacity / 32;
	if (pool->capacity % 32 != 0)
		num_masks++;

	return num_masks;
}

/*
 */
Opal_Result opal_poolInitialize(Opal_Pool *pool, uint32_t element_size, uint32_t capacity)
{
	assert(pool);
	assert(element_size > 0);

	memset(pool, 0, sizeof(Opal_Pool));

	if (capacity > 0)
	{
		uint32_t num_masks = opal_poolGetNumMasks(pool);

		pool->data = (uint8_t *)malloc(element_size * capacity);
		pool->generations = (uint8_t *)malloc(sizeof(uint8_t) * capacity);
		pool->free_indices = (uint32_t *)malloc(sizeof(uint32_t) * capacity);
		pool->free_masks = (uint32_t *)malloc(sizeof(uint32_t) * num_masks);

		for (uint32_t i = 0; i < capacity; ++i)
			pool->free_indices[i] = capacity - i - 1;

		memset(pool->free_masks, 0xFFFFFFFF, sizeof(uint32_t) * num_masks);
		memset(pool->generations, 0, sizeof(uint8_t) * capacity);
	}

	pool->element_size = element_size;
	pool->capacity = capacity;
	pool->num_free_indices = capacity;

	return OPAL_SUCCESS;
}

Opal_Result opal_poolShutdown(Opal_Pool *pool)
{
	assert(pool);

	free(pool->data);
	free(pool->generations);
	free(pool->free_indices);

	memset(pool, 0, sizeof(Opal_Pool));

	return OPAL_SUCCESS;
}

/*
 */
Opal_PoolHandle opal_poolAddElement(Opal_Pool *pool, const void *data)
{
	assert(pool);
	assert(data);

	if (pool->size == pool->capacity)
	{
		uint32_t old_capacity = pool->capacity;
		uint32_t old_num_masks = opal_poolGetNumMasks(pool);

		pool->capacity = (pool->capacity == 0) ? 1 : pool->capacity * 2;
		uint32_t new_num_masks = opal_poolGetNumMasks(pool);

		pool->data = (uint8_t *)realloc(pool->data, pool->element_size * pool->capacity);
		pool->generations = (uint8_t *)realloc(pool->generations, sizeof(uint8_t) * pool->capacity);
		pool->free_indices = (uint32_t *)realloc(pool->free_indices, sizeof(uint32_t) * pool->capacity);

		if (old_num_masks != new_num_masks)
			pool->free_masks = (uint32_t *)realloc(pool->free_masks, sizeof(uint32_t) * new_num_masks);

		for (uint32_t i = old_capacity; i < pool->capacity; ++i)
		{
			pool->num_free_indices++;
			pool->free_indices[pool->num_free_indices - 1] = old_capacity + pool->capacity - i - 1;
			pool->generations[i] = 0;
		}

		for (uint32_t i = old_num_masks; i < new_num_masks; ++i)
			pool->free_masks[i] = 0xFFFFFFFF;
	}

	assert(pool->num_free_indices > 0);

	uint32_t index = opal_poolGrabIndex(pool);

	uint8_t *data_ptr = pool->data + index * pool->element_size;
	uint8_t *generation_ptr = pool->generations + index;

	memcpy(data_ptr, data, pool->element_size);
	(*generation_ptr)++;

	pool->size++;

	return opal_poolHandlePack(index, *generation_ptr);
}

Opal_Result opal_poolRemoveElement(Opal_Pool *pool, Opal_PoolHandle handle)
{
	assert(pool);

	if (handle == OPAL_POOL_HANDLE_NULL)
		return OPAL_INTERNAL_ERROR;

	if (pool->size == 0)
		return OPAL_INTERNAL_ERROR;

	uint32_t index = opal_poolHandleGetIndex(handle);
	uint8_t generation = opal_poolHandleGetGeneration(handle);

	if (pool->generations[index] != generation)
		return OPAL_INTERNAL_ERROR;

	if (opal_poolIsIndexFree(pool, index))
		return OPAL_INTERNAL_ERROR;

	opal_poolReleaseIndex(pool, index);
	pool->size--;

	return OPAL_SUCCESS;
}

void *opal_poolGetElement(const Opal_Pool *pool, Opal_PoolHandle handle)
{
	assert(pool);
	assert(pool->size > 0);

	if (handle == OPAL_POOL_HANDLE_NULL)
		return NULL;

	uint32_t index = opal_poolHandleGetIndex(handle);
	uint8_t generation = opal_poolHandleGetGeneration(handle);

	if (pool->generations[index] != generation)
		return NULL;

	if (opal_poolIsIndexFree(pool, index))
		return NULL;

	return pool->data + index * pool->element_size;
}
