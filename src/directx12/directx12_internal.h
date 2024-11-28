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
#define D3D12_MAX_COMMAND_POOL_ALLOCATORS 8U

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
	ID3D12Debug1 *debug;
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
	Opal_Pool semaphores;
	Opal_Pool buffers;
	Opal_Pool command_pools;
	Opal_Pool command_buffers;
	Opal_Pool shaders;
	Opal_Pool descriptor_heaps;
	Opal_Pool bindset_layouts;
	Opal_Pool pipeline_layouts;
	Opal_Pool pipelines;
	Opal_Pool swapchains;

	DirectX12_Allocator allocator;
} DirectX12_Device;

typedef struct DirectX12_Queue_t
{
	ID3D12CommandQueue *queue;
	ID3D12Fence *fence;
	UINT64 wanted_value;
	HANDLE event;
	D3D12_COMMAND_LIST_TYPE type;
} DirectX12_Queue;

typedef struct DirectX12_Semaphore_t
{
	ID3D12Fence *fence;
	HANDLE event;
} DirectX12_Semaphore;

typedef struct DirectX12_Buffer_t
{
	ID3D12Resource *buffer;
	uint32_t map_count;
	DirectX12_Allocation allocation;
} DirectX12_Buffer;

typedef struct DirectX12_CommandPool_t
{
	ID3D12CommandAllocator *allocators[D3D12_MAX_COMMAND_POOL_ALLOCATORS];
	uint32_t usages[D3D12_MAX_COMMAND_POOL_ALLOCATORS];
	D3D12_COMMAND_LIST_TYPE type;
} DirectX12_CommandPool;

typedef struct DirectX12_CommandBuffer_t
{
	ID3D12GraphicsCommandList *list;
	Opal_CommandPool pool;
	uint32_t index;
	uint32_t recording;
} DirectX12_CommandBuffer;

typedef struct DirectX12_Shader_t
{
	void *data;
	SIZE_T size;
} DirectX12_Shader;

typedef struct DirectX12_DescriptorHeap_t
{
	ID3D12DescriptorHeap *memory;
	D3D12_DESCRIPTOR_HEAP_TYPE type;
	Opal_Heap heap;
} DirectX12_DescriptorHeap;

typedef struct DirectX12_DescriptorInfo_t
{
	D3D12_DESCRIPTOR_RANGE_TYPE type;
	UINT binding;
	D3D12_SHADER_VISIBILITY visibility;
} DirectX12_DescriptorInfo;

typedef struct DirectX12_BindsetLayout_t
{
	DirectX12_DescriptorInfo *descriptors;
	uint32_t num_table_cbv_descriptors;
	uint32_t num_table_uav_descriptors;
	uint32_t num_table_srv_descriptors;
	uint32_t num_table_sampler_descriptors;
	uint32_t num_inline_descriptors;
} DirectX12_BindsetLayout;

typedef struct DirectX12_PipelineLayout_t
{
	ID3D12RootSignature *root_signature;
} DirectX12_PipelineLayout;

typedef struct DirectX12_Pipeline_t
{
	ID3D12PipelineState *pipeline_state;
} DirectX12_Pipeline;

typedef struct DirectX12_Surface_t
{
	HWND handle;
} DirectX12_Surface;

typedef struct DirectX12_Swapchain_t
{
	IDXGISwapChain3 *swapchain;
	UINT present_flags;
	uint32_t current_index;
	uint32_t num_textures;
} DirectX12_Swapchain;

typedef HRESULT (WINAPI* PFN_DXGI_CREATE_FACTORY)(REFIID, _COM_Outptr_ void **);

extern PFN_D3D12_CREATE_DEVICE opal_d3d12CreateDevice;
extern PFN_D3D12_SERIALIZE_ROOT_SIGNATURE opal_d3d12SerializeRootSignature;
extern PFN_D3D12_GET_DEBUG_INTERFACE opal_d3d12GetDebugInterface;
extern PFN_DXGI_CREATE_FACTORY opal_dxgiCreateFactory1;

Opal_Result directx12_helperFillDeviceInfo(IDXGIAdapter1 *adapter, ID3D12Device *device, Opal_DeviceInfo *info);
Opal_Result directx12_helperFillDeviceEnginesInfo(DirectX12_DeviceEnginesInfo *info);
Opal_Result directx12_deviceInitialize(DirectX12_Device *device_ptr, DirectX12_Instance *instance_ptr, IDXGIAdapter1 *adapter, ID3D12Device *device);
Opal_Result directx12_deviceAllocateMemory(DirectX12_Device *device_ptr, const DirectX12_AllocationDesc *desc, DirectX12_Allocation *allocation);

D3D12_RESOURCE_STATES directx12_helperToInitialBufferResourceState(Opal_AllocationMemoryType type, Opal_BufferUsageFlags usage);
D3D12_DESCRIPTOR_HEAP_TYPE directx12_helperToDescriptorHeapType(Opal_DescriptorHeapType type);
D3D12_SHADER_VISIBILITY directx12_helperToShaderVisibility(Opal_ShaderStage stage);
D3D12_STENCIL_OP directx12_helperToStencilOp(Opal_StencilOp op);
D3D12_COMPARISON_FUNC directx12_helperToComparisonFunc(Opal_CompareOp op);
D3D12_BLEND_OP directx12_helperToBlendOp(Opal_BlendOp op);
D3D12_BLEND directx12_helperToBlendFactor(Opal_BlendFactor factor);
D3D12_CULL_MODE directx12_helperToCullMode(Opal_CullMode mode);
UINT directx12_helperToSampleCount(Opal_Samples samples);
UINT directx12_helperToInstanceDataStepRate(Opal_VertexInputRate rate);
D3D12_INPUT_CLASSIFICATION directx12_helperToInputSlotClass(Opal_VertexInputRate rate);
D3D12_INDEX_BUFFER_STRIP_CUT_VALUE directx12_helperToStripCutValue(Opal_IndexFormat format);
D3D12_PRIMITIVE_TOPOLOGY_TYPE directx12_helperToPrimitiveTopology(Opal_PrimitiveType type);

DXGI_FORMAT directx12_helperToDXGIVertexFormat(Opal_VertexFormat format);
DXGI_FORMAT directx12_helperToDXGITextureFormat(Opal_TextureFormat format);
DXGI_USAGE directx12_helperToDXGIUsage(Opal_TextureUsageFlags usage);
DXGI_COLOR_SPACE_TYPE directx12_helperToDXGIColorSpace(Opal_ColorSpace color_space);

Opal_Result directx12_allocatorInitialize(DirectX12_Device *device, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps);
Opal_Result directx12_allocatorShutdown(DirectX12_Device *device);
Opal_Result directx12_allocatorAllocateMemory(DirectX12_Device *device, const DirectX12_AllocationDesc *desc, uint32_t dedicated, DirectX12_Allocation *allocation);
Opal_Result directx12_allocatorFreeMemory(DirectX12_Device *device, DirectX12_Allocation allocation);
