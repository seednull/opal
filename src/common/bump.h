#pragma once

#include <opal.h>

typedef struct Opal_Bump_t
{
	uint8_t *data;
	uint32_t size;
	uint32_t capacity;
} Opal_Bump;

Opal_Result opal_bumpInitialize(Opal_Bump *bump, uint32_t capacity);
Opal_Result opal_bumpShutdown(Opal_Bump *bump);

uint32_t opal_bumpAlloc(Opal_Bump *bump, uint32_t size);
Opal_Result opal_bumpReset(Opal_Bump *bump);
