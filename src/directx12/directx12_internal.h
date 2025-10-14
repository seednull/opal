#pragma once

#include "opal_internal.h"

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <dxgi1_4.h>
#include <d3d12.h>

#include "common/bump.h"
#include "common/heap.h"
#include "common/pool.h"

#define D3D12_MAX_MEMORY_TYPES 20U

typedef enum DirectX12_ResourceType_t
{
	DIRECTX12_RESOURCE_TYPE_BUFFER = 0,
	DIRECTX12_RESOURCE_TYPE_NON_DS_RT_TEXTURE,
	DIRECTX12_RESOURCE_TYPE_MSAA_NON_DS_RT_TEXTURE,
	DIRECTX12_RESOURCE_TYPE_DS_RT_TEXTURE,
	DIRECTX12_RESOURCE_TYPE_MSAA_DS_RT_TEXTURE,

	DIRECTX12_RESOURCE_TYPE_ENUM_MAX,
	DIRECTX12_RESOURCE_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} DirectX12_ResourceType;

typedef enum DirectX12_PassType_t
{
	DIRECTX12_PASS_TYPE_NONE = 0,
	DIRECTX12_PASS_TYPE_GRAPHICS,
	DIRECTX12_PASS_TYPE_COMPUTE,

	DIRECTX12_PASS_TYPE_ENUM_MAX,
	DIRECTX12_PASS_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} DirectX12_PassType;

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
	UINT64 alignment;
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

typedef struct DirectX12_FramebufferDescriptorHeap_t
{
	ID3D12DescriptorHeap *rtv_heap;
	ID3D12DescriptorHeap *dsv_heap;
} DirectX12_FramebufferDescriptorHeap;

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
	Opal_Pool textures;
	Opal_Pool texture_views;
	Opal_Pool samplers;
	Opal_Pool acceleration_structures;
	Opal_Pool command_allocators;
	Opal_Pool command_buffers;
	Opal_Pool shaders;
	Opal_Pool descriptor_heaps;
	Opal_Pool descriptor_set_layouts;
	Opal_Pool descriptor_sets;
	Opal_Pool pipeline_layouts;
	Opal_Pool pipelines;
	Opal_Pool swapchains;

	DirectX12_Allocator allocator;
	DirectX12_FramebufferDescriptorHeap framebuffer_descriptor_heap;
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
	D3D12_GPU_VIRTUAL_ADDRESS address;
	uint32_t map_count;
	DirectX12_Allocation allocation;
} DirectX12_Buffer;

typedef struct DirectX12_Texture_t
{
	ID3D12Resource *texture;
	DXGI_FORMAT format;
	UINT16 depth;
	UINT64 width;
	UINT64 height;
	UINT samples;
	DirectX12_Allocation allocation;
} DirectX12_Texture;

typedef struct DirectX12_TextureView_t
{
	ID3D12Resource *texture; // TODO: bad design, better to store Opal_Texture handle
	DXGI_FORMAT format;
	UINT subresource_index;
	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
	D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	D3D12_RENDER_TARGET_VIEW_DESC rtv_desc;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;
} DirectX12_TextureView;

typedef struct DirectX12_Sampler_t
{
	D3D12_SAMPLER_DESC desc;
} DirectX12_Sampler;

typedef struct DirectX12_AccelerationStructure_t
{
	ID3D12Resource *buffer;
	D3D12_GPU_VIRTUAL_ADDRESS address;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE type;
	uint32_t allow_compaction;
	DirectX12_Allocation allocation;
} DirectX12_AccelerationStructure;

typedef struct DirectX12_CommandAllocator_t
{
	ID3D12CommandAllocator *allocator;
	D3D12_COMMAND_LIST_TYPE type;
} DirectX12_CommandAllocator;

typedef struct DirectX12_CommandBuffer_t
{
	ID3D12GraphicsCommandList4 *list;
	D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS resolve_parameters[9];
	Opal_CommandAllocator allocator;
	uint32_t recording;
	DirectX12_PassType pass;
	Opal_PipelineLayout pipeline_layout;
} DirectX12_CommandBuffer;

typedef struct DirectX12_Shader_t
{
	void *data;
	SIZE_T size;
} DirectX12_Shader;

typedef struct DirectX12_DescriptorHeap_t
{
	ID3D12DescriptorHeap *resource_memory;
	ID3D12DescriptorHeap *sampler_memory;
	Opal_Heap resource_heap;
	Opal_Heap sampler_heap;
} DirectX12_DescriptorHeap;

typedef struct DirectX12_DescriptorInfo_t
{
	Opal_DescriptorType opal_type;
	D3D12_DESCRIPTOR_RANGE_TYPE api_type;
	D3D12_ROOT_PARAMETER_TYPE root_type;
	UINT binding;
} DirectX12_DescriptorInfo;

typedef struct DirectX12_DescriptorSetLayout_t
{
	DirectX12_DescriptorInfo *descriptors;
	uint32_t num_table_cbv_descriptors;
	uint32_t num_table_uav_descriptors;
	uint32_t num_table_srv_descriptors;
	uint32_t num_table_sampler_descriptors;
	uint32_t num_inline_descriptors;
} DirectX12_DescriptorSetLayout;

typedef struct DirectX12_DescriptorSet_t
{
	D3D12_GPU_DESCRIPTOR_HANDLE resource_handle;
	D3D12_GPU_DESCRIPTOR_HANDLE sampler_handle;
	Opal_HeapAllocation resource_allocation;
	Opal_HeapAllocation sampler_allocation;
	Opal_DescriptorSetLayout layout;
	Opal_DescriptorHeap heap;
	// TODO: think about using fixed array
	uint32_t num_inline_descriptors;
	Opal_DescriptorSetEntry *inline_descriptors;
} DirectX12_DescriptorSet;

typedef struct DirectX12_PipelineLayout_t
{
	ID3D12RootSignature *root_signature;
	uint32_t num_layout_table_offsets;
	uint32_t *layout_table_offsets;
	uint32_t num_inline_descriptors;
	uint32_t inline_offset;
} DirectX12_PipelineLayout;

typedef struct DirectX12_Pipeline_t
{
	ID3D12PipelineState *pipeline_state;
	ID3D12StateObject *state_object;
	ID3D12RootSignature *root_signature;
	D3D12_PRIMITIVE_TOPOLOGY primitive_topology;
	uint32_t num_raygen_shaders;
	uint32_t num_hitgroups;
	uint32_t num_miss_shaders;
	uint8_t *shader_handles;
} DirectX12_Pipeline;

typedef struct DirectX12_Surface_t
{
	HWND handle;
} DirectX12_Surface;

typedef struct DirectX12_Swapchain_t
{
	IDXGISwapChain3 *swapchain;
	Opal_TextureView *texture_views;
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

D3D12_RESOURCE_STATES directx12_helperToResourceState(Opal_ResourceState state);
D3D12_RESOURCE_STATES directx12_helperToInitialBufferResourceState(Opal_AllocationMemoryType type, Opal_BufferUsageFlags flags);
D3D12_RESOURCE_FLAGS directx12_helperToTextureFlags(Opal_TextureUsageFlags flags, Opal_TextureFormat format);
D3D12_RESOURCE_DIMENSION directx12_helperToTextureDimension(Opal_TextureType type);
DirectX12_ResourceType directx12_helperToTextureResourceType(Opal_TextureUsageFlags flags, Opal_Samples samples);
D3D12_FILTER directx12_helperToSamplerFilter(Opal_SamplerFilterMode min, Opal_SamplerFilterMode mag, Opal_SamplerFilterMode mip);
D3D12_TEXTURE_ADDRESS_MODE directx12_helperToSamplerAddressMode(Opal_SamplerAddressMode mode);
D3D12_STENCIL_OP directx12_helperToStencilOp(Opal_StencilOp op);
D3D12_COMPARISON_FUNC directx12_helperToComparisonFunc(Opal_CompareOp op);
D3D12_BLEND_OP directx12_helperToBlendOp(Opal_BlendOp op);
D3D12_BLEND directx12_helperToBlendFactor(Opal_BlendFactor factor);
D3D12_CULL_MODE directx12_helperToCullMode(Opal_CullMode mode);
UINT directx12_helperToSampleCount(Opal_Samples samples);
UINT directx12_helperToInstanceDataStepRate(Opal_VertexInputRate rate);
D3D12_INPUT_CLASSIFICATION directx12_helperToInputSlotClass(Opal_VertexInputRate rate);
D3D12_INDEX_BUFFER_STRIP_CUT_VALUE directx12_helperToStripCutValue(Opal_IndexFormat format);
D3D12_PRIMITIVE_TOPOLOGY_TYPE directx12_helperToPrimitiveTopologyType(Opal_PrimitiveType type);
D3D12_PRIMITIVE_TOPOLOGY directx12_helperToPrimitiveTopology(Opal_PrimitiveType type);
D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE directx12_helperToBeginningAccessType(Opal_LoadOp op);
D3D12_RENDER_PASS_ENDING_ACCESS_TYPE directx12_helperToEndingAccessType(Opal_StoreOp op);

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE directx12_helperToAccelerationStructureType(Opal_AccelerationStructureType type);
D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS directx12_helperToAccelerationStructureBuildFlags(Opal_AccelerationStructureBuildFlags flags);
D3D12_RAYTRACING_GEOMETRY_TYPE directx12_helperToAccelerationStructureGeometryType(Opal_AccelerationStructureGeometryType type);
D3D12_RAYTRACING_GEOMETRY_FLAGS directx12_helperToAccelerationStructureGeometryFlags(Opal_AccelerationStructureGeometryFlags flags);
D3D12_RAYTRACING_INSTANCE_FLAGS directx12_helperToAccelerationStructureInstanceFlags(Opal_AccelerationStructureInstanceFlags flags);
D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE directx12_helperToAccelerationStructureCopyMode(Opal_AccelerationStructureCopyMode mode);

DXGI_FORMAT directx12_helperToDXGIVertexFormat(Opal_VertexFormat format);
DXGI_FORMAT directx12_helperToDXGITextureFormat(Opal_TextureFormat format);
DXGI_FORMAT directx12_helperToDXGIIndexFormat(Opal_IndexFormat format);
DXGI_USAGE directx12_helperToDXGIUsage(Opal_TextureUsageFlags usage);
DXGI_COLOR_SPACE_TYPE directx12_helperToDXGIColorSpace(Opal_ColorSpace color_space);

Opal_Result directx12_allocatorInitialize(DirectX12_Device *device, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps);
Opal_Result directx12_allocatorShutdown(DirectX12_Device *device);
Opal_Result directx12_allocatorAllocateMemory(DirectX12_Device *device, const DirectX12_AllocationDesc *desc, uint32_t dedicated, DirectX12_Allocation *allocation);
Opal_Result directx12_allocatorFreeMemory(DirectX12_Device *device, DirectX12_Allocation allocation);
