#include "heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/*
 */
typedef struct Buffer_t
{
	Opal_HeapAllocation allocation;
	uint32_t size;
	void *ptr;
} Buffer;

typedef struct Context_t
{
	uint8_t *vram;
	Opal_Heap *heap;
} Context;

/*
 */
Context gpu_createContext(uint32_t size, uint32_t allocations)
{
	Context context = {0};
	context.vram = (uint8_t *)malloc(sizeof(uint8_t) * size);
	context.heap = (Opal_Heap *)malloc(sizeof(Opal_Heap));
	Opal_Result result = opal_heapInitialize(context.heap, size, allocations);
	assert(result == OPAL_SUCCESS);

	return context;
}

void gpu_destroyContext(Context *context)
{
	Opal_Result result = opal_heapShutdown(context->heap);
	assert(result == OPAL_SUCCESS);

	free(context->heap);
	free(context->vram);
}

void *gpu_map(const Context *context, const Buffer *buffer)
{
	return context->vram + buffer->allocation.offset;
}

Buffer gpu_createBuffer(const Context *context, uint32_t size)
{
	Buffer buffer = {0};
	buffer.size = size;
	Opal_Result result = opal_heapAlloc(context->heap, size, &buffer.allocation);
	assert(result == OPAL_SUCCESS);

	return buffer;
}

void gpu_destroyBuffer(const Context *context, const Buffer *buffer)
{
	Opal_Result result = opal_heapFree(context->heap, buffer->allocation);
	assert(result == OPAL_SUCCESS);
}

/*
 */
static Context context;
static Buffer buffers[16];
static Buffer big_buffer;

static const uint32_t heap_size = 16*1024*1024;
static const uint32_t max_allocations = 1000000;
static const uint32_t buffer_size = 1024 * 1024;

void heap_testEqualSize(uint32_t num_iterations)
{
	for (uint32_t iteration = 0; iteration < num_iterations; ++iteration)
	{
		for (uint32_t i = 0; i < 16; ++i)
		{
			buffers[i] = gpu_createBuffer(&context, buffer_size);
			buffers[i].ptr = gpu_map(&context, &buffers[i]);
		}

		for (uint32_t i = 0; i < 16; ++i)
			memset(buffers[i].ptr, i, sizeof(uint8_t) * buffers[i].size);

		for (uint32_t i = 0; i < 16; ++i)
			gpu_destroyBuffer(&context, &buffers[i]);
	}
}

void heap_testMaxSize(uint32_t num_iterations)
{
	for (uint32_t iteration = 0; iteration < num_iterations; ++iteration)
	{
		big_buffer = gpu_createBuffer(&context, heap_size);
		big_buffer.ptr = gpu_map(&context, &big_buffer);

		memset(big_buffer.ptr, 0xCC, sizeof(uint8_t) * big_buffer.size);

		gpu_destroyBuffer(&context, &big_buffer);
	}
}

void heap_testCheckerDestroy(uint32_t num_iterations)
{
	for (uint32_t iteration = 0; iteration < num_iterations; ++iteration)
	{
		for (uint32_t i = 0; i < 16; ++i)
		{
			buffers[i] = gpu_createBuffer(&context, buffer_size);
			buffers[i].ptr = gpu_map(&context, &buffers[i]);
		}

		for (uint32_t i = 0; i < 16; ++i)
			if (i % 2 == 0)
				gpu_destroyBuffer(&context, &buffers[i]);

		for (uint32_t i = 0; i < 16; ++i)
			if (i % 2 != 0)
				gpu_destroyBuffer(&context, &buffers[i]);
	}
}

/*
 */
int main()
{
	context = gpu_createContext(heap_size, max_allocations);

	heap_testEqualSize(16);
	heap_testMaxSize(16);
	heap_testCheckerDestroy(16);
	heap_testMaxSize(16);

	gpu_destroyContext(&context);

	return 0;
}
