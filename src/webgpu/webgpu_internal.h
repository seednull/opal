#pragma once

#include "opal_internal.h"

#include <webgpu/webgpu.h>

#include "common/bump.h"
#include "common/pool.h"

typedef struct WebGPU_Instance_t
{
	Opal_InstanceTable *vtbl;
	WGPUInstance instance;
	Opal_Pool surfaces;
} WebGPU_Instance;

typedef struct WebGPU_Device_t
{
	Opal_DeviceTable *vtbl;
	WGPUAdapter adapter;
	WGPUDevice device;
	WGPUQueue queue;

	Opal_Bump bump;

	Opal_Pool buffers;
	Opal_Pool textures;
	Opal_Pool texture_views;
	Opal_Pool samplers;
	Opal_Pool shaders;
	Opal_Pool bindset_layouts;
	// Opal_Pool bindset_pools;
	// Opal_Pool bindsets;
	// Opal_Pool command_pools;
	// Opal_Pool command_buffers;
	// Opal_Pool pipeline_layouts;
	// Opal_Pool pipelines;
	// Opal_Pool swapchains;
} WebGPU_Device;

typedef struct WebGPU_Surface_t
{
	WGPUSurface surface;
} WebGPU_Surface;

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
	Opal_Format format;
} WebGPU_Texture;

typedef struct WebGPU_TextureView_t
{
	WGPUTextureView texture_view;
} WebGPU_TextureView;

typedef struct WebGPU_Sampler_t
{
	WGPUSampler sampler;
} WebGPU_Sampler;

typedef struct WebGPU_Shader_t
{
	WGPUShaderModule shader;
} WebGPU_Shader;

typedef struct WebGPU_BindsetLayout_t
{
	WGPUBindGroupLayout layout;
} WebGPU_BindsetLayout;

WGPUBufferUsageFlags webgpu_helperToBufferUsage(Opal_BufferUsageFlags flags, Opal_AllocationMemoryType memory_type);

WGPUTextureUsageFlags webgpu_helperToTextureUsage(Opal_TextureUsageFlags flags);
WGPUTextureDimension webgpu_helperToTextureDimension(Opal_TextureType type);
WGPUTextureViewDimension webgpu_helperToTextureViewDimension(Opal_TextureViewType type);
WGPUTextureFormat webgpu_helperToTextureFormat(Opal_Format format);
uint32_t webgpu_helperToSampleCount(Opal_Samples samples);
WGPUTextureAspect webgpu_helperToTextureAspect(Opal_Format format);

WGPUAddressMode webgpu_helperToAddressMode(Opal_SamplerAddressMode mode);
WGPUFilterMode webgpu_helperToFilterMode(Opal_SamplerFilterMode mode);
WGPUMipmapFilterMode webgpu_helperToMipmapFilterMode(Opal_SamplerFilterMode mode);

WGPUCompareFunction webgpu_helperToCompareFunction(Opal_CompareOp op);

WGPUShaderStageFlags webgpu_helperToShaderStage(Opal_ShaderStage stage);

WGPUBufferBindingType webgpu_helperToBindingBufferType(Opal_BindingType type);
WGPUTextureSampleType webgpu_helperToBindingSampleType(Opal_Format format);
WGPUTextureViewDimension webgpu_helperToBindingViewDimension(Opal_BindingType type);

Opal_Result webgpu_deviceInitialize(WebGPU_Device *device_ptr, WebGPU_Instance *instance_ptr, WGPUAdapter adapter, WGPUDevice device, WGPUQueue queue);
Opal_Result webgpu_fillDeviceInfo(WGPUAdapter adapter, Opal_DeviceInfo *info);
