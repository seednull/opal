#pragma once

#include "opal_internal.h"
#include <Metal/Metal.h>

#include "common/bump.h"
#include "common/heap.h"
#include "common/pool.h"

#define METAL_MAX_MEMORY_TYPES 3U

typedef struct Metal_MemoryBlock_t
{
	id<MTLHeap> memory;
	uint64_t size;
	uint32_t memory_type;
	uint32_t heap;
} Metal_MemoryBlock;

typedef struct Metal_MemoryHeap_t
{
	Opal_Heap heap;
	Opal_PoolHandle block;
	uint32_t next_heap;
} Metal_MemoryHeap;

typedef struct Metal_Allocator_t
{
	Metal_MemoryHeap *heaps;
	uint32_t num_heaps;

	Opal_Pool blocks;

	uint32_t first_heap[METAL_MAX_MEMORY_TYPES];
	uint32_t last_used_heap[METAL_MAX_MEMORY_TYPES];

	uint32_t heap_size;
	uint32_t max_heaps;
	uint32_t max_heap_allocations;
} Metal_Allocator;

typedef struct Metal_AllocationDesc_t
{
	uint64_t size;
	uint64_t alignment;
	Opal_AllocationMemoryType allocation_type;
	Opal_AllocationHint hint;
} Metal_AllocationDesc;

typedef struct Metal_Allocation_t
{
	id<MTLHeap> memory;
	uint32_t offset;
	Opal_PoolHandle block;
	Opal_NodeIndex heap_metadata;
} Metal_Allocation;

typedef struct Metal_Instance_t
{
	Opal_InstanceTable *vtbl;
	uint32_t heap_size;
	uint32_t max_heap_allocations;
	uint32_t max_heaps;
} Metal_Instance;

typedef struct Metal_Device_t
{
	Opal_DeviceTable *vtbl;
	id<MTLDevice> device;

	Opal_Bump bump;
	Opal_Pool buffers;

	Metal_Allocator allocator;
} Metal_Device;

typedef struct Metal_Buffer_t
{
	id<MTLBuffer> buffer;
	Metal_Allocation allocation;
} Metal_Buffer;

Opal_Result metal_helperFillDeviceInfo(id<MTLDevice> metal_device, Opal_DeviceInfo *info);

Opal_Result metal_deviceInitialize(Metal_Device *device_ptr, Metal_Instance *instance_ptr, id<MTLDevice> metal_device);
Opal_Result metal_deviceAllocateMemory(Metal_Device *device_ptr, const Metal_AllocationDesc *desc, Metal_Allocation *allocation);

Opal_Result metal_allocatorInitialize(Metal_Device *device, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps);
Opal_Result metal_allocatorShutdown(Metal_Device *device);
Opal_Result metal_allocatorAllocateMemory(Metal_Device *device, const Metal_AllocationDesc *desc, uint32_t dedicated, Metal_Allocation *allocation);
Opal_Result metal_allocatorFreeMemory(Metal_Device *device, Metal_Allocation allocation);
