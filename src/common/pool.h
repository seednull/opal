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

	uint32_t element_size;
	uint32_t size;
	uint32_t capacity;

	uint32_t *masks;
	uint32_t *indices;
	uint32_t num_free_indices;
} Opal_Pool;

extern Opal_Result opal_poolInitialize(Opal_Pool *pool, uint32_t element_size, uint32_t capacity);
extern Opal_Result opal_poolShutdown(Opal_Pool *pool);

extern Opal_PoolHandle opal_poolAddElement(Opal_Pool *pool, const void *data);
extern Opal_Result opal_poolRemoveElement(Opal_Pool *pool, Opal_PoolHandle handle);
extern void *opal_poolGetElement(const Opal_Pool *pool, Opal_PoolHandle handle);
