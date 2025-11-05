#pragma once

#include "opal_internal.h"

#include <webgpu/webgpu.h>

#include "common/bump.h"
#include "common/pool.h"
#include "common/ring.h"

typedef enum WebGPU_PassType_t
{
	WEBGPU_PASS_TYPE_NONE = 0,
	WEBGPU_PASS_TYPE_GRAPHICS,
	WEBGPU_PASS_TYPE_COMPUTE,
	WEBGPU_PASS_TYPE_COPY,

	WEBGPU_PASS_TYPE_ENUM_MAX,
	WEBGPU_PASS_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} WebGPU_PassType;

typedef struct WebGPU_Instance_t
{
	Opal_InstanceTable *vtbl;
	WGPUInstance instance;
	Opal_Pool surfaces;
} WebGPU_Instance;

typedef struct WebGPU_Device_t
{
	Opal_DeviceTable *vtbl;
	WebGPU_Instance *instance;

	WGPUAdapter adapter;
	WGPUDevice device;
	Opal_Queue queue;

	Opal_Bump bump;

	Opal_Pool queues;
	Opal_Pool semaphores;
	Opal_Pool buffers;
	Opal_Pool textures;
	Opal_Pool texture_views;
	Opal_Pool samplers;
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
} WebGPU_Device;

typedef struct WebGPU_Surface_t
{
	WGPUSurface surface;
} WebGPU_Surface;

typedef struct WebGPU_Queue_t
{
	WGPUQueue queue;
	Opal_Ring submit_ring;
	void *submit_info;
} WebGPU_Queue;

typedef struct WebGPU_Semaphore_t
{
	uint64_t value;
} WebGPU_Semaphore;

typedef struct WebGPU_Buffer_t
{
	WGPUBuffer buffer;
	uint64_t size;
	void *mapped_ptr;
	uint32_t map_count;
	WGPUMapModeFlags map_flags;
} WebGPU_Buffer;

typedef struct WebGPU_Texture_t
{
	WGPUTexture texture;
	Opal_TextureFormat format;
	WGPUTextureAspect aspect;
	WGPUTextureDimension dimension;
} WebGPU_Texture;

typedef struct WebGPU_TextureView_t
{
	WGPUTextureView texture_view;
	WGPUTexture texture;
	WGPUTextureAspect aspect;
	uint32_t base_mip;
} WebGPU_TextureView;

typedef struct WebGPU_Sampler_t
{
	WGPUSampler sampler;
} WebGPU_Sampler;

typedef struct WebGPU_CommandAllocator_t
{
	uint32_t command_buffer_usage;
} WebGPU_CommandAllocator;

typedef struct WebGPU_CommandBuffer_t
{
	WGPUCommandEncoder command_encoder;
	WGPURenderPassEncoder render_pass_encoder;
	WGPUComputePassEncoder compute_pass_encoder;
	WGPUCommandBuffer command_buffer;
	WebGPU_PassType pass;
	Opal_CommandAllocator command_allocator;
} WebGPU_CommandBuffer;

typedef struct WebGPU_Shader_t
{
	WGPUShaderModule shader;
} WebGPU_Shader;

typedef struct WebGPU_DescriptorSetLayoutBinding_t
{
	uint32_t binding;
	Opal_DescriptorType type;
} WebGPU_DescriptorSetLayoutBinding;

typedef struct WebGPU_DescriptorSetLayout_t
{
	WGPUBindGroupLayout layout;
	uint32_t num_resource_descriptors;
	uint32_t num_sampler_descriptors;
	uint32_t num_bindings;
	WebGPU_DescriptorSetLayoutBinding *bindings;
} WebGPU_DescriptorSetLayout;

typedef struct WebGPU_DescriptorHeap_t
{
	uint32_t resource_usage;
	uint32_t resource_limit;
	uint32_t sampler_usage;
	uint32_t sampler_limit;
} WebGPU_DescriptorHeap;

typedef struct WebGPU_DescriptorSet_t
{
	WGPUBindGroup group;
	Opal_DescriptorSetLayout layout;
	Opal_DescriptorHeap heap;
} WebGPU_DescriptorSet;

typedef struct WebGPU_PipelineLayout_t
{
	WGPUPipelineLayout layout;
} WebGPU_PipelineLayout;

typedef struct WebGPU_GraphicsPipeline_t
{
	WGPURenderPipeline pipeline;
} WebGPU_GraphicsPipeline;

typedef struct WebGPU_ComputePipeline_t
{
	WGPUComputePipeline pipeline;
} WebGPU_ComputePipeline;

typedef struct WebGPU_Swapchain_t
{
	WGPUSwapChain swapchain;
	Opal_TextureView current_texture_view;
	uint64_t semaphore_value;
	uint64_t wait_value;
} WebGPU_Swapchain;

WGPUBufferUsageFlags webgpu_helperToBufferUsage(Opal_BufferUsageFlags flags, Opal_AllocationMemoryType memory_type);

WGPUTextureUsageFlags webgpu_helperToTextureUsage(Opal_TextureUsageFlags flags);
WGPUTextureDimension webgpu_helperToTextureDimension(Opal_TextureType type);
WGPUTextureViewDimension webgpu_helperToTextureViewDimension(Opal_TextureViewType type);
WGPUTextureFormat webgpu_helperToTextureFormat(Opal_TextureFormat format);
Opal_TextureFormat webgpu_helperFromTextureFormat(WGPUTextureFormat format);
uint32_t webgpu_helperToSampleCount(Opal_Samples samples);
WGPUTextureAspect webgpu_helperToTextureAspect(Opal_TextureFormat format);

WGPUAddressMode webgpu_helperToAddressMode(Opal_SamplerAddressMode mode);
WGPUFilterMode webgpu_helperToFilterMode(Opal_SamplerFilterMode mode);
WGPUMipmapFilterMode webgpu_helperToMipmapFilterMode(Opal_SamplerFilterMode mode);

WGPUVertexStepMode webgpu_helperToVertexStepMode(Opal_VertexInputRate rate);
WGPUVertexFormat webgpu_helperToVertexFormat(Opal_VertexFormat format);

WGPUCompareFunction webgpu_helperToCompareFunction(Opal_CompareOp op);
WGPUStencilOperation webgpu_helperToStencilOperation(Opal_StencilOp op);

WGPUBlendOperation webgpu_helperToBlendOperation(Opal_BlendOp op);
WGPUBlendFactor webgpu_helperToBlendFactor(Opal_BlendFactor factor);

WGPUPrimitiveTopology webgpu_helperToPrimitiveTopology(Opal_PrimitiveType type);
WGPUFrontFace webgpu_helperToFrontFace(Opal_FrontFace face);
WGPUCullMode webgpu_helperToCullMode(Opal_CullMode mode);

WGPUPresentMode webgpu_helperToPresentMode(Opal_PresentMode mode);
Opal_PresentMode webgpu_helperFromPresentMode(WGPUPresentMode mode);

WGPUShaderStageFlags webgpu_helperToShaderStage(Opal_ShaderStage stage);

WGPUBufferBindingType webgpu_helperToBindingBufferType(Opal_DescriptorType type);
WGPUTextureSampleType webgpu_helperToBindingSampleType(Opal_TextureFormat format);
WGPUTextureViewDimension webgpu_helperToBindingViewDimension(Opal_DescriptorType type);

WGPULoadOp webgpu_helperToLoadOp(Opal_LoadOp op);
WGPUStoreOp webgpu_helperToStoreOp(Opal_StoreOp op);

WGPUIndexFormat webgpu_helperToIndexFormat(Opal_IndexFormat format);

Opal_Result webgpu_fillDeviceInfo(WGPUAdapter adapter, Opal_DeviceInfo *info);
Opal_Result webgpu_deviceInitialize(WebGPU_Device *device_ptr, WebGPU_Instance *instance_ptr, WGPUAdapter adapter, WGPUDevice device, WGPUQueue queue);
