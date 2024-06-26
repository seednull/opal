#pragma once

#include <opal.h>

#define OPAL_POOL_MAX_ELEMENTS		0x00FFFFFF
#define OPAL_POOL_MAX_GENERATIONS	0xFF
#define OPAL_POOL_HANDLE_NULL		0xFFFFFFFF

typedef uint32_t Opal_PoolHandle;

typedef struct Opal_Pool_t
{
	uint8_t *data;
	uint8_t *generations;
	uint32_t *nexts;
	uint32_t *prevs;
	uint32_t head;
	uint32_t tail;

	uint32_t element_size;
	uint32_t size;
	uint32_t capacity;

	uint32_t *masks;
	uint32_t *indices;
	uint32_t num_free_indices;
} Opal_Pool;

Opal_Result opal_poolInitialize(Opal_Pool *pool, uint32_t element_size, uint32_t capacity);
Opal_Result opal_poolShutdown(Opal_Pool *pool);

Opal_PoolHandle opal_poolAddElement(Opal_Pool *pool, const void *data);
Opal_Result opal_poolRemoveElement(Opal_Pool *pool, Opal_PoolHandle handle);
void *opal_poolGetElement(const Opal_Pool *pool, Opal_PoolHandle handle);

void *opal_poolGetElementByIndex(const Opal_Pool *pool, uint32_t index);
uint32_t opal_poolGetHeadIndex(const Opal_Pool *pool);
uint32_t opal_poolGetTailIndex(const Opal_Pool *pool);
uint32_t opal_poolGetNextIndex(const Opal_Pool *pool, uint32_t index);
uint32_t opal_poolGetPrevIndex(const Opal_Pool *pool, uint32_t index);
