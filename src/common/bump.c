#include "bump.h"
#include "intrinsics.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 */
Opal_Result opal_bumpInitialize(Opal_Bump *bump, uint32_t capacity)
{
	assert(bump);
	assert(capacity > 0);

	memset(bump, 0, sizeof(Opal_Bump));

	bump->capacity = capacity;
	bump->data = (uint8_t *)malloc(capacity);

	return OPAL_SUCCESS;
}

Opal_Result opal_bumpShutdown(Opal_Bump *bump)
{
	assert(bump);

	free(bump->data);

	memset(bump, 0, sizeof(Opal_Bump));

	return OPAL_SUCCESS;
}

/*
 */
uint32_t opal_bumpAlloc(Opal_Bump *bump, uint32_t size)
{
	assert(bump);
	assert(bump->data);

	uint32_t new_capacity = bump->size + size;

	if (bump->capacity < new_capacity)
	{
		bump->data = (uint8_t *)realloc(bump->data, new_capacity);
		bump->capacity = new_capacity;
	}

	uint32_t offset = bump->size;
	bump->size += size;

	return offset;
}

Opal_Result opal_bumpReset(Opal_Bump *bump)
{
	assert(bump);
	assert(bump->data);

	bump->size = 0;

	return OPAL_SUCCESS;
}
