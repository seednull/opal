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
	Opal_Pool buffers;
	Opal_Pool textures;
	Opal_Pool texture_views;
	Opal_Pool samplers;
	Opal_Pool command_allocators;
	Opal_Pool command_buffers;
	Opal_Pool shaders;
	Opal_Pool pipeline_layouts;
	Opal_Pool swapchains;

	Metal_Allocator allocator;
} Metal_Device;

typedef struct Metal_Queue_t
{
	id<MTLCommandQueue> queue;
} Metal_Queue;

typedef struct Metal_Buffer_t
{
	id<MTLBuffer> buffer;
	Metal_Allocation allocation;
} Metal_Buffer;

typedef struct Metal_Texture_t
{
	id<MTLTexture> texture;
	MTLPixelFormat format;
	Metal_Allocation allocation;
} Metal_Texture;

typedef struct Metal_TextureView_t
{
	id<MTLTexture> texture_view;
	Opal_Texture texture;
} Metal_TextureView;

typedef struct Metal_Sampler_t
{
	id<MTLSamplerState> sampler;
} Metal_Sampler;

typedef struct Metal_CommandAllocator_t
{
	Opal_Queue queue;
	uint32_t command_buffer_usage;
} Metal_CommandAllocator;

typedef struct Metal_CommandBuffer_t
{
	id<MTLCommandBuffer> command_buffer;
	id<MTLRenderCommandEncoder> graphics_pass_encoder;
	id<MTLComputeCommandEncoder> compute_pass_encoder;
	id<MTLBlitCommandEncoder> copy_pass_encoder;
	Opal_Queue queue;
} Metal_CommandBuffer;

typedef struct Metal_Shader_t
{
	id<MTLLibrary> library;
} Metal_Shader;

typedef struct Metal_PipelineLayout_t
{
	Opal_DescriptorSetLayout *layouts;
	uint32_t num_layouts;
} Metal_PipelineLayout;

typedef struct Metal_Swapchain_t
{
	Opal_Surface surface;
	id<MTLCommandQueue> queue;
	CGColorSpaceRef colorspace;
} Metal_Swapchain;

typedef struct Metal_Surface_t
{
	CAMetalLayer *layer;
} Metal_Surface;

Opal_Result metal_helperFillDeviceEnginesInfo(Metal_DeviceEnginesInfo *info);
Opal_Result metal_helperFillDeviceInfo(id<MTLDevice> metal_device, Opal_DeviceInfo *info);

MTLTextureType metal_helperToTextureType(Opal_TextureType type, Opal_Samples samples);
MTLTextureType metal_helperToTextureViewType(Opal_TextureViewType type);
MTLPixelFormat metal_helperToPixelFormat(Opal_TextureFormat format);
NSUInteger metal_helperToSampleCount(Opal_Samples samples);
MTLTextureUsage metal_helperToTextureUsage(Opal_TextureUsageFlags flags);

MTLSamplerAddressMode metal_helperToSamplerAddressMode(Opal_SamplerAddressMode mode);
MTLSamplerMinMagFilter metal_helperToSamplerMinMagFilter(Opal_SamplerFilterMode mode);
MTLSamplerMipFilter metal_helperToSamplerMipFilter(Opal_SamplerFilterMode mode);

MTLCompareFunction metal_helperToCompareFunction(Opal_CompareOp op);

CFStringRef metal_helperToColorspaceName(Opal_ColorSpace space);

Opal_Result metal_deviceInitialize(Metal_Device *device_ptr, Metal_Instance *instance_ptr, id<MTLDevice> metal_device);
Opal_Result metal_deviceAllocateMemory(Metal_Device *device_ptr, const Metal_AllocationDesc *desc, Metal_Allocation *allocation);

Opal_Result metal_allocatorInitialize(Metal_Device *device, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps);
Opal_Result metal_allocatorShutdown(Metal_Device *device);
Opal_Result metal_allocatorAllocateMemory(Metal_Device *device, const Metal_AllocationDesc *desc, uint32_t dedicated, Metal_Allocation *allocation);
Opal_Result metal_allocatorFreeMemory(Metal_Device *device, Metal_Allocation allocation);
