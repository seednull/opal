#include "webgpu_internal.h"

#include <emscripten.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
typedef struct WebGPU_BufferMapRequest_t
{
	uint32_t finished;
	uint32_t status;
} WebGPU_BufferMapRequest;

static void webgpu_bufferMapCallback(WGPUBufferMapAsyncStatus status, void *user_data)
{
	WebGPU_BufferMapRequest *request = (WebGPU_BufferMapRequest*)user_data;

	request->status = status;
	request->finished = 1;
}

static Opal_Result webgpu_mapBufferSync(WGPUBuffer buffer, WGPUMapMode mode, size_t offset, size_t size, void **result)
{
	assert(result);
	WebGPU_BufferMapRequest request = {0};

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
typedef struct WebGPU_QueueSubmitRequest_t
{
	Opal_Device device;
	Opal_Queue queue;
} WebGPU_QueueSubmitRequest;

static void webgpu_deviceOnSubmittedWorkDoneCallback(WGPUQueueWorkDoneStatus status, void *user_data)
{
	assert(user_data);

	WebGPU_QueueSubmitRequest *submit_info = (WebGPU_QueueSubmitRequest *)user_data;
	WebGPU_Device *device_ptr = (WebGPU_Device *)submit_info->device;
	assert(device_ptr);

	WebGPU_Queue *queue_ptr = (WebGPU_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)submit_info->queue);
	assert(queue_ptr);

	uint32_t num_signal_semaphores = 0;
	opal_ringRead(&queue_ptr->submit_ring, &num_signal_semaphores, sizeof(uint32_t));

	uint32_t num_signal_swapchains = 0;
	opal_ringRead(&queue_ptr->submit_ring, &num_signal_swapchains, sizeof(uint32_t));

	opal_bumpReset(&device_ptr->bump);
	uint32_t semaphores_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(Opal_Semaphore) * num_signal_semaphores);
	uint32_t semaphores_values_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(uint64_t) * num_signal_semaphores);
	uint32_t swapchains_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(Opal_Swapchain) * num_signal_swapchains);
	uint32_t swapchains_values_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(uint64_t) * num_signal_swapchains);

	Opal_Semaphore *signal_semaphores = (Opal_Semaphore *)(device_ptr->bump.data + semaphores_offset);
	uint64_t *signal_semaphores_values = (uint64_t *)(device_ptr->bump.data + semaphores_values_offset);
	Opal_Swapchain *signal_swapchains = (Opal_Swapchain *)(device_ptr->bump.data + swapchains_offset);
	uint64_t *signal_swapchains_values = (uint64_t *)(device_ptr->bump.data + swapchains_values_offset);

	opal_ringRead(&queue_ptr->submit_ring, signal_semaphores, sizeof(Opal_Semaphore) * num_signal_semaphores);
	opal_ringRead(&queue_ptr->submit_ring, signal_semaphores_values, sizeof(uint64_t) * num_signal_semaphores);
	opal_ringRead(&queue_ptr->submit_ring, signal_swapchains, sizeof(Opal_Swapchain) * num_signal_swapchains);
	opal_ringRead(&queue_ptr->submit_ring, signal_swapchains_values, sizeof(uint64_t) * num_signal_swapchains);

	for (uint32_t i = 0; i < num_signal_semaphores; ++i)
	{
		WebGPU_Semaphore *semaphore_ptr = (WebGPU_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)signal_semaphores[i]);
		assert(semaphore_ptr);
		assert(semaphore_ptr->value <= signal_semaphores_values[i]);

		semaphore_ptr->value = signal_semaphores_values[i];
	}

	for (uint32_t i = 0; i < num_signal_swapchains; ++i)
	{
		WebGPU_Swapchain *swapchain_ptr = (WebGPU_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)signal_swapchains[i]);
		assert(swapchain_ptr);
		assert(swapchain_ptr->semaphore_value <= signal_swapchains_values[i]);

		swapchain_ptr->semaphore_value = signal_swapchains_values[i];
	}
}

/*
 */
static void webgpu_destroyQueue(WebGPU_Queue *queue_ptr)
{
	assert(queue_ptr);

	wgpuQueueOnSubmittedWorkDone(queue_ptr->queue, NULL, NULL);
	wgpuQueueRelease(queue_ptr->queue);

	opal_ringShutdown(&queue_ptr->submit_ring);
	free(queue_ptr->submit_info);
}

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

static void webgpu_destroyCommandBuffer(WebGPU_CommandBuffer *buffer_ptr)
{
	assert(buffer_ptr);

	if (buffer_ptr->command_encoder)
		wgpuCommandEncoderRelease(buffer_ptr->command_encoder);

	if (buffer_ptr->render_pass_encoder)
		wgpuRenderPassEncoderRelease(buffer_ptr->render_pass_encoder);

	if (buffer_ptr->compute_pass_encoder)
		wgpuComputePassEncoderRelease(buffer_ptr->compute_pass_encoder);

	if (buffer_ptr->command_buffer)
		wgpuCommandBufferRelease(buffer_ptr->command_buffer);
}

static void webgpu_destroyShader(WebGPU_Shader *shader_ptr)
{
	assert(shader_ptr);

	wgpuShaderModuleRelease(shader_ptr->shader);
}

static void webgpu_destroyBindsetLayout(WebGPU_BindsetLayout *layout_ptr)
{
	assert(layout_ptr);

	free(layout_ptr->bindings);
	wgpuBindGroupLayoutRelease(layout_ptr->layout);
}

static void webgpu_destroyBindset(WebGPU_Bindset *bindset_ptr)
{
	assert(bindset_ptr);

	wgpuBindGroupRelease(bindset_ptr->bindset);
}

static void webgpu_destroyPipelineLayout(WebGPU_PipelineLayout *layout_ptr)
{
	assert(layout_ptr);

	wgpuPipelineLayoutRelease(layout_ptr->layout);
}

static void webgpu_destroyPipeline(WebGPU_Pipeline *pipeline_ptr)
{
	assert(pipeline_ptr);

	wgpuRenderPipelineRelease(pipeline_ptr->render_pipeline);
	wgpuComputePipelineRelease(pipeline_ptr->compute_pipeline);
}

static void webgpu_destroySwapchain(WebGPU_Swapchain *swapchain_ptr)
{
	assert(swapchain_ptr);

	wgpuSwapChainRelease(swapchain_ptr->swapchain);
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

	*queue = device_ptr->queue;
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
	assert(this);
	assert(desc);
	assert(semaphore);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Semaphore result = {0};
	result.value = desc->initial_value;

	*semaphore = (Opal_Semaphore)opal_poolAddElement(&device_ptr->semaphores, &result);
	return OPAL_SUCCESS;
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
	if (format == WGPUTextureFormat_Undefined && desc->format != OPAL_TEXTURE_FORMAT_UNDEFINED)
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
	result.aspect = webgpu_helperToTextureAspect(desc->format);
	result.dimension = texture_info.dimension;

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

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;
	assert(queue == device_ptr->queue);

	WebGPU_CommandPool result = {0};
	
	*command_pool = (Opal_CommandPool)opal_poolAddElement(&device_ptr->command_pools, &result);
	return OPAL_SUCCESS;
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
	{
		assert(desc->size % 4 == 0);
		shader_info.nextInChain = &shader_spirv_info.chain;
	}
	else if (desc->type == OPAL_SHADER_SOURCE_TYPE_WGSL)
	{
		assert(((const char *)desc->data)[desc->size - 1] == 0);
		shader_info.nextInChain = &shader_wgsl_info.chain;
	}

	assert(shader_info.nextInChain);

	WGPUShaderModule webgpu_shader = wgpuDeviceCreateShaderModule(webgpu_device, &shader_info);
	if (webgpu_shader == NULL)
		return OPAL_WEBGPU_ERROR;

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
		Opal_TextureFormat format = bindings[i].texture_format;

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
	if (webgpu_layout == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_BindsetLayoutBinding *layout_bindings = (WebGPU_BindsetLayoutBinding *)malloc(sizeof(WebGPU_BindsetLayoutBinding) * num_bindings);

	WebGPU_BindsetLayout result = {0};
	result.layout = webgpu_layout;
	result.num_bindings = num_bindings;
	result.bindings = layout_bindings;

	for (uint32_t i = 0; i < num_bindings; ++i)
	{
		Opal_BindingType type = bindings[i].type;

		layout_bindings[i].binding = bindings[i].binding;
		layout_bindings[i].type = type;

		result.bindings_requirements[type]++;
	}

	*bindset_layout = (Opal_BindsetLayout)opal_poolAddElement(&device_ptr->bindset_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCreateBindsetPool(Opal_Device this, const Opal_BindsetPoolDesc *desc, Opal_BindsetPool *bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(bindset_pool);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_BindsetPool result = {0};
	result.bindset_limit = desc->max_bindsets;
	
	for (uint32_t i = 0; i < desc->num_entries; ++i)
	{
		Opal_BindsetPoolEntry entry = desc->entries[i];
		result.bindings_limits[entry.type] = entry.count;
	}

	*bindset_pool = (Opal_BindsetPool)opal_poolAddElement(&device_ptr->bindset_pools, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout)
{
	assert(this);
	assert(pipeline_layout);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(WGPUBindGroupLayout) * num_bindset_layouts);

	WGPUBindGroupLayout *webgpu_layouts = (WGPUBindGroupLayout *)device_ptr->bump.data;
	memset(webgpu_layouts, 0, sizeof(WGPUBindGroupLayout) * num_bindset_layouts);

	for (uint32_t i = 0; i < num_bindset_layouts; ++i)
	{
		WebGPU_BindsetLayout *layout_ptr = (WebGPU_BindsetLayout *)opal_poolGetElement(&device_ptr->bindset_layouts, (Opal_PoolHandle)bindset_layouts[i]);
		assert(layout_ptr);

		webgpu_layouts[i] = layout_ptr->layout;
	}

	WGPUPipelineLayoutDescriptor layout_info = {0};
	layout_info.bindGroupLayoutCount = num_bindset_layouts;
	layout_info.bindGroupLayouts = webgpu_layouts;

	WGPUPipelineLayout webgpu_pipeline_layout = wgpuDeviceCreatePipelineLayout(webgpu_device, &layout_info);
	if (webgpu_pipeline_layout == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_PipelineLayout result = {0};
	result.layout = webgpu_pipeline_layout;

	*pipeline_layout = (Opal_PipelineLayout)opal_poolAddElement(&device_ptr->pipeline_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	assert(this);
	assert(desc);
	assert(pipeline);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_PipelineLayout *layout_ptr = (WebGPU_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)desc->pipeline_layout);
	assert(layout_ptr);

	WebGPU_Shader *vertex_shader_ptr = (WebGPU_Shader *)opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)desc->vertex_shader);
	assert(vertex_shader_ptr);

	WebGPU_Shader *fragment_shader_ptr = (WebGPU_Shader *)opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)desc->fragment_shader);
	assert(fragment_shader_ptr);

	uint32_t num_vertex_streams = desc->num_vertex_streams;
	uint32_t num_total_vertex_attributes = 0;

	for (uint32_t i = 0; i < num_vertex_streams; ++i)
		num_total_vertex_attributes += desc->vertex_streams[i].num_vertex_attributes;

	opal_bumpReset(&device_ptr->bump);
	uint32_t buffers_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(WGPUVertexBufferLayout) * num_vertex_streams);
	uint32_t attributes_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(WGPUVertexAttribute) * num_total_vertex_attributes);

	WGPUVertexBufferLayout *vertex_buffers = (WGPUVertexBufferLayout *)(device_ptr->bump.data + buffers_offset);
	WGPUVertexAttribute *vertex_attributes = (WGPUVertexAttribute *)(device_ptr->bump.data + attributes_offset);

	uint32_t attribute_offset = 0;
	for (uint32_t i = 0; i < num_vertex_streams; ++i)
	{
		uint32_t num_vertex_attributes = desc->vertex_streams[i].num_vertex_attributes;
		const Opal_VertexAttribute *opal_vertex_attributes = desc->vertex_streams[i].attributes;

		vertex_buffers[i].arrayStride = desc->vertex_streams[i].stride;
		vertex_buffers[i].stepMode = webgpu_helperToVertexStepMode(desc->vertex_streams[i].rate);
		vertex_buffers[i].attributeCount = num_vertex_attributes;
		vertex_buffers[i].attributes = &vertex_attributes[attribute_offset];

		for (uint32_t j = 0; j < num_vertex_attributes; ++j)
		{
			vertex_attributes[attribute_offset].format = webgpu_helperToVertexFormat(opal_vertex_attributes[j].format);
			vertex_attributes[attribute_offset].offset = opal_vertex_attributes[j].offset;
			vertex_attributes[attribute_offset].shaderLocation = attribute_offset;
			attribute_offset++;
		}
	}

	WGPUDepthStencilState depthstencil_state = {0};

	if (desc->depth_stencil_attachment_format)
		depthstencil_state.format = webgpu_helperToTextureFormat(*desc->depth_stencil_attachment_format);

	depthstencil_state.depthWriteEnabled = desc->depth_write;
	depthstencil_state.depthCompare = webgpu_helperToCompareFunction(desc->depth_compare_op);

	depthstencil_state.stencilFront.compare = webgpu_helperToCompareFunction(desc->stencil_front.compare_op);
	depthstencil_state.stencilFront.failOp = webgpu_helperToStencilOperation(desc->stencil_front.fail_op);
	depthstencil_state.stencilFront.depthFailOp = webgpu_helperToStencilOperation(desc->stencil_front.depth_fail_op);
	depthstencil_state.stencilFront.passOp = webgpu_helperToStencilOperation(desc->stencil_front.pass_op);

	depthstencil_state.stencilBack.compare = webgpu_helperToCompareFunction(desc->stencil_back.compare_op);
	depthstencil_state.stencilBack.failOp = webgpu_helperToStencilOperation(desc->stencil_back.fail_op);
	depthstencil_state.stencilBack.depthFailOp = webgpu_helperToStencilOperation(desc->stencil_back.depth_fail_op);
	depthstencil_state.stencilBack.passOp = webgpu_helperToStencilOperation(desc->stencil_back.pass_op);

	depthstencil_state.stencilReadMask = desc->stencil_read_mask;
	depthstencil_state.stencilWriteMask = desc->stencil_write_mask;

	WGPUColorTargetState color_targets[8];
	memset(color_targets, 0, sizeof(color_targets));

	WGPUBlendState blend_states[8];
	memset(blend_states, 0, sizeof(blend_states));

	for (uint32_t i = 0; i < desc->num_color_attachments; ++i)
	{
		blend_states[i].color.operation = webgpu_helperToBlendOperation(desc->color_blend_states[i].color_op);
		blend_states[i].color.srcFactor = webgpu_helperToBlendFactor(desc->color_blend_states[i].src_color);
		blend_states[i].color.dstFactor = webgpu_helperToBlendFactor(desc->color_blend_states[i].dst_color);
		blend_states[i].alpha.operation = webgpu_helperToBlendOperation(desc->color_blend_states[i].alpha_op);
		blend_states[i].alpha.srcFactor = webgpu_helperToBlendFactor(desc->color_blend_states[i].src_alpha);
		blend_states[i].alpha.dstFactor = webgpu_helperToBlendFactor(desc->color_blend_states[i].dst_alpha);

		color_targets[i].format = webgpu_helperToTextureFormat(desc->color_attachment_formats[i]);
		color_targets[i].writeMask = WGPUColorWriteMask_All;

		if (desc->color_blend_states[i].enable)
			color_targets[i].blend = &blend_states[i];

	}

	WGPUFragmentState fragment_state = {0};
	fragment_state.module = fragment_shader_ptr->shader;
	fragment_state.entryPoint = "main";
	fragment_state.targetCount = desc->num_color_attachments;
	fragment_state.targets = color_targets;

	WGPURenderPipelineDescriptor pipeline_info = {0};
	pipeline_info.layout = layout_ptr->layout;
	pipeline_info.vertex.module = vertex_shader_ptr->shader;
	pipeline_info.vertex.entryPoint = "main";
	pipeline_info.vertex.bufferCount = desc->num_vertex_streams;
	pipeline_info.vertex.buffers = vertex_buffers;

	pipeline_info.primitive.topology = webgpu_helperToPrimitiveTopology(desc->primitive_type);
	pipeline_info.primitive.frontFace = webgpu_helperToFrontFace(desc->front_face);
	pipeline_info.primitive.cullMode = webgpu_helperToCullMode(desc->cull_mode);

	if (desc->primitive_type == OPAL_PRIMITIVE_TYPE_LINE_STRIP || desc->primitive_type == OPAL_PRIMITIVE_TYPE_TRIANGLE_STRIP)
		pipeline_info.primitive.stripIndexFormat = webgpu_helperToIndexFormat(desc->strip_index_format);

	pipeline_info.multisample.count = webgpu_helperToSampleCount(desc->rasterization_samples);
	pipeline_info.multisample.mask = UINT32_MAX;

	if (desc->depth_enable)
		pipeline_info.depthStencil = &depthstencil_state;

	pipeline_info.fragment = &fragment_state;

	WGPURenderPipeline webgpu_pipeline = wgpuDeviceCreateRenderPipeline(webgpu_device, &pipeline_info);
	if (webgpu_pipeline == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_Pipeline result = {0};
	result.render_pipeline = webgpu_pipeline;

	*pipeline = (Opal_Pipeline)opal_poolAddElement(&device_ptr->pipelines, &result);
	return OPAL_SUCCESS;
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
	assert(this);
	assert(desc);
	assert(pipeline);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_PipelineLayout *layout_ptr = (WebGPU_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)desc->pipeline_layout);
	assert(layout_ptr);

	WebGPU_Shader *shader_ptr = (WebGPU_Shader *)opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)desc->compute_shader);
	assert(shader_ptr);

	WGPUComputePipelineDescriptor pipeline_info = {0};
	pipeline_info.layout = layout_ptr->layout;
	pipeline_info.compute.module = shader_ptr->shader;
	pipeline_info.compute.entryPoint = "main";

	WGPUComputePipeline webgpu_pipeline = wgpuDeviceCreateComputePipeline(webgpu_device, &pipeline_info);
	if (webgpu_pipeline == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_Pipeline result = {0};
	result.compute_pipeline = webgpu_pipeline;

	*pipeline = (Opal_Pipeline)opal_poolAddElement(&device_ptr->pipelines, &result);
	return OPAL_SUCCESS;
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
	assert(this);
	assert(desc);
	assert(swapchain);

	static Opal_TextureFormat supported_formats[] =
	{
		OPAL_TEXTURE_FORMAT_BGRA8_UNORM,
		OPAL_TEXTURE_FORMAT_RGBA8_UNORM,
		OPAL_TEXTURE_FORMAT_RGBA16_SFLOAT
	};
	static uint32_t num_supported_formats = sizeof(supported_formats) / sizeof(Opal_TextureFormat);

	uint32_t format_supported = 0;
	for (uint32_t i = 0; i < num_supported_formats; ++i)
	{
		if (desc->format == supported_formats[i])
		{
			format_supported = 1;
			break;
		}
	}

	if (format_supported == 0)
		return OPAL_SWAPCHAIN_FORMAT_NOT_SUPPORTED;

	if (desc->mode != OPAL_PRESENT_MODE_FIFO)
		return OPAL_SWAPCHAIN_PRESENT_MODE_NOT_SUPPORTED;

	if (desc->colorspace != OPAL_COLORSPACE_SRGB)
		return OPAL_SWAPCHAIN_COLORSPACE_NOT_SUPPORTED;

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Instance *instance_ptr = device_ptr->instance;
	WebGPU_Surface *surface_ptr = (WebGPU_Surface *)opal_poolGetElement(&instance_ptr->surfaces, (Opal_PoolHandle)desc->surface);
	assert(surface_ptr);

	WGPUAdapter webgpu_adapter = device_ptr->adapter;
	WGPUDevice webgpu_device = device_ptr->device;
	WGPUSurface webgpu_surface = surface_ptr->surface;

	WGPUSwapChainDescriptor swapchain_info = {0};
	swapchain_info.usage = webgpu_helperToTextureUsage(desc->usage);
	swapchain_info.format = webgpu_helperToTextureFormat(desc->format);
	swapchain_info.presentMode = webgpu_helperToPresentMode(desc->mode);

	WGPUSwapChain webgpu_swapchain = wgpuDeviceCreateSwapChain(webgpu_device, webgpu_surface, &swapchain_info);
	if (webgpu_swapchain == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_Swapchain result = {0};
	result.swapchain = webgpu_swapchain;
	result.current_texture_view = OPAL_NULL_HANDLE;

	*swapchain = (Opal_Pipeline)opal_poolAddElement(&device_ptr->swapchains, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceDestroySemaphore(Opal_Device this, Opal_Semaphore semaphore)
{
	assert(this);
	assert(semaphore);

	Opal_PoolHandle handle = (Opal_PoolHandle)semaphore;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	opal_poolRemoveElement(&device_ptr->semaphores, handle);

	return OPAL_SUCCESS;
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
	assert(this);
	assert(command_pool);

	Opal_PoolHandle handle = (Opal_PoolHandle)command_pool;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;

	opal_poolRemoveElement(&device_ptr->command_pools, handle);
	return OPAL_SUCCESS;
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
	assert(this);
	assert(bindset_pool);

	Opal_PoolHandle handle = (Opal_PoolHandle)bindset_pool;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;

	opal_poolRemoveElement(&device_ptr->bindset_pools, handle);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	assert(this);
	assert(pipeline_layout);

	Opal_PoolHandle handle = (Opal_PoolHandle)pipeline_layout;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_PipelineLayout *layout_ptr = (WebGPU_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, handle);
	assert(layout_ptr);

	opal_poolRemoveElement(&device_ptr->pipeline_layouts, handle);

	webgpu_destroyPipelineLayout(layout_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceDestroyPipeline(Opal_Device this, Opal_Pipeline pipeline)
{
	assert(this);
	assert(pipeline);

	Opal_PoolHandle handle = (Opal_PoolHandle)pipeline;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Pipeline *pipeline_ptr = (WebGPU_Pipeline *)opal_poolGetElement(&device_ptr->pipelines, handle);
	assert(pipeline_ptr);

	opal_poolRemoveElement(&device_ptr->pipelines, handle);

	webgpu_destroyPipeline(pipeline_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceDestroySwapchain(Opal_Device this, Opal_Swapchain swapchain)
{
	assert(this);
	assert(swapchain);

	Opal_PoolHandle handle = (Opal_PoolHandle)swapchain;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Swapchain *swapchain_ptr = (WebGPU_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, handle);
	assert(swapchain_ptr);

	opal_poolRemoveElement(&device_ptr->swapchains, handle);

	webgpu_destroySwapchain(swapchain_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceDestroy(Opal_Device this)
{
	assert(this);

	WebGPU_Device *ptr = (WebGPU_Device *)this;

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->swapchains);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_Swapchain *swapchain_ptr = (WebGPU_Swapchain *)opal_poolGetElementByIndex(&ptr->swapchains, head);
			webgpu_destroySwapchain(swapchain_ptr);

			head = opal_poolGetNextIndex(&ptr->swapchains, head);
		}

		opal_poolShutdown(&ptr->swapchains);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->pipelines);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_Pipeline *pipeline_ptr = (WebGPU_Pipeline *)opal_poolGetElementByIndex(&ptr->pipelines, head);
			webgpu_destroyPipeline(pipeline_ptr);

			head = opal_poolGetNextIndex(&ptr->pipelines, head);
		}

		opal_poolShutdown(&ptr->pipelines);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->command_buffers);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_CommandBuffer *buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElementByIndex(&ptr->command_buffers, head);
			webgpu_destroyCommandBuffer(buffer_ptr);

			head = opal_poolGetNextIndex(&ptr->command_buffers, head);
		}

		opal_poolShutdown(&ptr->command_buffers);
	}

	opal_poolShutdown(&ptr->command_pools);

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->pipeline_layouts);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_PipelineLayout *layout_ptr = (WebGPU_PipelineLayout *)opal_poolGetElementByIndex(&ptr->pipeline_layouts, head);
			webgpu_destroyPipelineLayout(layout_ptr);

			head = opal_poolGetNextIndex(&ptr->pipeline_layouts, head);
		}

		opal_poolShutdown(&ptr->pipeline_layouts);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->bindsets);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_Bindset *bindset_ptr = (WebGPU_Bindset *)opal_poolGetElementByIndex(&ptr->bindsets, head);
			webgpu_destroyBindset(bindset_ptr);

			head = opal_poolGetNextIndex(&ptr->bindsets, head);
		}

		opal_poolShutdown(&ptr->bindsets);
	}

	opal_poolShutdown(&ptr->bindset_pools);

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

	opal_poolShutdown(&ptr->semaphores);

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->queues);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			WebGPU_Queue *queue_ptr = (WebGPU_Queue *)opal_poolGetElementByIndex(&ptr->queues, head);
			webgpu_destroyQueue(queue_ptr);

			head = opal_poolGetNextIndex(&ptr->queues, head);
		}

		opal_poolShutdown(&ptr->queues);
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
	assert(this);
	assert(command_pool);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandPool *command_pool_ptr = (WebGPU_CommandPool *)opal_poolGetElement(&device_ptr->command_pools, (Opal_PoolHandle)command_pool);
	assert(command_pool_ptr);

	command_pool_ptr->command_buffer_usage++;

	WebGPU_CommandBuffer result = {0};

	*command_buffer = (Opal_CommandBuffer)opal_poolAddElement(&device_ptr->command_buffers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceFreeCommandBuffer(Opal_Device this, Opal_CommandPool command_pool, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_pool);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandPool *command_pool_ptr = (WebGPU_CommandPool *)opal_poolGetElement(&device_ptr->command_pools, (Opal_PoolHandle)command_pool);
	assert(command_pool_ptr);

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	assert(command_pool_ptr->command_buffer_usage > 0);
	command_pool_ptr->command_buffer_usage--;

	webgpu_destroyCommandBuffer(command_buffer_ptr);
	opal_poolRemoveElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceResetCommandPool(Opal_Device this, Opal_CommandPool command_pool)
{
	assert(this);
	assert(command_pool);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandPool *command_pool_ptr = (WebGPU_CommandPool *)opal_poolGetElement(&device_ptr->command_pools, (Opal_PoolHandle)command_pool);
	assert(command_pool_ptr);

	command_pool_ptr->command_buffer_usage = 0;

	// TODO: fix memory leak caused by orphaned WebGPU_CommandBuffer instances

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceResetCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	webgpu_destroyCommandBuffer(command_buffer_ptr);

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceAllocateEmptyBindset(Opal_Device this, Opal_BindsetLayout bindset_layout, Opal_BindsetPool bindset_pool, Opal_Bindset *bindset)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_layout);
	OPAL_UNUSED(bindset_pool);
	OPAL_UNUSED(bindset);
 
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceAllocatePrefilledBindset(Opal_Device this, Opal_BindsetLayout bindset_layout, Opal_BindsetPool bindset_pool, uint32_t num_bindings, const Opal_BindsetBinding *bindings, Opal_Bindset *bindset)
{
	assert(this);
	assert(bindset_layout);
	assert(bindset_pool);
	assert(num_bindings > 0);
	assert(bindings);
	assert(bindset);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_BindsetPool *bindset_pool_ptr = (WebGPU_BindsetPool *)opal_poolGetElement(&device_ptr->bindset_pools, (Opal_PoolHandle)bindset_pool);
	assert(bindset_pool_ptr);

	WebGPU_BindsetLayout *bindset_layout_ptr = (WebGPU_BindsetLayout *)opal_poolGetElement(&device_ptr->bindset_layouts, (Opal_PoolHandle)bindset_layout);
	assert(bindset_layout_ptr);

	if (bindset_pool_ptr->bindset_usage == bindset_pool_ptr->bindset_limit)
		return OPAL_NO_POOL_MEMORY;

	for (uint32_t i = 0; i < OPAL_BINDING_TYPE_ENUM_MAX; ++i)
	{
		uint32_t limit = bindset_pool_ptr->bindings_limits[i];
		uint32_t requirement = bindset_pool_ptr->bindings_usages[i] + bindset_layout_ptr->bindings_requirements[i];

		if (requirement > limit)
			return OPAL_NO_POOL_MEMORY;
	}

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(WGPUBindGroupEntry) * num_bindings);

	WGPUBindGroupEntry *webgpu_entries = (WGPUBindGroupEntry *)device_ptr->bump.data;
	memset(webgpu_entries, 0, sizeof(WGPUBindGroupEntry) * num_bindings);

	for (uint32_t i = 0; i < num_bindings; ++i)
	{
		uint32_t binding = bindings[i].binding;
		Opal_BindingType type = OPAL_BINDING_TYPE_ENUM_MAX;

		// TODO: replace naive O(N) layout <-> binding search loop by O(1) hashmap lookup
		for (uint32_t j = 0; j < bindset_layout_ptr->num_bindings; ++j)
		{
			if (bindset_layout_ptr->bindings[j].binding == binding)
			{
				type = bindset_layout_ptr->bindings[j].type;
				break;
			}
		}

		if (type == OPAL_BINDING_TYPE_ENUM_MAX)
			return OPAL_INVALID_BINDING_INDEX;

		webgpu_entries[i].binding = binding;


		switch (type)
		{
			case OPAL_BINDING_TYPE_UNIFORM_BUFFER:
			case OPAL_BINDING_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER_DYNAMIC:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER_READONLY:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC:
			{
				Opal_BufferView data = bindings[i].data.buffer_view;

				WebGPU_Buffer *buffer_ptr = (WebGPU_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)data.buffer);
				assert(buffer_ptr);

				webgpu_entries[i].buffer = buffer_ptr->buffer;
				webgpu_entries[i].size = data.size;
				webgpu_entries[i].offset = data.offset;
			}
			break;

			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_1D:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_2D:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_2D_ARRAY:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_CUBE:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_CUBE_ARRAY:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_3D:
			case OPAL_BINDING_TYPE_MULTISAMPLED_TEXTURE_2D:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_1D:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_2D:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_2D_ARRAY:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_3D:
			{
				Opal_TextureView data = bindings[i].data.texture_view;

				WebGPU_TextureView *texture_view_ptr = (WebGPU_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)data);
				assert(texture_view_ptr);

				webgpu_entries[i].textureView = texture_view_ptr->texture_view;
			}
			break;

			case OPAL_BINDING_TYPE_SAMPLER:
			case OPAL_BINDING_TYPE_COMPARE_SAMPLER:
			{
				Opal_Sampler data = bindings[i].data.sampler;

				WebGPU_Sampler *sampler_ptr = (WebGPU_Sampler *)opal_poolGetElement(&device_ptr->samplers, (Opal_PoolHandle)data);
				assert(sampler_ptr);

				webgpu_entries[i].sampler = sampler_ptr->sampler;
			}
			break;

			default:
				assert(0);
		}
	}

	WGPUBindGroupDescriptor group_info = {0};
	group_info.layout = bindset_layout_ptr->layout;
	group_info.entryCount = num_bindings;
	group_info.entries = webgpu_entries;

	WGPUBindGroup webgpu_bindset = wgpuDeviceCreateBindGroup(webgpu_device, &group_info);
	if (webgpu_bindset == NULL)
		return OPAL_WEBGPU_ERROR;

	bindset_pool_ptr->bindset_usage++;

	for (uint32_t i = 0; i < OPAL_BINDING_TYPE_ENUM_MAX; ++i)
		bindset_pool_ptr->bindings_usages[i] += bindset_layout_ptr->bindings_requirements[i];

	WebGPU_Bindset result = {0};
	result.bindset = webgpu_bindset;
	result.layout = bindset_layout;

	*bindset = (Opal_Bindset)opal_poolAddElement(&device_ptr->bindsets, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceFreeBindset(Opal_Device this, Opal_BindsetPool bindset_pool, Opal_Bindset bindset)
{
	assert(this);
	assert(bindset_pool);
	assert(bindset);
 
	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_BindsetPool *bindset_pool_ptr = (WebGPU_BindsetPool *)opal_poolGetElement(&device_ptr->bindset_pools, (Opal_PoolHandle)bindset_pool);
	assert(bindset_pool_ptr);

	WebGPU_Bindset *bindset_ptr = (WebGPU_Bindset *)opal_poolGetElement(&device_ptr->bindsets, (Opal_PoolHandle)bindset);
	assert(bindset_ptr);

	WebGPU_BindsetLayout *bindset_layout_ptr = (WebGPU_BindsetLayout *)opal_poolGetElement(&device_ptr->bindset_layouts, (Opal_PoolHandle)bindset_ptr->layout);
	assert(bindset_layout_ptr);

	assert(bindset_pool_ptr->bindset_usage > 0);
	bindset_pool_ptr->bindset_usage--;

	for (uint32_t i = 0; i < OPAL_BINDING_TYPE_ENUM_MAX; ++i)
	{
		assert(bindset_pool_ptr->bindings_usages[i] >= bindset_layout_ptr->bindings_requirements[i]);
		bindset_pool_ptr->bindings_usages[i] -= bindset_layout_ptr->bindings_requirements[i];
	}

	webgpu_destroyBindset(bindset_ptr);
	opal_poolRemoveElement(&device_ptr->bindsets, (Opal_PoolHandle)bindset);

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceResetBindsetPool(Opal_Device this, Opal_BindsetPool bindset_pool)
{
	assert(this);
	assert(bindset_pool);
 
	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_BindsetPool *bindset_pool_ptr = (WebGPU_BindsetPool *)opal_poolGetElement(&device_ptr->bindset_pools, (Opal_PoolHandle)bindset_pool);
	assert(bindset_pool_ptr);

	bindset_pool_ptr->bindset_usage = 0;

	for (uint32_t i = 0; i < OPAL_BINDING_TYPE_ENUM_MAX; ++i)
		bindset_pool_ptr->bindings_usages[i] = 0;

	// TODO: invalidate related WebGPU_Bindset instances
	// TODO: fix memory leak caused by orphaned WebGPU_Bindset instances

	return OPAL_SUCCESS;
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
	assert(this);
	assert(bindset);
	assert(num_bindings > 0);
	assert(bindings);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceBeginCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_encoder == NULL);
	
	WGPUCommandEncoder webgpu_encoder = wgpuDeviceCreateCommandEncoder(webgpu_device, NULL);
	if (webgpu_encoder == NULL)
		return OPAL_WEBGPU_ERROR;

	command_buffer_ptr->command_encoder = webgpu_encoder;
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_encoder);
	
	WGPUCommandBuffer webgpu_command_buffer = wgpuCommandEncoderFinish(command_buffer_ptr->command_encoder, NULL);
	if (webgpu_command_buffer == NULL)
		return OPAL_WEBGPU_ERROR;

	wgpuCommandEncoderRelease(command_buffer_ptr->command_encoder);

	command_buffer_ptr->command_encoder = NULL;
	command_buffer_ptr->command_buffer = webgpu_command_buffer;

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceQuerySemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t *value)
{
	assert(this);
	assert(semaphore);
	assert(value);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Semaphore *semaphore_ptr = (WebGPU_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);

	*value = semaphore_ptr->value;
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceSignalSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value)
{
	assert(this);
	assert(semaphore);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Semaphore *semaphore_ptr = (WebGPU_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);
	assert(semaphore_ptr->value <= value);

	semaphore_ptr->value = value;
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceWaitSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds)
{
	assert(this);
	assert(semaphore);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;

	WebGPU_Semaphore *semaphore_ptr = (WebGPU_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);

	uint64_t current_timeout = timeout_milliseconds;
	while (current_timeout-- > 0)
	{
		if (semaphore_ptr->value >= value)
			return OPAL_SUCCESS;

		emscripten_sleep(1);
	}

	return OPAL_WAIT_TIMEOUT;
}

static Opal_Result webgpu_deviceWaitQueue(Opal_Device this, Opal_Queue queue)
{
	assert(this);
	assert(queue);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WebGPU_Queue *queue_ptr = (WebGPU_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queue);
	assert(queue_ptr);

	while (opal_ringGetSize(&queue_ptr->submit_ring) != 0)
		emscripten_sleep(0);

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceWaitIdle(Opal_Device this)
{
	assert(this);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	return webgpu_deviceWaitQueue(this, device_ptr->queue);
}

static Opal_Result webgpu_deviceSubmit(Opal_Device this, Opal_Queue queue, const Opal_SubmitDesc *desc)
{
	assert(this);
	assert(queue);
	assert(desc);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	assert(queue == device_ptr->queue);

	WebGPU_Queue *queue_ptr = (WebGPU_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queue);
	assert(queue_ptr);

	WGPUQueue webgpu_queue = queue_ptr->queue;

	// ignore wait semaphores?

	opal_ringWrite(&queue_ptr->submit_ring, &desc->num_signal_semaphores, sizeof(uint32_t));
	opal_ringWrite(&queue_ptr->submit_ring, &desc->num_signal_swapchains, sizeof(uint32_t));
	opal_ringWrite(&queue_ptr->submit_ring, desc->signal_semaphores, sizeof(Opal_Semaphore) * desc->num_signal_semaphores);
	opal_ringWrite(&queue_ptr->submit_ring, desc->signal_values, sizeof(uint64_t) * desc->num_signal_semaphores);
	opal_ringWrite(&queue_ptr->submit_ring, desc->signal_swapchains, sizeof(Opal_Swapchain) * desc->num_signal_swapchains);

	for (uint32_t i = 0; i < desc->num_signal_swapchains; ++i)
	{
		WebGPU_Swapchain *swapchain_ptr = (WebGPU_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)desc->signal_swapchains[i]);
		assert(swapchain_ptr);

		opal_ringWrite(&queue_ptr->submit_ring, &swapchain_ptr->wait_value, sizeof(uint64_t));
	}

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(WGPUCommandBuffer) * desc->num_command_buffers);

	WGPUCommandBuffer *command_buffers = (WGPUCommandBuffer *)device_ptr->bump.data;
	for (uint32_t i = 0; i < desc->num_command_buffers; ++i)
	{
		WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)desc->command_buffers[i]);
		assert(command_buffer_ptr);
		assert(command_buffer_ptr->command_buffer);

		command_buffers[i] = command_buffer_ptr->command_buffer;
	}

	wgpuQueueSubmit(webgpu_queue, desc->num_command_buffers, command_buffers);
	wgpuQueueOnSubmittedWorkDone(webgpu_queue, webgpu_deviceOnSubmittedWorkDoneCallback, queue_ptr->submit_info);

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceAcquire(Opal_Device this, Opal_Swapchain swapchain, Opal_TextureView *texture_view)
{
	assert(this);
	assert(swapchain);
	assert(texture_view);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_Swapchain *swapchain_ptr = (WebGPU_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)swapchain);
	assert(swapchain_ptr);

	if (swapchain_ptr->current_texture_view != OPAL_NULL_HANDLE)
	{
		uint64_t max_timeout_frames = 1000;

		while (max_timeout_frames-- > 0)
		{
			if (swapchain_ptr->semaphore_value >= swapchain_ptr->wait_value)
				break;

			emscripten_sleep(0);
		}

		if (swapchain_ptr->semaphore_value < swapchain_ptr->wait_value)
			return OPAL_WAIT_TIMEOUT;

		webgpu_deviceDestroyTextureView(this, swapchain_ptr->current_texture_view);
		swapchain_ptr->current_texture_view = OPAL_NULL_HANDLE;
	}

	WGPUTextureView webgpu_texture_view = wgpuSwapChainGetCurrentTextureView(swapchain_ptr->swapchain);
	if (webgpu_texture_view == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_TextureView result = {0};
	result.texture_view = webgpu_texture_view;

	Opal_TextureView handle = (Opal_TextureView)opal_poolAddElement(&device_ptr->texture_views, &result);
	swapchain_ptr->current_texture_view = handle;
	swapchain_ptr->wait_value = swapchain_ptr->semaphore_value + 1;

	*texture_view = handle;
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_devicePresent(Opal_Device this, Opal_Swapchain swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depth_stencil_attachment)
{
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_encoder);

	WGPUCommandEncoder webgpu_encoder = command_buffer_ptr->command_encoder;

	WGPURenderPassColorAttachment *webgpu_colors = NULL;

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(WGPURenderPassColorAttachment) * num_color_attachments);

	if (num_color_attachments > 0)
		webgpu_colors = (WGPURenderPassColorAttachment *)device_ptr->bump.data;

	for (uint32_t i = 0; i < num_color_attachments; ++i)
	{
		const Opal_FramebufferAttachment *opal_attachment = &color_attachments[i];

		WebGPU_TextureView *texture_view_ptr = (WebGPU_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)opal_attachment->texture_view);
		assert(texture_view_ptr);

		WGPURenderPassColorAttachment *webgpu_color = &webgpu_colors[i];

		WebGPU_TextureView *resolve_texture_view_ptr = (WebGPU_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)opal_attachment->resolve_texture_view);

		webgpu_color->view = texture_view_ptr->texture_view;
		webgpu_color->resolveTarget = (resolve_texture_view_ptr) ? resolve_texture_view_ptr->texture_view : NULL;
		webgpu_color->depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
		webgpu_color->loadOp = webgpu_helperToLoadOp(opal_attachment->load_op);
		webgpu_color->storeOp = webgpu_helperToStoreOp(opal_attachment->store_op);
		webgpu_color->clearValue.r = opal_attachment->clear_value.color.f[0];
		webgpu_color->clearValue.g = opal_attachment->clear_value.color.f[1];
		webgpu_color->clearValue.b = opal_attachment->clear_value.color.f[2];
		webgpu_color->clearValue.a = opal_attachment->clear_value.color.f[3];
	}

	WGPURenderPassDepthStencilAttachment webgpu_depthstencil = {0};

	if (depth_stencil_attachment)
	{
		const Opal_FramebufferAttachment *opal_attachment = depth_stencil_attachment;

		WebGPU_TextureView *texture_view_ptr = (WebGPU_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)opal_attachment->texture_view);
		assert(texture_view_ptr);

		webgpu_depthstencil.view = texture_view_ptr->texture_view;
		webgpu_depthstencil.depthLoadOp = webgpu_helperToLoadOp(opal_attachment->load_op);
		webgpu_depthstencil.depthStoreOp = webgpu_helperToStoreOp(opal_attachment->store_op);
		webgpu_depthstencil.depthClearValue = opal_attachment->clear_value.depth_stencil.depth;
		webgpu_depthstencil.stencilLoadOp = webgpu_helperToLoadOp(opal_attachment->load_op);
		webgpu_depthstencil.stencilStoreOp = webgpu_helperToStoreOp(opal_attachment->store_op);
		webgpu_depthstencil.stencilClearValue = opal_attachment->clear_value.depth_stencil.stencil;
	}

	WGPURenderPassDescriptor pass_info = {0};
	pass_info.colorAttachmentCount = num_color_attachments;
	pass_info.colorAttachments = webgpu_colors;

	if (depth_stencil_attachment)
		pass_info.depthStencilAttachment = &webgpu_depthstencil;

	WGPURenderPassEncoder pass_encoder = wgpuCommandEncoderBeginRenderPass(webgpu_encoder, &pass_info);
	if (pass_encoder == NULL)
		return OPAL_WEBGPU_ERROR;

	command_buffer_ptr->render_pass_encoder = pass_encoder;
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->render_pass_encoder);

	WGPURenderPassEncoder webgpu_encoder = command_buffer_ptr->render_pass_encoder;

	wgpuRenderPassEncoderEnd(webgpu_encoder);
	wgpuRenderPassEncoderRelease(webgpu_encoder);

	command_buffer_ptr->render_pass_encoder = NULL;
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdBeginComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_encoder);

	WGPUCommandEncoder webgpu_encoder = command_buffer_ptr->command_encoder;

	WGPUComputePassEncoder pass_encoder = wgpuCommandEncoderBeginComputePass(webgpu_encoder, NULL);
	if (pass_encoder == NULL)
		return OPAL_WEBGPU_ERROR;

	command_buffer_ptr->compute_pass_encoder = pass_encoder;
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdEndComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->compute_pass_encoder);

	WGPUComputePassEncoder webgpu_encoder = command_buffer_ptr->compute_pass_encoder;

	wgpuComputePassEncoderEnd(webgpu_encoder);
	wgpuComputePassEncoderRelease(webgpu_encoder);

	command_buffer_ptr->compute_pass_encoder = NULL;
	return OPAL_SUCCESS;
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
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->render_pass_encoder || command_buffer_ptr->compute_pass_encoder);

	WebGPU_Pipeline *pipeline_ptr = (WebGPU_Pipeline *)opal_poolGetElement(&device_ptr->pipelines, (Opal_PoolHandle)pipeline);
	assert(pipeline_ptr);

	WGPURenderPassEncoder webgpu_render_encoder = command_buffer_ptr->render_pass_encoder;
	if (webgpu_render_encoder)
	{
		assert(pipeline_ptr->render_pipeline);
		wgpuRenderPassEncoderSetPipeline(webgpu_render_encoder, pipeline_ptr->render_pipeline);
	}

	WGPUComputePassEncoder webgpu_compute_encoder = command_buffer_ptr->compute_pass_encoder;
	if (webgpu_compute_encoder)
	{
		assert(pipeline_ptr->compute_pipeline);
		wgpuComputePassEncoderSetPipeline(webgpu_compute_encoder, pipeline_ptr->compute_pipeline);
	}

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdSetBindsets(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout, uint32_t num_bindsets, const Opal_Bindset *bindsets)
{
	assert(this);
	assert(command_buffer);
	assert(pipeline_layout);
	assert(num_bindsets > 0);
	assert(bindsets);

	OPAL_UNUSED(pipeline_layout);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->render_pass_encoder || command_buffer_ptr->compute_pass_encoder);

	WGPURenderPassEncoder webgpu_render_encoder = command_buffer_ptr->render_pass_encoder;
	if (webgpu_render_encoder)
	{
		for (uint32_t i = 0; i < num_bindsets; ++i)
		{
			WebGPU_Bindset *bindset_ptr = (WebGPU_Bindset *)opal_poolGetElement(&device_ptr->bindsets, (Opal_PoolHandle)bindsets[i]);
			assert(bindset_ptr);

			wgpuRenderPassEncoderSetBindGroup(webgpu_render_encoder, i, bindset_ptr->bindset, 0, NULL);
		}
	}

	WGPUComputePassEncoder webgpu_compute_encoder = command_buffer_ptr->compute_pass_encoder;
	if (webgpu_compute_encoder)
	{
		for (uint32_t i = 0; i < num_bindsets; ++i)
		{
			WebGPU_Bindset *bindset_ptr = (WebGPU_Bindset *)opal_poolGetElement(&device_ptr->bindsets, (Opal_PoolHandle)bindsets[i]);
			assert(bindset_ptr);

			wgpuComputePassEncoderSetBindGroup(webgpu_compute_encoder, i, bindset_ptr->bindset, 0, NULL);
		}
	}

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers)
{
	assert(this);
	assert(command_buffer);
	assert(num_vertex_buffers > 0);
	assert(vertex_buffers);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->render_pass_encoder);

	WGPURenderPassEncoder webgpu_render_encoder = command_buffer_ptr->render_pass_encoder;
	for (uint32_t i = 0; i < num_vertex_buffers; ++i)
	{
		Opal_BufferView view = vertex_buffers[i];

		WebGPU_Buffer *buffer_ptr = (WebGPU_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)view.buffer);
		assert(buffer_ptr);

		wgpuRenderPassEncoderSetVertexBuffer(webgpu_render_encoder, i, buffer_ptr->buffer, view.offset, view.size);
	}

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer, Opal_IndexFormat index_format)
{
	assert(this);
	assert(command_buffer);
	assert(index_buffer.buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->render_pass_encoder);

	WGPURenderPassEncoder webgpu_render_encoder = command_buffer_ptr->render_pass_encoder;

	WebGPU_Buffer *buffer_ptr = (WebGPU_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)index_buffer.buffer);
	assert(buffer_ptr);

	WGPUIndexFormat webgpu_index_format = webgpu_helperToIndexFormat(index_format);

	wgpuRenderPassEncoderSetIndexBuffer(webgpu_render_encoder, buffer_ptr->buffer, webgpu_index_format, index_buffer.offset, index_buffer.size);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdSetViewport(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Viewport viewport)
{
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->render_pass_encoder);

	WGPURenderPassEncoder webgpu_render_encoder = command_buffer_ptr->render_pass_encoder;

	wgpuRenderPassEncoderSetViewport(webgpu_render_encoder, viewport.x, viewport.y, viewport.width, viewport.height, viewport.min_depth, viewport.max_depth);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdSetScissor(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->render_pass_encoder);

	WGPURenderPassEncoder webgpu_render_encoder = command_buffer_ptr->render_pass_encoder;

	wgpuRenderPassEncoderSetScissorRect(webgpu_render_encoder, x, y, width, height);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdDraw(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertices, uint32_t num_instances, uint32_t base_vertex, uint32_t base_instance)
{
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->render_pass_encoder);

	WGPURenderPassEncoder webgpu_render_encoder = command_buffer_ptr->render_pass_encoder;

	wgpuRenderPassEncoderDraw(webgpu_render_encoder, num_vertices, num_instances, base_vertex, base_instance);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdDrawIndexed(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t num_instances, uint32_t base_index, int32_t vertex_offset, uint32_t base_instance)
{
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->render_pass_encoder);

	WGPURenderPassEncoder webgpu_render_encoder = command_buffer_ptr->render_pass_encoder;

	wgpuRenderPassEncoderDrawIndexed(webgpu_render_encoder, num_indices, num_instances, base_index, vertex_offset, base_instance);
	return OPAL_SUCCESS;
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
	assert(this);
	assert(command_buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->compute_pass_encoder);

	WGPUComputePassEncoder webgpu_compute_encoder = command_buffer_ptr->compute_pass_encoder;

	wgpuComputePassEncoderDispatchWorkgroups(webgpu_compute_encoder, num_groups_x, num_groups_y, num_groups_z);
	return OPAL_SUCCESS;
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
	assert(this);
	assert(command_buffer);
	assert(src.buffer);
	assert(dst.buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_encoder);

	WGPUCommandEncoder webgpu_encoder = command_buffer_ptr->command_encoder;

	WebGPU_Buffer *src_buffer_ptr = (WebGPU_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)src.buffer);
	assert(src_buffer_ptr);

	WebGPU_Buffer *dst_buffer_ptr = (WebGPU_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)dst.buffer);
	assert(dst_buffer_ptr);

	wgpuCommandEncoderCopyBufferToBuffer(webgpu_encoder, src_buffer_ptr->buffer, src.offset, dst_buffer_ptr->buffer, dst.offset, size);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferTextureRegion src, Opal_TextureRegion dst)
{
	assert(this);
	assert(command_buffer);
	assert(src.buffer);
	assert(dst.texture);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_encoder);

	WGPUCommandEncoder webgpu_encoder = command_buffer_ptr->command_encoder;

	WebGPU_Buffer *buffer_ptr = (WebGPU_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)src.buffer);
	assert(buffer_ptr);

	WebGPU_Texture *texture_ptr = (WebGPU_Texture *)opal_poolGetElement(&device_ptr->textures, (Opal_PoolHandle)dst.texture);
	assert(texture_ptr);

	WGPUImageCopyBuffer buffer_info = {0};
	buffer_info.layout.offset = src.offset;
	buffer_info.layout.bytesPerRow = src.row_size;
	buffer_info.layout.rowsPerImage = src.num_rows;
	buffer_info.buffer = buffer_ptr->buffer;

	WGPUImageCopyTexture texture_info = {0};
	texture_info.texture = texture_ptr->texture;
	texture_info.mipLevel = dst.base_mip;
	texture_info.origin.x = dst.offset_x;
	texture_info.origin.y = dst.offset_y;
	texture_info.origin.z = dst.offset_z;
	texture_info.aspect = texture_ptr->aspect;

	WGPUExtent3D copy_info = {0};
	copy_info.width = dst.width;
	copy_info.height = dst.height;
	copy_info.depthOrArrayLayers = (texture_ptr->dimension == WGPUTextureDimension_3D) ? dst.depth : dst.base_layer;

	wgpuCommandEncoderCopyBufferToTexture(webgpu_encoder, &buffer_info, &texture_info, &copy_info);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdCopyTextureToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferTextureRegion dst)
{
	assert(this);
	assert(command_buffer);
	assert(src.texture);
	assert(dst.buffer);

	WebGPU_Device *device_ptr = (WebGPU_Device *)this;
	WGPUDevice webgpu_device = device_ptr->device;

	WebGPU_CommandBuffer *command_buffer_ptr = (WebGPU_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_encoder);

	WGPUCommandEncoder webgpu_encoder = command_buffer_ptr->command_encoder;

	WebGPU_Texture *texture_ptr = (WebGPU_Texture *)opal_poolGetElement(&device_ptr->textures, (Opal_PoolHandle)src.texture);
	assert(texture_ptr);

	WebGPU_Buffer *buffer_ptr = (WebGPU_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)dst.buffer);
	assert(buffer_ptr);

	WGPUImageCopyBuffer buffer_info = {0};
	buffer_info.layout.offset = dst.offset;
	buffer_info.layout.bytesPerRow = dst.row_size;
	buffer_info.layout.rowsPerImage = dst.num_rows;
	buffer_info.buffer = buffer_ptr->buffer;

	WGPUImageCopyTexture texture_info = {0};
	texture_info.texture = texture_ptr->texture;
	texture_info.mipLevel = src.base_mip;
	texture_info.origin.x = src.offset_x;
	texture_info.origin.y = src.offset_y;
	texture_info.origin.z = src.offset_z;
	texture_info.aspect = texture_ptr->aspect;

	WGPUExtent3D copy_info = {0};
	copy_info.width = src.width;
	copy_info.height = src.height;
	copy_info.depthOrArrayLayers = (texture_ptr->dimension == WGPUTextureDimension_3D) ? src.depth : src.base_layer;

	wgpuCommandEncoderCopyTextureToBuffer(webgpu_encoder, &texture_info, &buffer_info, &copy_info);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdBufferTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdBufferQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdBufferQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdTextureTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdTextureQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(queue);

	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCmdTextureQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(queue);

	return OPAL_SUCCESS;
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
	webgpu_deviceAllocateEmptyBindset,
	webgpu_deviceAllocatePrefilledBindset,
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
	webgpu_deviceCmdDraw,
	webgpu_deviceCmdDrawIndexed,
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
	device_ptr->instance = instance_ptr;

	// data
	device_ptr->adapter = adapter;
	device_ptr->device = device;

	// bump
	opal_bumpInitialize(&device_ptr->bump, 256);

	// pools
	opal_poolInitialize(&device_ptr->queues, sizeof(WebGPU_Queue), 32);
	opal_poolInitialize(&device_ptr->semaphores, sizeof(WebGPU_Semaphore), 32);
	opal_poolInitialize(&device_ptr->buffers, sizeof(WebGPU_Buffer), 32);
	opal_poolInitialize(&device_ptr->textures, sizeof(WebGPU_Texture), 32);
	opal_poolInitialize(&device_ptr->texture_views, sizeof(WebGPU_Texture), 32);
	opal_poolInitialize(&device_ptr->samplers, sizeof(WebGPU_Sampler), 32);
	opal_poolInitialize(&device_ptr->command_pools, sizeof(WebGPU_CommandPool), 32);
	opal_poolInitialize(&device_ptr->command_buffers, sizeof(WebGPU_CommandBuffer), 32);
	opal_poolInitialize(&device_ptr->shaders, sizeof(WebGPU_Shader), 32);
	opal_poolInitialize(&device_ptr->bindset_layouts, sizeof(WebGPU_BindsetLayout), 32);
	opal_poolInitialize(&device_ptr->bindset_pools, sizeof(WebGPU_BindsetPool), 32);
	opal_poolInitialize(&device_ptr->bindsets, sizeof(WebGPU_Bindset), 32);
	opal_poolInitialize(&device_ptr->pipeline_layouts, sizeof(WebGPU_PipelineLayout), 32);
	opal_poolInitialize(&device_ptr->pipelines, sizeof(WebGPU_Pipeline), 32);
	opal_poolInitialize(&device_ptr->swapchains, sizeof(WebGPU_Swapchain), 32);

	// queues
	{
		WebGPU_QueueSubmitRequest *info = (WebGPU_QueueSubmitRequest *)malloc(sizeof(WebGPU_QueueSubmitRequest));
		info->device = (Opal_Device)device_ptr;

		WebGPU_Queue result = {0};
		result.queue = queue;
		result.submit_info = info;
		opal_ringInitialize(&result.submit_ring, 1024);
		
		info->queue = (Opal_Queue)opal_poolAddElement(&device_ptr->queues, &result);
		device_ptr->queue = info->queue;
	}

	return OPAL_SUCCESS;
}
