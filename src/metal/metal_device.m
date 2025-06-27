#include "metal_internal.h"

/*
 */
static Opal_Result metal_deviceFreeDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set);
static Opal_Result metal_deviceUpdateDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set, uint32_t num_entries, const Opal_DescriptorSetEntry *entries);

/*
 */
static void metal_destroyBuffer(Metal_Device *device_ptr, Metal_Buffer *buffer_ptr)
{
	assert(device_ptr);
	assert(buffer_ptr);

	[buffer_ptr->buffer release];
	metal_allocatorFreeMemory(device_ptr, buffer_ptr->allocation);
}

static void metal_destroyTexture(Metal_Device *device_ptr, Metal_Texture *texture_ptr)
{
	assert(device_ptr);
	assert(texture_ptr);

	[texture_ptr->texture release];
	metal_allocatorFreeMemory(device_ptr, texture_ptr->allocation);
}

static void metal_destroyTextureView(Metal_Device *device_ptr, Metal_TextureView *texture_view_ptr)
{
	assert(device_ptr);
	assert(texture_view_ptr);

	OPAL_UNUSED(device_ptr);
	OPAL_UNUSED(texture_view_ptr);

	[texture_view_ptr->texture_view release];
}

static void metal_destroySampler(Metal_Device *device_ptr, Metal_Sampler *sampler_ptr)
{
	assert(device_ptr);
	assert(sampler_ptr);

	[sampler_ptr->sampler release];
}

/*
 */
static Opal_Result metal_deviceGetInfo(Opal_Device this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	Metal_Device *ptr = (Metal_Device *)this;
	return metal_helperFillDeviceInfo(ptr->device, info);
}

static Opal_Result metal_deviceGetQueue(Opal_Device this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	assert(this);
	assert(queue);
	assert(engine_type < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceGetAccelerationStructurePrebuildInfo(Opal_Device this, const Opal_AccelerationStructureBuildDesc *desc, Opal_AccelerationStructurePrebuildInfo *info)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(info);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceGetShaderBindingTablePrebuildInfo(Opal_Device this, const Opal_ShaderBindingTableBuildDesc *desc, Opal_ShaderBindingTablePrebuildInfo *info)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(info);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceGetSupportedSurfaceFormats(Opal_Device this, Opal_Surface surface, uint32_t *num_formats, Opal_SurfaceFormat *formats)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(num_formats);
	OPAL_UNUSED(formats);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceGetSupportedPresentModes(Opal_Device this, Opal_Surface surface, uint32_t *num_present_modes, Opal_PresentMode *present_modes)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(num_present_modes);
	OPAL_UNUSED(present_modes);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceGetPreferredSurfaceFormat(Opal_Device this, Opal_Surface surface, Opal_SurfaceFormat *format)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(format);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceGetPreferredSurfacePresentMode(Opal_Device this, Opal_Surface surface, Opal_PresentMode *present_mode)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(present_mode);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateSemaphore(Opal_Device this, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(semaphore);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	assert(this);
	assert(desc);
	assert(buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;
	id<MTLDevice> metal_device = device_ptr->device;

	id<MTLBuffer> metal_buffer = nil;
	Metal_Allocation allocation = {0};

	// fill buffer info
	MTLResourceOptions options = 0;
	switch (desc->memory_type)
	{
		case OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL: options = MTLResourceStorageModePrivate | MTLResourceCPUCacheModeDefaultCache; break;
		case OPAL_ALLOCATION_MEMORY_TYPE_STREAM: options = MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined; break;
		case OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD: options = MTLResourceStorageModeShared | MTLResourceCPUCacheModeDefaultCache; break;
		case OPAL_ALLOCATION_MEMORY_TYPE_READBACK: options = MTLResourceStorageModeShared | MTLResourceCPUCacheModeDefaultCache; break;

		default: assert(false); return OPAL_METAL_ERROR;
	}

	MTLSizeAndAlign allocation_info = [metal_device heapBufferSizeAndAlignWithLength: desc->size options: options];

	// fill allocation info
	Metal_AllocationDesc allocation_desc = {0};
	allocation_desc.size = allocation_info.size;
	allocation_desc.alignment = allocation_info.align;
	allocation_desc.allocation_type = desc->memory_type;
	allocation_desc.hint = desc->hint;

	Opal_Result opal_result = metal_deviceAllocateMemory(device_ptr, &allocation_desc, &allocation);
	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	assert(allocation.offset % allocation_info.align == 0);

	metal_buffer = [allocation.memory newBufferWithLength: allocation_desc.size options: options offset: allocation.offset];
	if (!metal_buffer)
	{
		metal_allocatorFreeMemory(device_ptr, allocation);
		return OPAL_METAL_ERROR;
	}

	// create opal struct
	Metal_Buffer result = {0};
	result.buffer = metal_buffer;
	result.allocation = allocation;

	*buffer = (Opal_Buffer)opal_poolAddElement(&device_ptr->buffers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	assert(this);
	assert(desc);
	assert(texture);

	Metal_Device *device_ptr = (Metal_Device *)this;
	id<MTLDevice> metal_device = device_ptr->device;

	id<MTLTexture> metal_texture = nil;
	Metal_Allocation allocation = {0};

	// fill texture info
	MTLTextureDescriptor *info = [[MTLTextureDescriptor alloc] init];
	MTLPixelFormat metal_format = metal_helperToPixelFormat(desc->format);

	info.textureType = metal_helperToTextureType(desc->type, desc->samples);
	info.pixelFormat = metal_format;
	info.width = desc->width;
	info.height = desc->height;
	info.depth = desc->depth;
	info.mipmapLevelCount = desc->mip_count;
	info.sampleCount = metal_helperToSampleCount(desc->samples);
	info.arrayLength = desc->layer_count;
	info.resourceOptions = MTLResourceStorageModePrivate | MTLResourceCPUCacheModeDefaultCache;
	info.usage = metal_helperToTextureUsage(desc->usage);

	MTLSizeAndAlign allocation_info = [metal_device heapTextureSizeAndAlignWithDescriptor: info];

	// fill allocation info
	Metal_AllocationDesc allocation_desc = {0};
	allocation_desc.size = allocation_info.size;
	allocation_desc.alignment = allocation_info.align;
	allocation_desc.allocation_type = OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL;
	allocation_desc.hint = desc->hint;

	Opal_Result opal_result = metal_deviceAllocateMemory(device_ptr, &allocation_desc, &allocation);
	if (opal_result != OPAL_SUCCESS)
	{
		[info release];
		return opal_result;
	}

	assert(allocation.offset % allocation_info.align == 0);

	metal_texture = [allocation.memory newTextureWithDescriptor: info offset: allocation.offset];
	[info release];

	if (!metal_texture)
	{
		metal_allocatorFreeMemory(device_ptr, allocation);
		return OPAL_METAL_ERROR;
	}

	// create opal struct
	Metal_Texture result = {0};
	result.texture = metal_texture;
	result.format = metal_format;
	result.allocation = allocation;

	*texture = (Opal_Texture)opal_poolAddElement(&device_ptr->textures, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	assert(this);
	assert(desc);
	assert(texture_view);

	Metal_Device *device_ptr = (Metal_Device *)this;
	id<MTLDevice> metal_device = device_ptr->device;

	Metal_Texture *texture_ptr = (Metal_Texture *)opal_poolGetElement(&device_ptr->textures, (Opal_PoolHandle)desc->texture);
	assert(texture_ptr);

	id<MTLTexture> metal_texture_view = [texture_ptr->texture
		newTextureViewWithPixelFormat: texture_ptr->format
		textureType: metal_helperToTextureViewType(desc->type)
		levels: NSMakeRange(desc->base_mip, desc->mip_count)
		slices: NSMakeRange(desc->base_layer, desc->layer_count)
	];

	if (!metal_texture_view)
		return OPAL_METAL_ERROR;

	Metal_TextureView result = {0};
	result.texture_view = metal_texture_view;
	result.texture = desc->texture;

	*texture_view = (Opal_TextureView)opal_poolAddElement(&device_ptr->texture_views, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateSampler(Opal_Device this, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	assert(this);
	assert(desc);
	assert(sampler);

	Metal_Device *device_ptr = (Metal_Device *)this;
	id<MTLDevice> metal_device = device_ptr->device;

	id<MTLSamplerState> metal_sampler = nil;

	// fill sampler info
	MTLSamplerDescriptor *info = [[MTLSamplerDescriptor alloc] init];

	info.rAddressMode = metal_helperToSamplerAddressMode(desc->address_mode_u);
	info.sAddressMode = metal_helperToSamplerAddressMode(desc->address_mode_v);
	info.tAddressMode = metal_helperToSamplerAddressMode(desc->address_mode_w);
	info.minFilter = metal_helperToSamplerMinMagFilter(desc->min_filter);
	info.magFilter = metal_helperToSamplerMinMagFilter(desc->mag_filter);
	info.mipFilter = metal_helperToSamplerMipFilter(desc->mip_filter);
	info.lodMinClamp = desc->min_lod;
	info.lodMaxClamp = desc->max_lod;
	info.maxAnisotropy = desc->max_anisotropy;
	info.compareFunction = metal_helperToCompareFunction(desc->compare_op);
	info.supportArgumentBuffers = true;

	metal_sampler = [metal_device newSamplerStateWithDescriptor: info];
	[info release];

	if (!metal_sampler)
		return OPAL_METAL_ERROR;

	// create opal struct
	Metal_Sampler result = {0};
	result.sampler = metal_sampler;

	*sampler = (Opal_Texture)opal_poolAddElement(&device_ptr->samplers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateAccelerationStructure(Opal_Device this, const Opal_AccelerationStructureDesc *desc, Opal_AccelerationStructure *acceleration_structure)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(acceleration_structure);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateCommandAllocator(Opal_Device this, Opal_Queue queue, Opal_CommandAllocator *command_allocator)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(command_allocator);

	return OPAL_NOT_SUPPORTED;
}


static Opal_Result metal_deviceCreateCommandBuffer(Opal_Device this, Opal_CommandAllocator command_allocator, Opal_CommandBuffer *command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_allocator);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateShader(Opal_Device this, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(shader);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateDescriptorHeap(Opal_Device this, const Opal_DescriptorHeapDesc *desc, Opal_DescriptorHeap *descriptor_heap)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(descriptor_heap);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateDescriptorSetLayout(Opal_Device this, uint32_t num_entries, const Opal_DescriptorSetLayoutEntry *entries, Opal_DescriptorSetLayout *descriptor_set_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(num_entries);
	OPAL_UNUSED(entries);
	OPAL_UNUSED(descriptor_set_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_descriptor_set_layouts, const Opal_DescriptorSetLayout *descriptor_set_layouts, Opal_PipelineLayout *pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(num_descriptor_set_layouts);
	OPAL_UNUSED(descriptor_set_layouts);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateMeshletPipeline(Opal_Device this, const Opal_MeshletPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateComputePipeline(Opal_Device this, const Opal_ComputePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateRaytracePipeline(Opal_Device this, const Opal_RaytracePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateSwapchain(Opal_Device this, const Opal_SwapchainDesc *desc, Opal_Swapchain *swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroySemaphore(Opal_Device this, Opal_Semaphore semaphore)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer)
{
	assert(this);
	assert(buffer);

	Opal_PoolHandle handle = (Opal_PoolHandle)buffer;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_Device *device_ptr = (Metal_Device *)this;
	Metal_Buffer *buffer_ptr = (Metal_Buffer *)opal_poolGetElement(&device_ptr->buffers, handle);
	assert(buffer_ptr);

	opal_poolRemoveElement(&device_ptr->buffers, handle);

	metal_destroyBuffer(device_ptr, buffer_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroyTexture(Opal_Device this, Opal_Texture texture)
{
	assert(this);
	assert(texture);

	Opal_PoolHandle handle = (Opal_PoolHandle)texture;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_Device *device_ptr = (Metal_Device *)this;
	Metal_Texture *texture_ptr = (Metal_Texture *)opal_poolGetElement(&device_ptr->textures, handle);
	assert(texture_ptr);

	opal_poolRemoveElement(&device_ptr->textures, handle);

	metal_destroyTexture(device_ptr, texture_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	assert(this);
	assert(texture_view);

	Opal_PoolHandle handle = (Opal_PoolHandle)texture_view;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_Device *device_ptr = (Metal_Device *)this;
	Metal_TextureView *texture_view_ptr = (Metal_TextureView *)opal_poolGetElement(&device_ptr->texture_views, handle);
	assert(texture_view_ptr);

	opal_poolRemoveElement(&device_ptr->texture_views, handle);

	metal_destroyTextureView(device_ptr, texture_view_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroySampler(Opal_Device this, Opal_Sampler sampler)
{
	assert(this);
	assert(sampler);

	Opal_PoolHandle handle = (Opal_PoolHandle)sampler;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_Device *device_ptr = (Metal_Device *)this;
	Metal_Sampler *sampler_ptr = (Metal_Sampler *)opal_poolGetElement(&device_ptr->samplers, handle);
	assert(sampler_ptr);

	opal_poolRemoveElement(&device_ptr->samplers, handle);

	metal_destroySampler(device_ptr, sampler_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroyAccelerationStructure(Opal_Device this, Opal_AccelerationStructure acceleration_structure)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(acceleration_structure);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyCommandAllocator(Opal_Device this, Opal_CommandAllocator command_allocator)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_allocator);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyCommandBuffer(Opal_Device this, Opal_CommandAllocator command_allocator, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_allocator);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(shader);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyDescriptorHeap(Opal_Device this, Opal_DescriptorHeap descriptor_heap)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(descriptor_heap);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyDescriptorSetLayout(Opal_Device this, Opal_DescriptorSetLayout descriptor_set_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(descriptor_set_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyPipeline(Opal_Device this, Opal_Pipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroySwapchain(Opal_Device this, Opal_Swapchain swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroy(Opal_Device this)
{
	assert(this);

	Metal_Device *ptr = (Metal_Device *)this;

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->samplers);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_Sampler *sampler_ptr = (Metal_Sampler *)opal_poolGetElementByIndex(&ptr->samplers, head);
			metal_destroySampler(ptr, sampler_ptr);

			head = opal_poolGetNextIndex(&ptr->samplers, head);
		}

		opal_poolShutdown(&ptr->samplers);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->texture_views);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_TextureView *texture_view_ptr = (Metal_TextureView *)opal_poolGetElementByIndex(&ptr->texture_views, head);
			metal_destroyTextureView(ptr, texture_view_ptr);

			head = opal_poolGetNextIndex(&ptr->texture_views, head);
		}

		opal_poolShutdown(&ptr->texture_views);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->textures);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_Texture *texture_ptr = (Metal_Texture *)opal_poolGetElementByIndex(&ptr->textures, head);
			metal_destroyTexture(ptr, texture_ptr);

			head = opal_poolGetNextIndex(&ptr->textures, head);
		}

		opal_poolShutdown(&ptr->textures);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->buffers);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_Buffer *buffer_ptr = (Metal_Buffer *)opal_poolGetElementByIndex(&ptr->buffers, head);
			metal_destroyBuffer(ptr, buffer_ptr);

			head = opal_poolGetNextIndex(&ptr->buffers, head);
		}

		opal_poolShutdown(&ptr->buffers);
	}

	opal_bumpShutdown(&ptr->bump);

	Opal_Result result = metal_allocatorShutdown(ptr);
	assert(result == OPAL_SUCCESS);

	OPAL_UNUSED(result);

	[ptr->device release];

	free(ptr);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceBuildShaderBindingTable(Opal_Device this, const Opal_ShaderBindingTableBuildDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceBuildAccelerationStructureInstanceBuffer(Opal_Device this, const Opal_AccelerationStructureInstanceBufferBuildDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceResetCommandAllocator(Opal_Device this, Opal_CommandAllocator command_allocator)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_allocator);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceAllocateDescriptorSet(Opal_Device this, const Opal_DescriptorSetAllocationDesc *desc, Opal_DescriptorSet *descriptor_set)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(descriptor_set);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceFreeDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(descriptor_set);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr)
{
	assert(this);
	assert(buffer);
	assert(ptr);

	Metal_Device *device_ptr = (Metal_Device *)this;
	Metal_Buffer *buffer_ptr = (Metal_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer);
	assert(buffer_ptr);

	void *buffer_data = buffer_ptr->buffer.contents;
	if (buffer_data == NULL)
		return OPAL_METAL_ERROR;

	*ptr = buffer_data;
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer)
{
	assert(this);
	assert(buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;
	Metal_Buffer *buffer_ptr = (Metal_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer);
	assert(buffer_ptr);
	OPAL_UNUSED(buffer_ptr);

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceWriteBuffer(Opal_Device this, Opal_Buffer buffer, uint64_t offset, const void *data, uint64_t size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(offset);
	OPAL_UNUSED(data);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceUpdateDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set, uint32_t num_entries, const Opal_DescriptorSetEntry *entries)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(descriptor_set);
	OPAL_UNUSED(num_entries);
	OPAL_UNUSED(entries);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceBeginCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceQuerySemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t *value)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceSignalSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceWaitSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);
	OPAL_UNUSED(timeout_milliseconds);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceWaitQueue(Opal_Device this, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceWaitIdle(Opal_Device this)
{
	OPAL_UNUSED(this);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceSubmit(Opal_Device this, Opal_Queue queue, const Opal_SubmitDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceAcquire(Opal_Device this, Opal_Swapchain swapchain, Opal_TextureView *texture_view)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);
	OPAL_UNUSED(texture_view);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_devicePresent(Opal_Device this, Opal_Swapchain swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depth_stencil_attachment)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_color_attachments);
	OPAL_UNUSED(color_attachments);
	OPAL_UNUSED(depth_stencil_attachment);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdBeginComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdEndComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdBeginRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdEndRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Pipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetDescriptorHeap(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_DescriptorHeap descriptor_heap)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(descriptor_heap);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetDescriptorSet(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline_layout);
	OPAL_UNUSED(index);
	OPAL_UNUSED(descriptor_set);
	OPAL_UNUSED(num_dynamic_offsets);
	OPAL_UNUSED(dynamic_offsets);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_VertexBufferView *vertex_buffers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_vertex_buffers);
	OPAL_UNUSED(vertex_buffers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_IndexBufferView index_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(index_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetViewport(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Viewport viewport)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(viewport);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetScissor(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(x);
	OPAL_UNUSED(y);
	OPAL_UNUSED(width);
	OPAL_UNUSED(height);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdDraw(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertices, uint32_t num_instances, uint32_t base_vertex, uint32_t base_instance)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_vertices);
	OPAL_UNUSED(num_instances);
	OPAL_UNUSED(base_vertex);
	OPAL_UNUSED(base_instance);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdDrawIndexed(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t num_instances, uint32_t base_index, int32_t vertex_offset, uint32_t base_instance)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_indices);
	OPAL_UNUSED(num_instances);
	OPAL_UNUSED(base_index);
	OPAL_UNUSED(vertex_offset);
	OPAL_UNUSED(base_instance);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdMeshletDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_groups_x);
	OPAL_UNUSED(num_groups_y);
	OPAL_UNUSED(num_groups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdComputeDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_groups_x);
	OPAL_UNUSED(num_groups_y);
	OPAL_UNUSED(num_groups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdRaytraceDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView raygen_entry, Opal_BufferView hitgroup_entry, Opal_BufferView miss_entry, uint32_t width, uint32_t height, uint32_t depth)
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

static Opal_Result metal_deviceCmdBuildAccelerationStructures(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_build_descs, const Opal_AccelerationStructureBuildDesc *descs)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_build_descs);
	OPAL_UNUSED(descs);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdCopyAccelerationStructure(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_AccelerationStructure src, Opal_AccelerationStructure dst, Opal_AccelerationStructureCopyMode mode)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(mode);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdCopyAccelerationStructuresPostbuildInfo(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_src_acceleration_structures, const Opal_AccelerationStructure *src_acceleration_structures, Opal_BufferView dst_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_src_acceleration_structures);
	OPAL_UNUSED(src_acceleration_structures);
	OPAL_UNUSED(dst_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdCopyBufferToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Buffer src_buffer, uint64_t src_offset, Opal_Buffer dst_buffer, uint64_t dst_offset, uint64_t size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src_buffer);
	OPAL_UNUSED(src_offset);
	OPAL_UNUSED(dst_buffer);
	OPAL_UNUSED(dst_offset);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferTextureRegion src, Opal_TextureRegion dst, Opal_Extent3D size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdCopyTextureToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferTextureRegion dst, Opal_Extent3D size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdCopyTextureToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_TextureRegion dst, Opal_Extent3D size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdBufferTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdBufferQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdBufferQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdTextureTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdTextureQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdTextureQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
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
	metal_deviceGetInfo,
	metal_deviceGetQueue,
	metal_deviceGetAccelerationStructurePrebuildInfo,
	metal_deviceGetShaderBindingTablePrebuildInfo,

	metal_deviceGetSupportedSurfaceFormats,
	metal_deviceGetSupportedPresentModes,
	metal_deviceGetPreferredSurfaceFormat,
	metal_deviceGetPreferredSurfacePresentMode,

	metal_deviceCreateSemaphore,
	metal_deviceCreateBuffer,
	metal_deviceCreateTexture,
	metal_deviceCreateTextureView,
	metal_deviceCreateSampler,
	metal_deviceCreateAccelerationStructure,
	metal_deviceCreateCommandAllocator,
	metal_deviceCreateCommandBuffer,
	metal_deviceCreateShader,
	metal_deviceCreateDescriptorHeap,
	metal_deviceCreateDescriptorSetLayout,
	metal_deviceCreatePipelineLayout,
	metal_deviceCreateGraphicsPipeline,
	metal_deviceCreateMeshletPipeline,
	metal_deviceCreateComputePipeline,
	metal_deviceCreateRaytracePipeline,
	metal_deviceCreateSwapchain,

	metal_deviceDestroySemaphore,
	metal_deviceDestroyBuffer,
	metal_deviceDestroyTexture,
	metal_deviceDestroyTextureView,
	metal_deviceDestroySampler,
	metal_deviceDestroyAccelerationStructure,
	metal_deviceDestroyCommandAllocator,
	metal_deviceDestroyCommandBuffer,
	metal_deviceDestroyShader,
	metal_deviceDestroyDescriptorHeap,
	metal_deviceDestroyDescriptorSetLayout,
	metal_deviceDestroyPipelineLayout,
	metal_deviceDestroyPipeline,
	metal_deviceDestroySwapchain,
	metal_deviceDestroy,

	metal_deviceBuildShaderBindingTable,
	metal_deviceBuildAccelerationStructureInstanceBuffer,
	metal_deviceResetCommandAllocator,
	metal_deviceAllocateDescriptorSet,
	metal_deviceFreeDescriptorSet,
	metal_deviceMapBuffer,
	metal_deviceUnmapBuffer,
	metal_deviceWriteBuffer,
	metal_deviceUpdateDescriptorSet,
	metal_deviceBeginCommandBuffer,
	metal_deviceEndCommandBuffer,
	metal_deviceQuerySemaphore,
	metal_deviceSignalSemaphore,
	metal_deviceWaitSemaphore,
	metal_deviceWaitQueue,
	metal_deviceWaitIdle,
	metal_deviceSubmit,
	metal_deviceAcquire,
	metal_devicePresent,

	metal_deviceCmdBeginGraphicsPass,
	metal_deviceCmdEndGraphicsPass,
	metal_deviceCmdBeginComputePass,
	metal_deviceCmdEndComputePass,
	metal_deviceCmdBeginRaytracePass,
	metal_deviceCmdEndRaytracePass,
	metal_deviceCmdSetPipeline,
	metal_deviceCmdSetDescriptorHeap,
	metal_deviceCmdSetDescriptorSet,
	metal_deviceCmdSetVertexBuffers,
	metal_deviceCmdSetIndexBuffer,
	metal_deviceCmdSetViewport,
	metal_deviceCmdSetScissor,
	metal_deviceCmdDraw,
	metal_deviceCmdDrawIndexed,
	metal_deviceCmdMeshletDispatch,
	metal_deviceCmdComputeDispatch,
	metal_deviceCmdRaytraceDispatch,
	metal_deviceCmdBuildAccelerationStructures,
	metal_deviceCmdCopyAccelerationStructure,
	metal_deviceCmdCopyAccelerationStructuresPostbuildInfo,
	metal_deviceCmdCopyBufferToBuffer,
	metal_deviceCmdCopyBufferToTexture,
	metal_deviceCmdCopyTextureToBuffer,
	metal_deviceCmdCopyTextureToTexture,
	metal_deviceCmdBufferTransitionBarrier,
	metal_deviceCmdBufferQueueGrabBarrier,
	metal_deviceCmdBufferQueueReleaseBarrier,
	metal_deviceCmdTextureTransitionBarrier,
	metal_deviceCmdTextureQueueGrabBarrier,
	metal_deviceCmdTextureQueueReleaseBarrier,
};

/*
 */
Opal_Result metal_deviceInitialize(Metal_Device *device_ptr, Metal_Instance *instance_ptr, id<MTLDevice> metal_device)
{
	assert(instance_ptr);
	assert(device_ptr);

	OPAL_UNUSED(instance_ptr);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	device_ptr->device = metal_device;

	// allocator
	Opal_Result result = metal_allocatorInitialize(device_ptr, instance_ptr->heap_size, instance_ptr->max_heap_allocations, instance_ptr->max_heaps);
	assert(result == OPAL_SUCCESS);

	// bump
	opal_bumpInitialize(&device_ptr->bump, 256);

	// pools
	opal_poolInitialize(&device_ptr->buffers, sizeof(Metal_Buffer), 32);
	opal_poolInitialize(&device_ptr->textures, sizeof(Metal_Texture), 32);
	opal_poolInitialize(&device_ptr->texture_views, sizeof(Metal_TextureView), 32);
	opal_poolInitialize(&device_ptr->samplers, sizeof(Metal_Sampler), 32);

	return OPAL_SUCCESS;
}

Opal_Result metal_deviceAllocateMemory(Metal_Device *device_ptr, const Metal_AllocationDesc *desc, Metal_Allocation *allocation)
{
	assert(device_ptr);

	Metal_Allocator *allocator = &device_ptr->allocator;

	// resolve allocation type (suballocated or dedicated)
	uint32_t dedicated = (desc->hint == OPAL_ALLOCATION_HINT_PREFER_DEDICATED);
	const float heap_threshold = 0.7f;

	if (desc->hint == OPAL_ALLOCATION_HINT_AUTO)
	{
		uint32_t too_big_for_heap = desc->size > allocator->heap_size * heap_threshold;
		dedicated = too_big_for_heap;
	}

	if (desc->hint == OPAL_ALLOCATION_HINT_PREFER_HEAP)
	{
		uint32_t wont_fit_in_heap = desc->size > allocator->heap_size;
		dedicated = wont_fit_in_heap;
	}

	Opal_Result opal_result = OPAL_NO_MEMORY;

	if (dedicated == 0)
		opal_result = metal_allocatorAllocateMemory(device_ptr, desc, 0, allocation);

	if (opal_result == OPAL_NO_MEMORY)
		opal_result = metal_allocatorAllocateMemory(device_ptr, desc, 1, allocation);

	return opal_result;
}
