#pragma once

#include "opal_internal.h"

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <dxgi1_4.h>
#include <d3d12.h>

#include "common/bump.h"
#include "common/heap.h"
#include "common/pool.h"

#define D3D12_MAX_MEMORY_TYPES 16U

typedef enum DirectX12_ResourceType_t
{
	DIRECTX12_RESOURCE_TYPE_BUFFER = 0,
	DIRECTX12_RESOURCE_TYPE_NON_DS_RT_TEXTURE = 1,
	DIRECTX12_RESOURCE_TYPE_DS_RT_TEXTURE = 2,
	DIRECTX12_RESOURCE_TYPE_MSAA_DS_RT_TEXTURE = 3,

	DIRECTX12_RESOURCE_TYPE_ENUM_MAX,
	DIRECTX12_RESOURCE_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} DirectX12_ResourceType;

typedef struct DirectX12_MemoryBlock_t
{
	ID3D12Heap *memory;
	UINT64 size;
	uint32_t memory_type;
	uint32_t heap;
} DirectX12_MemoryBlock;

typedef struct DirectX12_MemoryHeap_t
{
	Opal_Heap heap;
	Opal_PoolHandle block;
	uint32_t next_heap;
} DirectX12_MemoryHeap;

typedef struct DirectX12_Allocator_t
{
	DirectX12_MemoryHeap *heaps;
	uint32_t num_heaps;

	Opal_Pool blocks;

	uint32_t first_heap[D3D12_MAX_MEMORY_TYPES];
	uint32_t last_used_heap[D3D12_MAX_MEMORY_TYPES];

	uint32_t heap_size;
	uint32_t max_heaps;
	uint32_t max_heap_allocations;
} DirectX12_Allocator;

typedef struct DirectX12_AllocationDesc_t
{
	UINT64 size;
	DirectX12_ResourceType resource_type;
	Opal_AllocationMemoryType allocation_type;
	Opal_AllocationHint hint;
} DirectX12_AllocationDesc;

typedef struct DirectX12_Allocation_t
{
	ID3D12Heap *memory;
	uint32_t offset;
	Opal_PoolHandle block;
	Opal_NodeIndex heap_metadata;
} DirectX12_Allocation;

typedef struct DirectX12_DeviceEnginesInfo_t
{
	uint32_t queue_counts[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX];
} DirectX12_DeviceEnginesInfo;

typedef struct DirectX12_Instance_t
{
	Opal_InstanceTable *vtbl;
	IDXGIFactory2 *factory;
	uint32_t heap_size;
	uint32_t max_heap_allocations;
	uint32_t max_heaps;
	Opal_Pool surfaces;
} DirectX12_Instance;

typedef struct DirectX12_Device_t
{
	Opal_DeviceTable *vtbl;
	DirectX12_Instance *instance;
	IDXGIAdapter1 *adapter;
	ID3D12Device *device;

	DirectX12_DeviceEnginesInfo device_engines_info;
	Opal_Queue *queue_handles[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX];
	Opal_Bump bump;
	Opal_Pool queues;
	Opal_Pool buffers;
	Opal_Pool command_pools;
	Opal_Pool command_buffers;
	Opal_Pool swapchains;

	DirectX12_Allocator allocator;
} DirectX12_Device;

typedef struct DirectX12_Queue_t
{
	ID3D12CommandQueue *queue;
	D3D12_COMMAND_LIST_TYPE type;
} DirectX12_Queue;

typedef struct DirectX12_Buffer_t
{
	ID3D12Resource *buffer;
	uint32_t map_count;
	DirectX12_Allocation allocation;
} DirectX12_Buffer;

typedef struct DirectX12_CommandPool_t
{
	ID3D12CommandAllocator *allocator;
	D3D12_COMMAND_LIST_TYPE type;
} DirectX12_CommandPool;

typedef struct DirectX12_CommandBuffer_t
{
	ID3D12GraphicsCommandList *list;
	Opal_CommandPool pool;
} DirectX12_CommandBuffer;

typedef struct DirectX12_Surface_t
{
	HWND handle;
} DirectX12_Surface;

typedef struct DirectX12_Swapchain_t
{
	IDXGISwapChain3 *swapchain;
} DirectX12_Swapchain;

Opal_Result directx12_helperFillDeviceInfo(IDXGIAdapter1 *adapter, ID3D12Device *device, Opal_DeviceInfo *info);
Opal_Result directx12_helperFillDeviceEnginesInfo(DirectX12_DeviceEnginesInfo *info);
Opal_Result directx12_deviceInitialize(DirectX12_Device *device_ptr, DirectX12_Instance *instance_ptr, IDXGIAdapter1 *adapter, ID3D12Device *device);
Opal_Result directx12_deviceAllocateMemory(DirectX12_Device *device_ptr, const DirectX12_AllocationDesc *desc, DirectX12_Allocation *allocation);

D3D12_RESOURCE_STATES directx12_helperToInitialBufferResourceState(Opal_AllocationMemoryType type, Opal_BufferUsageFlags usage);

DXGI_FORMAT directx12_helperToDXGIFormat(Opal_TextureFormat format);
DXGI_USAGE directx12_helperToDXGIUsage(Opal_TextureUsageFlags usage);
DXGI_COLOR_SPACE_TYPE directx12_helperToDXGIColorSpace(Opal_ColorSpace color_space);

Opal_Result directx12_allocatorInitialize(DirectX12_Device *device, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps);
Opal_Result directx12_allocatorShutdown(DirectX12_Device *device);
Opal_Result directx12_allocatorAllocateMemory(DirectX12_Device *device, const DirectX12_AllocationDesc *desc, uint32_t dedicated, DirectX12_Allocation *allocation);
Opal_Result directx12_allocatorFreeMemory(DirectX12_Device *device, DirectX12_Allocation allocation);
