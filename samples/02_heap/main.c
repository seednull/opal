#include "heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

const uint32_t heap_size = 256*1024*1024;
const uint32_t max_allocations = 1000000;
const uint32_t buffer_size = 64;

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
	opal_heapInitialize(context.heap, size, allocations);

	return context;
}

void gpu_destroyContext(Context *context)
{
	opal_heapShutdown(context->heap);
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
	opal_heapAlloc(context->heap, size, &buffer.allocation);

	return buffer;
}

void gpu_destroyBuffer(const Context *context, const Buffer *buffer)
{
	opal_heapFree(context->heap, buffer->allocation);
}

/*
 */
static Context context;
static Buffer buffers[16];

/*
 */
int main()
{
	context = gpu_createContext(heap_size, max_allocations);

	for (uint32_t i = 0; i < 16; ++i)
	{
		buffers[i] = gpu_createBuffer(&context, buffer_size);
		buffers[i].ptr = gpu_map(&context, &buffers[i]);
	}

	for (uint32_t i = 0; i < 16; ++i)
		memset(buffers[i].ptr, i, sizeof(uint8_t) * buffers[i].size);

	for (uint32_t i = 0; i < 16; ++i)
		gpu_destroyBuffer(&context, &buffers[i]);

	gpu_destroyContext(&context);

	return 0;
}
