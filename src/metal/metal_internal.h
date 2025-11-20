#pragma once

#include "opal_internal.h"
#include <QuartzCore/CAMetalLayer.h>
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

	id<MTLResidencySet> residency_set;
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

typedef struct Metal_DeviceEnginesInfo_t
{
	uint32_t queue_counts[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX];
} Metal_DeviceEnginesInfo;

typedef struct Metal_Instance_t
{
	Opal_InstanceTable *vtbl;
	uint32_t heap_size;
	uint32_t max_heap_allocations;
	uint32_t max_heaps;

	Opal_Pool surfaces;
} Metal_Instance;

typedef struct Metal_Device_t
{
	Opal_DeviceTable *vtbl;
	Metal_Instance *instance;
	id<MTLDevice> device;

	Metal_DeviceEnginesInfo device_engines_info;
	Opal_Queue *queue_handles[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX];
	Opal_Bump bump;
	Opal_Pool queues;
	Opal_Pool semaphores;
	Opal_Pool fences;
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
	Opal_Pool graphics_pipelines;
	Opal_Pool compute_pipelines;
	Opal_Pool swapchains;

	Metal_Allocator allocator;
} Metal_Device;

typedef struct Metal_Queue_t
{
	id<MTLCommandQueue> queue;
} Metal_Queue;

typedef struct Metal_Semaphore_t
{
	id<MTLEvent> event;
	id<MTLSharedEvent> shared_event;
} Metal_Semaphore;

typedef struct Metal_Fence_t
{
	id<MTLFence> fence;
} Metal_Fence;

typedef struct Metal_Buffer_t
{
	id<MTLBuffer> buffer;
	Metal_Allocation allocation;
} Metal_Buffer;

typedef struct Metal_Texture_t
{
	id<MTLTexture> texture;
	MTLPixelFormat format;
	Opal_Samples samples;
	Metal_Allocation allocation;
} Metal_Texture;

typedef struct Metal_TextureView_t
{
	id<MTLTexture> texture_view;
	Opal_Texture texture;
	NSUInteger base_mip;
	NSUInteger base_layer;
} Metal_TextureView;

typedef struct Metal_Sampler_t
{
	id<MTLSamplerState> sampler;
} Metal_Sampler;

typedef struct Metal_AccelerationStructure_t
{
	id<MTLAccelerationStructure> acceleration_structure;
	Metal_Allocation allocation;
} Metal_AccelerationStructure;

typedef struct Metal_CommandAllocator_t
{
	Opal_Queue queue;
	uint32_t command_buffer_usage;
} Metal_CommandAllocator;

typedef struct Metal_CommandBuffer_t
{
	id<MTLCommandBuffer> command_buffer;
	NSAutoreleasePool *pool;

	id<MTLRenderCommandEncoder> graphics_pass_encoder;
	MTLPrimitiveType primitive_type;
	Opal_IndexBufferView index_buffer_view;
	Opal_PipelineLayout pipeline_layout;
	uint32_t vertex_binding_offset;

	id<MTLComputeCommandEncoder> compute_pass_encoder;
	MTLSize threadgroup_size;

	id<MTLBlitCommandEncoder> copy_pass_encoder;
	id<MTLAccelerationStructureCommandEncoder> acceleration_structure_pass_encoder;

	Opal_Queue queue;
	Opal_CommandAllocator command_allocator;
} Metal_CommandBuffer;

typedef struct Metal_Shader_t
{
	id<MTLLibrary> library;
} Metal_Shader;

typedef struct Metal_DescriptorHeap_t
{
	id<MTLBuffer> buffer;
	Metal_Allocation allocation;
	Opal_Heap heap;
} Metal_DescriptorHeap;

typedef struct Metal_DescriptorInfo_t
{
	Opal_DescriptorType opal_type;
	MTLDataType api_type;
	uint32_t binding;
} Metal_DescriptorInfo;

typedef struct Metal_DescriptorSetLayout_t
{
	id<MTLArgumentEncoder> encoder;
	uint32_t num_blocks;
	uint32_t num_static_descriptors;
	uint32_t num_dynamic_descriptors;
	uint32_t num_descriptors;
	Metal_DescriptorInfo *descriptors;
} Metal_DescriptorSetLayout;

typedef struct Metal_DescriptorSet_t
{
	Opal_DescriptorSetLayout layout;
	Opal_DescriptorHeap heap;
	Opal_HeapAllocation allocation;
	uint64_t buffer_offset;
	// TODO: think about using fixed array
	uint32_t num_static_descriptors;
	uint32_t num_dynamic_descriptors;
	Opal_DescriptorSetEntry *dynamic_descriptors;
} Metal_DescriptorSet;

typedef struct Metal_PipelineLayout_t
{
	Opal_DescriptorSetLayout *layouts;
	uint32_t *dynamic_binding_offsets;
	uint32_t num_layouts;
	uint32_t vertex_binding_offset;
} Metal_PipelineLayout;

typedef struct Metal_GraphicsPipeline_t
{
	id<MTLRenderPipelineState> pipeline;
	id<MTLDepthStencilState> depth_stencil_state;
	MTLPrimitiveType primitive_type;
	MTLCullMode cull_mode;
	MTLWinding winding;
	MTLIndexType index_format;
} Metal_GraphicsPipeline;

typedef struct Metal_ComputePipeline_t
{
	id<MTLComputePipelineState> pipeline;
	MTLSize threadgroup_size;
} Metal_ComputePipeline;

typedef struct Metal_Swapchain_t
{
	Opal_Surface surface;
	id<MTLCommandQueue> queue;
	CGColorSpaceRef colorspace;

	Opal_TextureView current_texture_view;
	id<CAMetalDrawable> current_drawable;
} Metal_Swapchain;

typedef struct Metal_Surface_t
{
	CAMetalLayer *layer;
} Metal_Surface;

Opal_Result metal_deviceInitialize(Metal_Device *device_ptr, Metal_Instance *instance_ptr, id<MTLDevice> metal_device);

Opal_Result metal_helperFillDeviceEnginesInfo(Metal_DeviceEnginesInfo *info);
Opal_Result metal_helperFillDeviceInfo(id<MTLDevice> metal_device, Opal_DeviceInfo *info);

MTLDataType metal_helperToArgumentDataType(Opal_DescriptorType type);

MTLTextureType metal_helperToTextureType(Opal_TextureType type, Opal_Samples samples);
MTLTextureType metal_helperToTextureViewType(Opal_TextureViewType type, Opal_Samples samples);
MTLPixelFormat metal_helperToPixelFormat(Opal_TextureFormat format);
NSUInteger metal_helperToSampleCount(Opal_Samples samples);
MTLTextureUsage metal_helperToTextureUsage(Opal_TextureUsageFlags flags);

MTLSamplerAddressMode metal_helperToSamplerAddressMode(Opal_SamplerAddressMode mode);
MTLSamplerMinMagFilter metal_helperToSamplerMinMagFilter(Opal_SamplerFilterMode mode);
MTLSamplerMipFilter metal_helperToSamplerMipFilter(Opal_SamplerFilterMode mode);

MTLAccelerationStructureInstanceOptions metal_helperToAccelerationStructureInstanceOptions(Opal_AccelerationStructureInstanceFlags flags);

MTLStencilOperation metal_helperToStencilOperation(Opal_StencilOp op);
MTLCompareFunction metal_helperToCompareFunction(Opal_CompareOp op);

MTLAttributeFormat metal_helperToAttributeFormat(Opal_VertexFormat format);
MTLVertexFormat metal_helperToVertexFormat(Opal_VertexFormat format);
MTLVertexStepFunction metal_helperToVertexStepFunction(Opal_VertexInputRate rate);

MTLBlendFactor metal_helperToBlendFactor(Opal_BlendFactor factor);
MTLBlendOperation metal_helperToBlendOperation(Opal_BlendOp op);

MTLPrimitiveTopologyClass metal_helperToPrimitiveTopologyClass(Opal_PrimitiveType type);
MTLPrimitiveType metal_helperToPrimitiveType(Opal_PrimitiveType type);

MTLCullMode metal_helperToCullMode(Opal_CullMode mode);
MTLWinding metal_helperToWinding(Opal_FrontFace face);
MTLIndexType metal_helperToIndexType(Opal_IndexFormat format);
uint32_t metal_helperToIndexSize(Opal_IndexFormat format);

MTLLoadAction metal_helperToLoadAction(Opal_LoadOp op);
MTLStoreAction metal_helperToStoreAction(Opal_StoreOp op, uint32_t resolve);

CFStringRef metal_helperToColorspaceName(Opal_ColorSpace space);

MTLRenderStages metal_helperToRenderStages(Opal_BarrierStageFlags stages);

Opal_Result metal_allocatorInitialize(Metal_Device *device, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps);
Opal_Result metal_allocatorShutdown(Metal_Device *device);
Opal_Result metal_allocatorAllocateMemory(Metal_Device *device, const Metal_AllocationDesc *desc, uint32_t dedicated, Metal_Allocation *allocation);
Opal_Result metal_allocatorFreeMemory(Metal_Device *device, Metal_Allocation allocation);
