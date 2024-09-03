#include "webgpu_internal.h"

#include <emscripten.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
typedef struct WebGPU_AsyncRequest_t
{
	uint32_t finished;
	uint32_t status;
} WebGPU_AsyncRequest;

static void webgpu_bufferMapCallback(WGPUBufferMapAsyncStatus status, void *user_data)
{
	WebGPU_AsyncRequest *request = (WebGPU_AsyncRequest*)user_data;

	request->status = status;
	request->finished = 1;
}

static Opal_Result webgpu_mapBufferSync(WGPUBuffer buffer, WGPUMapMode mode, size_t offset, size_t size, void **result)
{
	assert(result);
	WebGPU_AsyncRequest request = {0};

	wgpuBufferMapAsync(buffer, mode, offset, size, webgpu_bufferMapCallback, &request);

	while (!request.finished)
		emscripten_sleep(0);

	if (request.status != WGPUBufferMapAsyncStatus_Success)
		return OPAL_WEBGPU_ERROR;

	void *mapped_ptr = wgpuBufferGetMappedRange(buffer, offset, size);
	if (mapped_ptr == NULL)
		return OPAL_WEBGPU_ERROR;

	*result = mapped_ptr;
	return OPAL_SUCCESS;
}

/*
 */
static void webgpu_destroyBuffer(WebGPU_Buffer *buffer_ptr)
{
	assert(buffer_ptr);

	if (buffer_ptr->map_count > 0)
		wgpuBufferUnmap(buffer_ptr->buffer);

	wgpuBufferDestroy(buffer_ptr->buffer);
	wgpuBufferRelease(buffer_ptr->buffer);
}

static void webgpu_destroyTexture(WebGPU_Texture *texture_ptr)
{
	assert(texture_ptr);

	wgpuTextureDestroy(texture_ptr->texture);
	wgpuTextureRelease(texture_ptr->texture);
}

static void webgpu_destroyTextureView(WebGPU_TextureView *texture_view_ptr)
{
	assert(texture_view_ptr);

	wgpuTextureViewRelease(texture_view_ptr->texture_view);
}

static void webgpu_destroySampler(WebGPU_Sampler *sampler_ptr)
{
	assert(sampler_ptr);

	wgpuSamplerRelease(sampler_ptr->sampler);
}

static void webgpu_destroyShader(WebGPU_Shader *shader_ptr)
{
	assert(shader_ptr);

	wgpuShaderModuleRelease(shader_ptr->shader);
}

static void webgpu_destroyBindsetLayout(WebGPU_BindsetLayout *layout_ptr)
{
	assert(layout_ptr);

	wgpuBindGroupLayoutRelease(layout_ptr->layout);
}

/*
 */
static Opal_Result webgpu_deviceGetInfo(Opal_Device this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	return webgpu_fillDeviceInfo(device_ptr->adapter, info);
}

static Opal_Result webgpu_deviceGetQueue(Opal_Device this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	assert(this);
	assert(queue);
	assert(engine_type < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	if (index != 0)
		return OPAL_INVALID_QUEUE_INDEX;

	if (engine_type != OPAL_DEVICE_ENGINE_TYPE_MAIN)
		return OPAL_INVALID_QUEUE_TYPE;

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;

	*queue = (Opal_Queue)device_ptr->queue;
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceGetAccelerationStructurePrebuildInfo(Opal_Device this, const Opal_AccelerationStructureBuildDesc *desc, Opal_AccelerationStructurePrebuildInfo *info)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(info);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceGetShaderBindingTablePrebuildInfo(Opal_Device this, const Opal_ShaderBindingTableLayoutDesc *desc, Opal_ShaderBindingTablePrebuildInfo *info)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(info);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateSemaphore(Opal_Device this, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(semaphore);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	assert(this);
	assert(desc);
	assert(buffer);

	{
		Opal_BufferUsageFlags unsupported_usages = 
			OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE |
			OPAL_BUFFER_USAGE_SHADER_BINDING_TABLE;

		if ((desc->usage & unsupported_usages) != 0)
			return OPAL_BUFFER_USAGE_NOT_SUPPORTED;
	}

	if (desc->memory_type != OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL)
	{
		Opal_BufferUsageFlags unsupported_usages = 
			OPAL_BUFFER_USAGE_VERTEX |
			OPAL_BUFFER_USAGE_INDEX |
			OPAL_BUFFER_USAGE_UNIFORM |
			OPAL_BUFFER_USAGE_STORAGE |
			OPAL_BUFFER_USAGE_INDIRECT;
		
		if ((desc->usage & unsupported_usages) != 0)
			return OPAL_BUFFER_USAGE_NOT_SUPPORTED;
	}

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WGPUBufferDescriptor buffer_info = {0};
	buffer_info.usage = webgpu_helperToBufferUsage(desc->usage, desc->memory_type);
	buffer_info.size = desc->size;

	WGPUBuffer webgpu_buffer = wgpuDeviceCreateBuffer(webgpu_device, &buffer_info);
	if (webgpu_buffer == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_Buffer result = {0};
	result.buffer = webgpu_buffer;
	result.size = desc->size;

	if (buffer_info.usage & WGPUBufferUsage_MapWrite)
		result.map_flags |= WGPUMapMode_Write;

	if (buffer_info.usage & WGPUBufferUsage_MapRead)
		result.map_flags |= WGPUMapMode_Read;

	*buffer = (Opal_Buffer)opal_poolAddElement(&device_ptr->buffers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	assert(this);
	assert(desc);
	assert(texture);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WGPUTextureFormat format = webgpu_helperToTextureFormat(desc->format);
	if (format == WGPUTextureFormat_Undefined && desc->format != OPAL_FORMAT_UNDEFINED)
		return OPAL_TEXTURE_FORMAT_NOT_SUPPORTED;

	WGPUTextureDescriptor texture_info = {0};
	texture_info.usage = webgpu_helperToTextureUsage(desc->usage);
	texture_info.dimension = webgpu_helperToTextureDimension(desc->type);
	texture_info.size.width = desc->width;
	texture_info.size.height = desc->height;
	texture_info.size.depthOrArrayLayers = (desc->type == OPAL_TEXTURE_TYPE_3D) ? desc->depth : desc->layer_count;
	texture_info.format = format;
	texture_info.mipLevelCount = desc->mip_count;
	texture_info.sampleCount = webgpu_helperToSampleCount(desc->samples);
	texture_info.viewFormatCount = 1;
	texture_info.viewFormats = &format;

	WGPUTexture webgpu_texture = wgpuDeviceCreateTexture(webgpu_device, &texture_info);
	if (webgpu_texture == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_Texture result = {0};
	result.texture = webgpu_texture;
	result.format = desc->format;

	*texture = (Opal_Texture)opal_poolAddElement(&device_ptr->textures, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	assert(this);
	assert(desc);
	assert(texture_view);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_Texture *texture_ptr = (WebGPU_Texture *)opal_poolGetElement(&device_ptr->textures, (Opal_PoolHandle)desc->texture);
	assert(texture_ptr);

	WGPUTextureViewDescriptor texture_view_info = {0};
	texture_view_info.format = webgpu_helperToTextureFormat(texture_ptr->format);
	texture_view_info.dimension = webgpu_helperToTextureViewDimension(desc->type);
	texture_view_info.baseMipLevel = desc->base_mip;
	texture_view_info.mipLevelCount = desc->mip_count;
	texture_view_info.baseArrayLayer = desc->base_layer;
	texture_view_info.arrayLayerCount = desc->layer_count;
	texture_view_info.aspect = webgpu_helperToTextureAspect(texture_ptr->format);

	WGPUTextureView webgpu_texture_view = wgpuTextureCreateView(texture_ptr->texture, &texture_view_info);
	if (webgpu_texture_view == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_TextureView result = {0};
	result.texture_view = webgpu_texture_view;

	*texture_view = (Opal_TextureView)opal_poolAddElement(&device_ptr->texture_views, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCreateSampler(Opal_Device this, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	assert(this);
	assert(desc);
	assert(sampler);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WGPUSamplerDescriptor sampler_info = {0};
	sampler_info.addressModeU = webgpu_helperToAddressMode(desc->address_mode_u);
	sampler_info.addressModeV = webgpu_helperToAddressMode(desc->address_mode_v);
	sampler_info.addressModeW = webgpu_helperToAddressMode(desc->address_mode_w);
	sampler_info.magFilter = webgpu_helperToFilterMode(desc->mag_filter);
	sampler_info.minFilter = webgpu_helperToFilterMode(desc->mag_filter);
	sampler_info.mipmapFilter = webgpu_helperToMipmapFilterMode(desc->mip_filter);
	sampler_info.lodMinClamp = desc->min_lod;
	sampler_info.lodMaxClamp = desc->max_lod;
	sampler_info.compare = webgpu_helperToCompareFunction(desc->compare_op);
	sampler_info.maxAnisotropy = (uint16_t)desc->max_anisotropy;

	WGPUSampler webgpu_sampler = wgpuDeviceCreateSampler(webgpu_device, &sampler_info);
	if (webgpu_sampler == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_Sampler result = {0};
	result.sampler = webgpu_sampler;

	*sampler = (Opal_Sampler)opal_poolAddElement(&device_ptr->samplers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCreateAccelerationStructure(Opal_Device this, const Opal_AccelerationStructureDesc *desc, Opal_AccelerationStructure *acceleration_structure)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(acceleration_structure);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateCommandPool(Opal_Device this, Opal_Queue queue, Opal_CommandPool *command_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateShader(Opal_Device this, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	assert(this);
	assert(desc);
	assert(shader);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	if (desc->type == OPAL_SHADER_SOURCE_TYPE_DXIL_BINARY || desc->type == OPAL_SHADER_SOURCE_TYPE_MSL)
		return OPAL_SHADER_SOURCE_NOT_SUPPORTED;

	WGPUShaderModuleSPIRVDescriptor shader_spirv_info = {0};
	shader_spirv_info.chain.sType = WGPUSType_ShaderModuleSPIRVDescriptor;
	shader_spirv_info.code = desc->data;
	shader_spirv_info.codeSize = desc->size;

	WGPUShaderModuleWGSLDescriptor shader_wgsl_info = {0};
	shader_wgsl_info.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
	shader_wgsl_info.code = (const char *)desc->data;

	WGPUShaderModuleDescriptor shader_info = {0};
	
	if (desc->type == OPAL_SHADER_SOURCE_TYPE_SPIRV_BINARY)
		shader_info.nextInChain = &shader_spirv_info.chain;

	else if (desc->type == OPAL_SHADER_SOURCE_TYPE_WGSL)
		shader_info.nextInChain = &shader_wgsl_info.chain;

	assert(shader_info.nextInChain);

	WGPUShaderModule webgpu_shader = wgpuDeviceCreateShaderModule(webgpu_device, &shader_info);

	WebGPU_Shader result = {0};
	result.shader = webgpu_shader;

	*shader = (Opal_Shader)opal_poolAddElement(&device_ptr->shaders, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCreateBindsetLayout(Opal_Device this, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout)
{
	assert(this);
	assert(num_bindings > 0);
	assert(bindings);
	assert(bindset_layout);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(WGPUBindGroupLayoutEntry) * num_bindings);

	WGPUBindGroupLayoutEntry *webgpu_bindings = (WGPUBindGroupLayoutEntry *)device_ptr->bump.data;
	memset(webgpu_bindings, 0, sizeof(WGPUBindGroupLayoutEntry) * num_bindings);

	for (uint32_t i = 0; i < num_bindings; ++i)
	{
		webgpu_bindings[i].binding = bindings[i].binding;
		webgpu_bindings[i].visibility = webgpu_helperToShaderStage(bindings[i].visibility);

		Opal_BindingType type = bindings[i].type;
		Opal_Format format = bindings[i].texture_format;

		switch (type)
		{
			case OPAL_BINDING_TYPE_UNIFORM_BUFFER:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER_READONLY:
			{
				webgpu_bindings[i].buffer.type = webgpu_helperToBindingBufferType(type);
			}
			break;

			case OPAL_BINDING_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER_DYNAMIC:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC:
			{
				webgpu_bindings[i].buffer.type = webgpu_helperToBindingBufferType(type);
				webgpu_bindings[i].buffer.hasDynamicOffset = 1;
			}
			break;

			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_1D:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_2D:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_2D_ARRAY:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_CUBE:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_CUBE_ARRAY:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_3D:
			{
				webgpu_bindings[i].texture.sampleType = webgpu_helperToBindingSampleType(format);
				webgpu_bindings[i].texture.viewDimension = webgpu_helperToBindingViewDimension(type);
			}
			break;

			case OPAL_BINDING_TYPE_MULTISAMPLED_TEXTURE_2D:
			{
				webgpu_bindings[i].texture.sampleType = webgpu_helperToBindingSampleType(format);
				webgpu_bindings[i].texture.viewDimension = WGPUTextureViewDimension_2D;
				webgpu_bindings[i].texture.multisampled = 1;
			}
			break;

			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_1D:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_2D:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_2D_ARRAY:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_3D:
			{
				webgpu_bindings[i].storageTexture.access = WGPUStorageTextureAccess_ReadWrite;
				webgpu_bindings[i].storageTexture.format = webgpu_helperToTextureFormat(format);
				webgpu_bindings[i].storageTexture.viewDimension = webgpu_helperToBindingViewDimension(type);
			}
			break;

			case OPAL_BINDING_TYPE_SAMPLER:
			{
				webgpu_bindings[i].sampler.type = WGPUSamplerBindingType_Filtering;
			}
			break;

			case OPAL_BINDING_TYPE_COMPARE_SAMPLER:
			{
				webgpu_bindings[i].sampler.type = WGPUSamplerBindingType_Comparison;
			}
			break;

			default:
				assert(false);
		}
	}

	WGPUBindGroupLayoutDescriptor layout_info = {0};
	layout_info.entryCount = num_bindings;
	layout_info.entries = webgpu_bindings;
	
	WGPUBindGroupLayout webgpu_layout = wgpuDeviceCreateBindGroupLayout(webgpu_device, &layout_info);

	WebGPU_BindsetLayout result = {0};
	result.layout = webgpu_layout;

	*bindset_layout = (Opal_BindsetLayout)opal_poolAddElement(&device_ptr->bindset_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCreateBindsetPool(Opal_Device this, Opal_BindsetLayout bindset_layout, uint32_t max_bindsets, Opal_BindsetPool *bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_layout);
	OPAL_UNUSED(max_bindsets);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(num_bindset_layouts);
	OPAL_UNUSED(bindset_layouts);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateMeshletPipeline(Opal_Device this, const Opal_MeshletPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateComputePipeline(Opal_Device this, const Opal_ComputePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateRaytracePipeline(Opal_Device this, const Opal_RaytracePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateSwapchain(Opal_Device this, const Opal_SwapchainDesc *desc, Opal_Swapchain *swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroySemaphore(Opal_Device this, Opal_Semaphore semaphore)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer)
{
	assert(this);
	assert(buffer);

	Opal_PoolHandle handle = (Opal_PoolHandle)buffer;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Buffer *buffer_ptr = (WebGPU_Buffer *)opal_poolGetElement(&device_ptr->buffers, handle);
	assert(buffer_ptr);

	opal_poolRemoveElement(&device_ptr->buffers, handle);

	webgpu_destroyBuffer(buffer_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceDestroyTexture(Opal_Device this, Opal_Texture texture)
{
	assert(this);
	assert(texture);

	Opal_PoolHandle handle = (Opal_PoolHandle)texture;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Texture *texture_ptr = (WebGPU_Texture *)opal_poolGetElement(&device_ptr->textures, handle);
	assert(texture_ptr);

	opal_poolRemoveElement(&device_ptr->textures, handle);

	webgpu_destroyTexture(texture_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	assert(this);
	assert(texture_view);

	Opal_PoolHandle handle = (Opal_PoolHandle)texture_view;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_TextureView *texture_view_ptr = (WebGPU_TextureView *)opal_poolGetElement(&device_ptr->texture_views, handle);
	assert(texture_view_ptr);

	opal_poolRemoveElement(&device_ptr->texture_views, handle);

	webgpu_destroyTextureView(texture_view_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceDestroySampler(Opal_Device this, Opal_Sampler sampler)
{
	assert(this);
	assert(sampler);

	Opal_PoolHandle handle = (Opal_PoolHandle)sampler;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Sampler *sampler_ptr = (WebGPU_Sampler *)opal_poolGetElement(&device_ptr->samplers, handle);
	assert(sampler_ptr);

	opal_poolRemoveElement(&device_ptr->samplers, handle);

	webgpu_destroySampler(sampler_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceDestroyAccelerationStructure(Opal_Device this, Opal_AccelerationStructure acceleration_structure)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(acceleration_structure);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyCommandPool(Opal_Device this, Opal_CommandPool command_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	assert(this);
	assert(shader);

	Opal_PoolHandle handle = (Opal_PoolHandle)shader;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Shader *shader_ptr = (WebGPU_Shader *)opal_poolGetElement(&device_ptr->shaders, handle);
	assert(shader_ptr);

	opal_poolRemoveElement(&device_ptr->shaders, handle);

	webgpu_destroyShader(shader_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceDestroyBindsetLayout(Opal_Device this, Opal_BindsetLayout bindset_layout)
{
	assert(this);
	assert(bindset_layout);

	Opal_PoolHandle handle = (Opal_PoolHandle)bindset_layout;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_BindsetLayout *layout_ptr = (WebGPU_BindsetLayout *)opal_poolGetElement(&device_ptr->bindset_layouts, handle);
	assert(layout_ptr);

	opal_poolRemoveElement(&device_ptr->bindset_layouts, handle);

	webgpu_destroyBindsetLayout(layout_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceDestroyBindsetPool(Opal_Device this, Opal_BindsetPool bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyPipeline(Opal_Device this, Opal_Pipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroySwapchain(Opal_Device this, Opal_Swapchain swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroy(Opal_Device this)
{
	assert(this);

	WebGPU_Device *ptr = (WebGPU_Device *)this;

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->bindset_layouts);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_BindsetLayout *layout_ptr = (WebGPU_BindsetLayout *)opal_poolGetElementByIndex(&ptr->bindset_layouts, head);
			webgpu_destroyBindsetLayout(layout_ptr);

			head = opal_poolGetNextIndex(&ptr->bindset_layouts, head);
		}

		opal_poolShutdown(&ptr->bindset_layouts);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->shaders);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_Shader *shader_ptr = (WebGPU_Shader *)opal_poolGetElementByIndex(&ptr->shaders, head);
			webgpu_destroyShader(shader_ptr);

			head = opal_poolGetNextIndex(&ptr->shaders, head);
		}

		opal_poolShutdown(&ptr->shaders);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->samplers);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_Sampler *sampler_ptr = (WebGPU_Sampler *)opal_poolGetElementByIndex(&ptr->samplers, head);
			webgpu_destroySampler(sampler_ptr);

			head = opal_poolGetNextIndex(&ptr->samplers, head);
		}

		opal_poolShutdown(&ptr->samplers);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->texture_views);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_TextureView *texture_view_ptr = (WebGPU_TextureView *)opal_poolGetElementByIndex(&ptr->texture_views, head);
			webgpu_destroyTextureView(texture_view_ptr);

			head = opal_poolGetNextIndex(&ptr->texture_views, head);
		}

		opal_poolShutdown(&ptr->texture_views);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->textures);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_Texture *texture_ptr = (WebGPU_Texture *)opal_poolGetElementByIndex(&ptr->textures, head);
			webgpu_destroyTexture(texture_ptr);

			head = opal_poolGetNextIndex(&ptr->textures, head);
		}

		opal_poolShutdown(&ptr->textures);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->buffers);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_Buffer *buffer_ptr = (WebGPU_Buffer *)opal_poolGetElementByIndex(&ptr->buffers, head);
			webgpu_destroyBuffer(buffer_ptr);

			head = opal_poolGetNextIndex(&ptr->buffers, head);
		}

		opal_poolShutdown(&ptr->buffers);
	}

	opal_bumpShutdown(&ptr->bump);

	wgpuDeviceDestroy(ptr->device);
	wgpuDeviceRelease(ptr->device);
	wgpuAdapterRelease(ptr->adapter);

	free(ptr);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceBuildShaderBindingTable(Opal_Device this, const Opal_ShaderBindingTableBuildDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceAllocateCommandBuffer(Opal_Device this, Opal_CommandPool command_pool, Opal_CommandBuffer *command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceFreeCommandBuffer(Opal_Device this, Opal_CommandPool command_pool, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceResetCommandPool(Opal_Device this, Opal_CommandPool command_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceResetCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceAllocateBindset(Opal_Device this, Opal_BindsetPool bindset_pool, Opal_Bindset *bindset)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);
	OPAL_UNUSED(bindset);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceFreeBindset(Opal_Device this, Opal_BindsetPool bindset_pool, Opal_Bindset bindset)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);
	OPAL_UNUSED(bindset);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceResetBindsetPool(Opal_Device this, Opal_BindsetPool bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr)
{
	assert(this);
	assert(buffer);
	assert(ptr);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Buffer *buffer_ptr = (WebGPU_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer);
	assert(buffer_ptr);

	if (buffer_ptr->map_flags == 0)
		return OPAL_BUFFER_UNMAPPABLE;

	if (buffer_ptr->map_count == 0)
	{
		Opal_Result result = webgpu_mapBufferSync(buffer_ptr->buffer, buffer_ptr->map_flags, 0, (size_t)buffer_ptr->size, &buffer_ptr->mapped_ptr);

		if (result != OPAL_SUCCESS)
			return result;
	}

	assert(buffer_ptr->mapped_ptr);

	buffer_ptr->map_count++;
	*ptr = buffer_ptr->mapped_ptr;

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer)
{
	assert(this);
	assert(buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Buffer *buffer_ptr = (WebGPU_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer);
	assert(buffer_ptr);

	if (buffer_ptr->map_flags == 0)
		return OPAL_BUFFER_UNMAPPABLE;

	assert(buffer_ptr->map_count > 0);
	buffer_ptr->map_count--;

	if (buffer_ptr->map_count == 0)
	{
		wgpuBufferUnmap(buffer_ptr->buffer);
		buffer_ptr->mapped_ptr = NULL;
	}
	
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceUpdateBindset(Opal_Device this, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset);
	OPAL_UNUSED(num_bindings);
	OPAL_UNUSED(bindings);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceBeginCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceQuerySemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t *value)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceSignalSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceWaitSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);
	OPAL_UNUSED(timeout_milliseconds);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceWaitQueue(Opal_Device this, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceWaitIdle(Opal_Device this)
{
	OPAL_UNUSED(this);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceSubmit(Opal_Device this, Opal_Queue queue, const Opal_SubmitDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceAcquire(Opal_Device this, Opal_Swapchain swapchain, Opal_TextureView *texture_view)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);
	OPAL_UNUSED(texture_view);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_devicePresent(Opal_Device this, Opal_Swapchain swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depth_stencil_attachment)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_color_attachments);
	OPAL_UNUSED(color_attachments);
	OPAL_UNUSED(depth_stencil_attachment);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBeginComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdEndComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBeginRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdEndRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Pipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetBindsets(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout, uint32_t num_bindsets, const Opal_Bindset *bindsets)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline_layout);
	OPAL_UNUSED(num_bindsets);
	OPAL_UNUSED(bindsets);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_vertex_buffers);
	OPAL_UNUSED(vertex_buffers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer, Opal_IndexFormat index_format)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(index_buffer);
	OPAL_UNUSED(index_format);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetViewport(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Viewport viewport)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(viewport);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetScissor(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(x);
	OPAL_UNUSED(y);
	OPAL_UNUSED(width);
	OPAL_UNUSED(height);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdDrawIndexedInstanced(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t base_index, uint32_t num_instances, uint32_t base_instance)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_indices);
	OPAL_UNUSED(base_index);
	OPAL_UNUSED(num_instances);
	OPAL_UNUSED(base_instance);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdMeshletDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_groups_x);
	OPAL_UNUSED(num_groups_y);
	OPAL_UNUSED(num_groups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdComputeDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_groups_x);
	OPAL_UNUSED(num_groups_y);
	OPAL_UNUSED(num_groups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdRaytraceDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView raygen_entry, Opal_BufferView hitgroup_entry, Opal_BufferView miss_entry, uint32_t width, uint32_t height, uint32_t depth)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(raygen_entry);
	OPAL_UNUSED(hitgroup_entry);
	OPAL_UNUSED(miss_entry);
	OPAL_UNUSED(width);
	OPAL_UNUSED(height);
	OPAL_UNUSED(depth);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBuildAccelerationStructures(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_build_descs, const Opal_AccelerationStructureBuildDesc *descs)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_build_descs);
	OPAL_UNUSED(descs);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyAccelerationStructure(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_AccelerationStructure src, Opal_AccelerationStructure dst, Opal_AccelerationStructureCopyMode mode)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(mode);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyAccelerationStructuresPostbuildInfo(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_src_acceleration_structures, const Opal_AccelerationStructure *src_acceleration_structures, Opal_BufferView dst_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_src_acceleration_structures);
	OPAL_UNUSED(src_acceleration_structures);
	OPAL_UNUSED(dst_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyBufferToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_TextureRegion dst)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyTextureToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferView dst)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBufferTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBufferQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBufferQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdTextureTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdTextureQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdTextureQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

/*
 */
static Opal_DeviceTable device_vtbl =
{
	webgpu_deviceGetInfo,
	webgpu_deviceGetQueue,
	webgpu_deviceGetAccelerationStructurePrebuildInfo,
	webgpu_deviceGetShaderBindingTablePrebuildInfo,

	webgpu_deviceCreateSemaphore,
	webgpu_deviceCreateBuffer,
	webgpu_deviceCreateTexture,
	webgpu_deviceCreateTextureView,
	webgpu_deviceCreateSampler,
	webgpu_deviceCreateAccelerationStructure,
	webgpu_deviceCreateCommandPool,
	webgpu_deviceCreateShader,
	webgpu_deviceCreateBindsetLayout,
	webgpu_deviceCreateBindsetPool,
	webgpu_deviceCreatePipelineLayout,
	webgpu_deviceCreateGraphicsPipeline,
	webgpu_deviceCreateMeshletPipeline,
	webgpu_deviceCreateComputePipeline,
	webgpu_deviceCreateRaytracePipeline,
	webgpu_deviceCreateSwapchain,

	webgpu_deviceDestroySemaphore,
	webgpu_deviceDestroyBuffer,
	webgpu_deviceDestroyTexture,
	webgpu_deviceDestroyTextureView,
	webgpu_deviceDestroySampler,
	webgpu_deviceDestroyAccelerationStructure,
	webgpu_deviceDestroyCommandPool,
	webgpu_deviceDestroyShader,
	webgpu_deviceDestroyBindsetLayout,
	webgpu_deviceDestroyBindsetPool,
	webgpu_deviceDestroyPipelineLayout,
	webgpu_deviceDestroyPipeline,
	webgpu_deviceDestroySwapchain,
	webgpu_deviceDestroy,

	webgpu_deviceBuildShaderBindingTable,
	webgpu_deviceAllocateCommandBuffer,
	webgpu_deviceFreeCommandBuffer,
	webgpu_deviceResetCommandPool,
	webgpu_deviceResetCommandBuffer,
	webgpu_deviceAllocateBindset,
	webgpu_deviceFreeBindset,
	webgpu_deviceResetBindsetPool,
	webgpu_deviceMapBuffer,
	webgpu_deviceUnmapBuffer,
	webgpu_deviceUpdateBindset,
	webgpu_deviceBeginCommandBuffer,
	webgpu_deviceEndCommandBuffer,
	webgpu_deviceQuerySemaphore,
	webgpu_deviceSignalSemaphore,
	webgpu_deviceWaitSemaphore,
	webgpu_deviceWaitQueue,
	webgpu_deviceWaitIdle,
	webgpu_deviceSubmit,
	webgpu_deviceAcquire,
	webgpu_devicePresent,

	webgpu_deviceCmdBeginGraphicsPass,
	webgpu_deviceCmdEndGraphicsPass,
	webgpu_deviceCmdBeginComputePass,
	webgpu_deviceCmdEndComputePass,
	webgpu_deviceCmdBeginRaytracePass,
	webgpu_deviceCmdEndRaytracePass,
	webgpu_deviceCmdSetPipeline,
	webgpu_deviceCmdSetBindsets,
	webgpu_deviceCmdSetVertexBuffers,
	webgpu_deviceCmdSetIndexBuffer,
	webgpu_deviceCmdSetViewport,
	webgpu_deviceCmdSetScissor,
	webgpu_deviceCmdDrawIndexedInstanced,
	webgpu_deviceCmdMeshletDispatch,
	webgpu_deviceCmdComputeDispatch,
	webgpu_deviceCmdRaytraceDispatch,
	webgpu_deviceCmdBuildAccelerationStructures,
	webgpu_deviceCmdCopyAccelerationStructure,
	webgpu_deviceCmdCopyAccelerationStructuresPostbuildInfo,
	webgpu_deviceCmdCopyBufferToBuffer,
	webgpu_deviceCmdCopyBufferToTexture,
	webgpu_deviceCmdCopyTextureToBuffer,
	webgpu_deviceCmdBufferTransitionBarrier,
	webgpu_deviceCmdBufferQueueGrabBarrier,
	webgpu_deviceCmdBufferQueueReleaseBarrier,
	webgpu_deviceCmdTextureTransitionBarrier,
	webgpu_deviceCmdTextureQueueGrabBarrier,
	webgpu_deviceCmdTextureQueueReleaseBarrier,
};

/*
 */
Opal_Result webgpu_deviceInitialize(WebGPU_Device *device_ptr, WebGPU_Instance *instance_ptr, WGPUAdapter adapter, WGPUDevice device, WGPUQueue queue)
{
	assert(device_ptr);
	assert(instance_ptr);
	assert(adapter);
	assert(device);
	assert(queue);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	device_ptr->adapter = adapter;
	device_ptr->device = device;
	device_ptr->queue = queue;

	// bump
	opal_bumpInitialize(&device_ptr->bump, 256);

	// pools
	opal_poolInitialize(&device_ptr->buffers, sizeof(WebGPU_Buffer), 32);
	opal_poolInitialize(&device_ptr->textures, sizeof(WebGPU_Texture), 32);
	opal_poolInitialize(&device_ptr->texture_views, sizeof(WebGPU_Texture), 32);
	opal_poolInitialize(&device_ptr->samplers, sizeof(WebGPU_Sampler), 32);
	opal_poolInitialize(&device_ptr->shaders, sizeof(WebGPU_Shader), 32);
	opal_poolInitialize(&device_ptr->bindset_layouts, sizeof(WebGPU_BindsetLayout), 32);
	// opal_poolInitialize(&device_ptr->bindset_pools, sizeof(WebGPU_BindsetPool), 32);
	// opal_poolInitialize(&device_ptr->bindsets, sizeof(WebGPU_Bindset), 32);
	// opal_poolInitialize(&device_ptr->command_pools, sizeof(WebGPU_CommandPool), 32);
	// opal_poolInitialize(&device_ptr->command_buffers, sizeof(WebGPU_CommandBuffer), 32);
	// opal_poolInitialize(&device_ptr->pipeline_layouts, sizeof(WebGPU_PipelineLayout), 32);
	// opal_poolInitialize(&device_ptr->pipelines, sizeof(WebGPU_Pipeline), 32);
	// opal_poolInitialize(&device_ptr->swapchains, sizeof(WebGPU_Swapchain), 32);

	return OPAL_SUCCESS;
}
