#pragma once

#include <opal.h>
#include <assert.h>

// Note: 'static' keyword is intentionally put here to avoid linker errors on emscripten.
//       If compiled only with OPAL_INLINE, it searches for symbols and fails to link.
//
//       On a side, it's ok to keep these intrinsics statis and have multiple copies in
//       translation units. We hope the compiler will inline them anyways.

/*
 */
static OPAL_INLINE uint32_t lzcnt(uint32_t value)
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

static OPAL_INLINE uint32_t tzcnt(uint32_t value)
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

static OPAL_INLINE uint32_t popcnt(uint32_t value)
{
#ifdef _MSC_VER
	return __popcnt(value);
#else
	return __builtin_popcount(value);
#endif
}

static OPAL_INLINE uint32_t isPow2(uint32_t value)
{
	return (value & (value - 1)) == 0;
}

static OPAL_INLINE uint32_t alignDown(uint32_t value, uint32_t alignment)
{
	assert(isPow2(alignment));

	uint32_t mask = alignment - 1;
	return value & ~mask;
}

static OPAL_INLINE uint32_t alignUp(uint32_t value, uint32_t alignment)
{
	assert(isPow2(alignment));

	uint32_t mask = alignment - 1;
	return (value + mask) & ~mask;
}

static OPAL_INLINE uint32_t min(uint32_t a, uint32_t b)
{
	return (a < b) ? a : b;
}

static OPAL_INLINE uint32_t max(uint32_t a, uint32_t b)
{
	return (a < b) ? b : a;
}
