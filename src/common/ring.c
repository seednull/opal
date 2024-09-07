#include "ring.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

Opal_Result opal_ringInitialize(Opal_Ring *ring, uint32_t size)
{
	assert(ring);
	assert(size > 0);

	ring->data = (uint8_t *)malloc(size);
	ring->capacity = size;
	ring->size = 0;
	ring->read = 0;
	ring->write = 0;

	return OPAL_SUCCESS;
}

Opal_Result opal_ringShutdown(Opal_Ring *ring)
{
	assert(ring);
	free(ring->data);

	return OPAL_SUCCESS;
}

uint32_t opal_ringGetSize(const Opal_Ring *ring)
{
	assert(ring);
	return ring->size;
}

void opal_ringRead(Opal_Ring *ring, void *data, uint32_t size)
{
	assert(ring);
	assert(ring->size >= size);
	assert(data);

	if (size == 0)
		return;

	uint8_t *begin_ptr = ring->data + ring->read;

	ring->read += size;
	ring->read %= ring->capacity;
	ring->size -= size;

	uint8_t *end_ptr = ring->data + ring->read;

	if (begin_ptr < end_ptr)
		memcpy(data, begin_ptr, size);
	else
	{
		uint32_t remainder = size - ring->read;
		memcpy(data, begin_ptr, remainder);

		if (ring->read > 0)
			memcpy((uint8_t *)data + remainder, ring->data, ring->read);
	}
}

void opal_ringWrite(Opal_Ring *ring, const void *data, uint32_t size)
{
	if (size == 0)
		return;

	assert(ring);
	assert(ring->size + size <= ring->capacity);
	assert(data);

	uint8_t *begin_ptr = ring->data + ring->write;

	ring->write += size;
	ring->write %= ring->capacity;
	ring->size += size;

	uint8_t *end_ptr = ring->data + ring->write;

	if (begin_ptr < end_ptr)
		memcpy(begin_ptr, data, size);
	else
	{
		uint32_t remainder = size - ring->write;
		memcpy(begin_ptr, data, remainder);

		if (ring->write > 0)
			memcpy(ring->data, (uint8_t *)data + remainder, ring->write);
	}
}
