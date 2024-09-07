#pragma once

#include <opal.h>

typedef struct Opal_Ring_t
{
	uint8_t *data;
	uint32_t capacity;
	uint32_t size;
	uint32_t read;
	uint32_t write;
} Opal_Ring;

Opal_Result opal_ringInitialize(Opal_Ring *ring, uint32_t size);
Opal_Result opal_ringShutdown(Opal_Ring *ring);

uint32_t opal_ringGetSize(const Opal_Ring *ring);

void opal_ringRead(Opal_Ring *ring, void *data, uint32_t size);
void opal_ringWrite(Opal_Ring *ring, const void *data, uint32_t size);
