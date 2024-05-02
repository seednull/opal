#pragma once

#include <opal.h>
#include <assert.h>

/*
 */
OPAL_INLINE uint32_t lzcnt(uint32_t value)
{
	assert(value != 0);

#ifdef _MSC_VER
	uint32_t result = 0;
	_BitScanReverse(&result, value);
	return 31 - result;
#else
	return __builtin_clz(value);
#endif
}

OPAL_INLINE uint32_t tzcnt(uint32_t value)
{
	assert(value != 0);

#ifdef _MSC_VER
	uint32_t result = 0;
	_BitScanForward(&result, value);
	return result;
#else
	return __builtin_ctz(value);
#endif
}

OPAL_INLINE uint32_t popcnt(uint32_t value)
{
#ifdef _MSC_VER
	return __popcnt(value);
#else
	return __builtin_popcount(value);
#endif
}

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
