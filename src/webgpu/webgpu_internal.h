#pragma once

#include "opal_internal.h"

#include <webgpu/webgpu.h>

#include "common/bump.h"
#include "common/pool.h"
#include "common/ring.h"

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
	Opal_Pool command_pools;
	Opal_Pool command_buffers;
	Opal_Pool shaders;
	Opal_Pool bindset_layouts;
	Opal_Pool bindset_pools;
	Opal_Pool bindsets;
	Opal_Pool pipeline_layouts;
	Opal_Pool pipelines;
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
} WebGPU_TextureView;

typedef struct WebGPU_Sampler_t
{
	WGPUSampler sampler;
} WebGPU_Sampler;

typedef struct WebGPU_CommandPool_t
{
	uint32_t command_buffer_usage;
} WebGPU_CommandPool;

typedef struct WebGPU_CommandBuffer_t
{
	WGPUCommandEncoder command_encoder;
	WGPURenderPassEncoder render_pass_encoder;
	WGPUComputePassEncoder compute_pass_encoder;
	WGPUCommandBuffer command_buffer;
} WebGPU_CommandBuffer;

typedef struct WebGPU_Shader_t
{
	WGPUShaderModule shader;
} WebGPU_Shader;

typedef struct WebGPU_BindsetLayoutBinding_t
{
	uint32_t binding;
	Opal_BindingType type;
} WebGPU_BindsetLayoutBinding;

typedef struct WebGPU_BindsetLayout_t
{
	WGPUBindGroupLayout layout;
	uint32_t num_bindings;
	WebGPU_BindsetLayoutBinding *bindings;
	uint32_t bindings_requirements[OPAL_BINDING_TYPE_ENUM_MAX];
} WebGPU_BindsetLayout;

typedef struct WebGPU_BindsetPool_t
{
	uint32_t bindset_usage;
	uint32_t bindset_limit;
	uint32_t bindings_usages[OPAL_BINDING_TYPE_ENUM_MAX];
	uint32_t bindings_limits[OPAL_BINDING_TYPE_ENUM_MAX];
} WebGPU_BindsetPool;

typedef struct WebGPU_Bindset_t
{
	WGPUBindGroup bindset;
	Opal_BindsetLayout layout;
} WebGPU_Bindset;

typedef struct WebGPU_PipelineLayout_t
{
	WGPUPipelineLayout layout;
} WebGPU_PipelineLayout;

typedef struct WebGPU_Pipeline_t
{
	WGPURenderPipeline render_pipeline;
	WGPUComputePipeline compute_pipeline;
} WebGPU_Pipeline;

typedef struct WebGPU_Swapchain_t
{
	WGPUSwapChain swapchain;
	Opal_TextureView current_texture_view;
	uint32_t current_semaphore;
} WebGPU_Swapchain;

WGPUBufferUsageFlags webgpu_helperToBufferUsage(Opal_BufferUsageFlags flags, Opal_AllocationMemoryType memory_type);

WGPUTextureUsageFlags webgpu_helperToTextureUsage(Opal_TextureUsageFlags flags);
WGPUTextureDimension webgpu_helperToTextureDimension(Opal_TextureType type);
WGPUTextureViewDimension webgpu_helperToTextureViewDimension(Opal_TextureViewType type);
WGPUTextureFormat webgpu_helperToTextureFormat(Opal_TextureFormat format);
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

WGPUShaderStageFlags webgpu_helperToShaderStage(Opal_ShaderStage stage);

WGPUBufferBindingType webgpu_helperToBindingBufferType(Opal_BindingType type);
WGPUTextureSampleType webgpu_helperToBindingSampleType(Opal_TextureFormat format);
WGPUTextureViewDimension webgpu_helperToBindingViewDimension(Opal_BindingType type);

WGPULoadOp webgpu_helperToLoadOp(Opal_LoadOp op);
WGPUStoreOp webgpu_helperToStoreOp(Opal_StoreOp op);

WGPUIndexFormat webgpu_helperToIndexFormat(Opal_IndexFormat format);

Opal_Result webgpu_fillDeviceInfo(WGPUAdapter adapter, Opal_DeviceInfo *info);
Opal_Result webgpu_deviceInitialize(WebGPU_Device *device_ptr, WebGPU_Instance *instance_ptr, WGPUAdapter adapter, WGPUDevice device, WGPUQueue queue);
