#include "metal_internal.h"
#include "common/intrinsics.h"

/*
 */
static Opal_Result metal_deviceFreeDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set);
static Opal_Result metal_deviceUpdateDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set, uint32_t num_entries, const Opal_DescriptorSetEntry *entries);

/*
 */
static void metal_destroyQueue(Metal_Device *device_ptr, Metal_Queue *queue_ptr)
{
	assert(device_ptr);
	assert(queue_ptr);

	@autoreleasepool
	{
		[queue_ptr->queue removeResidencySet: device_ptr->allocator.residency_set];
		[queue_ptr->queue release];
	}
}

static void metal_destroySemaphore(Metal_Device *device_ptr, Metal_Semaphore *semaphore_ptr)
{
	assert(device_ptr);
	assert(semaphore_ptr);

	OPAL_UNUSED(device_ptr);

	@autoreleasepool
	{
		[semaphore_ptr->event release];
	}
}

static void metal_destroyBuffer(Metal_Device *device_ptr, Metal_Buffer *buffer_ptr)
{
	assert(device_ptr);
	assert(buffer_ptr);

	@autoreleasepool
	{
		[buffer_ptr->buffer release];
		metal_allocatorFreeMemory(device_ptr, buffer_ptr->allocation);
	}
}

static void metal_destroyTexture(Metal_Device *device_ptr, Metal_Texture *texture_ptr)
{
	assert(device_ptr);
	assert(texture_ptr);

	@autoreleasepool
	{
		[texture_ptr->texture release];
		metal_allocatorFreeMemory(device_ptr, texture_ptr->allocation);
	}
}

static void metal_destroyTextureView(Metal_Device *device_ptr, Metal_TextureView *texture_view_ptr)
{
	assert(device_ptr);
	assert(texture_view_ptr);

	OPAL_UNUSED(device_ptr);

	@autoreleasepool
	{
		[texture_view_ptr->texture_view release];
	}
}

static void metal_destroySampler(Metal_Device *device_ptr, Metal_Sampler *sampler_ptr)
{
	assert(device_ptr);
	assert(sampler_ptr);

	OPAL_UNUSED(device_ptr);

	@autoreleasepool
	{
		[sampler_ptr->sampler release];
	}
}

static void metal_destroyAccelerationStructure(Metal_Device *device_ptr, Metal_AccelerationStructure *acceleration_structure_ptr)
{
	assert(device_ptr);
	assert(acceleration_structure_ptr);

	@autoreleasepool
	{
		[acceleration_structure_ptr->acceleration_structure release];
		metal_allocatorFreeMemory(device_ptr, acceleration_structure_ptr->allocation);
	}
}

static void metal_destroyCommandAllocator(Metal_Device *device_ptr, Metal_CommandAllocator *command_allocator_ptr)
{
	assert(device_ptr);
	assert(command_allocator_ptr);

	OPAL_UNUSED(device_ptr);
	OPAL_UNUSED(command_allocator_ptr);
}

static void metal_destroyCommandBuffer(Metal_Device *device_ptr, Metal_CommandBuffer *command_buffer_ptr)
{
	assert(device_ptr);
	assert(command_buffer_ptr);

	OPAL_UNUSED(device_ptr);
	OPAL_UNUSED(command_buffer_ptr);

	// TODO:
}

static void metal_destroyShader(Metal_Device *device_ptr, Metal_Shader *shader_ptr)
{
	assert(device_ptr);
	assert(shader_ptr);

	OPAL_UNUSED(device_ptr);

	@autoreleasepool
	{
		[shader_ptr->library release];
	}
}

static void metal_destroyDescriptorHeap(Metal_Device *device_ptr, Metal_DescriptorHeap *descriptor_heap_ptr)
{
	assert(device_ptr);
	assert(descriptor_heap_ptr);

	opal_heapShutdown(&descriptor_heap_ptr->heap);

	@autoreleasepool
	{
		[descriptor_heap_ptr->buffer release];
		metal_allocatorFreeMemory(device_ptr, descriptor_heap_ptr->allocation);
	}
}

static void metal_destroyDescriptorSetLayout(Metal_Device *device_ptr, Metal_DescriptorSetLayout *descriptor_set_layout_ptr)
{
	assert(device_ptr);
	assert(descriptor_set_layout_ptr);

	OPAL_UNUSED(device_ptr);

	@autoreleasepool
	{
		[descriptor_set_layout_ptr->encoder release];
	}

	if (descriptor_set_layout_ptr->descriptors)
	{
		assert(descriptor_set_layout_ptr->num_descriptors > 0);
		free(descriptor_set_layout_ptr->descriptors);
	}

	descriptor_set_layout_ptr->descriptors = NULL;
	descriptor_set_layout_ptr->num_descriptors = 0;
}

static void metal_destroyDescriptorSet(Metal_Device *device_ptr, Metal_DescriptorSet *descriptor_set_ptr)
{
	assert(device_ptr);
	assert(descriptor_set_ptr);

	Metal_DescriptorHeap *descriptor_heap_ptr = (Metal_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)descriptor_set_ptr->heap);
	assert(descriptor_heap_ptr);

	if (descriptor_set_ptr->num_static_descriptors > 0)
	{
		Opal_Result opal_result = opal_heapFree(&descriptor_heap_ptr->heap, descriptor_set_ptr->allocation);
		assert(opal_result == OPAL_SUCCESS);

	}

	if (descriptor_set_ptr->dynamic_descriptors)
	{
		assert(descriptor_set_ptr->num_dynamic_descriptors > 0);
		free(descriptor_set_ptr->dynamic_descriptors);
	}

	descriptor_set_ptr->dynamic_descriptors = NULL;
	descriptor_set_ptr->num_static_descriptors = 0;
	descriptor_set_ptr->num_dynamic_descriptors = 0;
}

static void metal_destroyPipelineLayout(Metal_Device *device_ptr, Metal_PipelineLayout *pipeline_layout_ptr)
{
	assert(device_ptr);
	assert(pipeline_layout_ptr);

	OPAL_UNUSED(device_ptr);

	if (pipeline_layout_ptr->layouts)
	{
		assert(pipeline_layout_ptr->num_layouts > 0);
		free(pipeline_layout_ptr->layouts);
	}

	if (pipeline_layout_ptr->dynamic_binding_offsets)
	{
		assert(pipeline_layout_ptr->num_layouts > 0);
		free(pipeline_layout_ptr->dynamic_binding_offsets);
	}

	pipeline_layout_ptr->layouts = NULL;
	pipeline_layout_ptr->dynamic_binding_offsets = NULL;
	pipeline_layout_ptr->num_layouts = 0;
}

static void metal_destroyPipeline(Metal_Device *device_ptr, Metal_Pipeline *pipeline_ptr)
{
	assert(device_ptr);
	assert(pipeline_ptr);

	OPAL_UNUSED(device_ptr);

	@autoreleasepool
	{
		if (pipeline_ptr->depth_stencil_state)
		{
			[pipeline_ptr->depth_stencil_state release];
			pipeline_ptr->depth_stencil_state = nil;
		}

		if (pipeline_ptr->render_pipeline)
		{
			[pipeline_ptr->render_pipeline release];
			pipeline_ptr->render_pipeline = nil;
		}

		if (pipeline_ptr->compute_pipeline)
		{
			[pipeline_ptr->compute_pipeline release];
			pipeline_ptr->compute_pipeline = nil;
		}
	}
}

static void metal_destroySwapchain(Metal_Device *device_ptr, Metal_Swapchain *swapchain_ptr)
{
	assert(device_ptr);
	assert(swapchain_ptr);

	OPAL_UNUSED(device_ptr);

	@autoreleasepool
	{
		CGColorSpaceRelease(swapchain_ptr->colorspace);
		[swapchain_ptr->queue release];
	}
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

	Metal_Device *ptr = (Metal_Device *)this;
	uint32_t queue_count = ptr->device_engines_info.queue_counts[engine_type];

	if (index >= queue_count)
		return OPAL_INVALID_QUEUE_INDEX;

	Opal_Queue *queue_handles = ptr->queue_handles[engine_type];
	assert(queue_handles);

	*queue = queue_handles[index];
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceGetAccelerationStructurePrebuildInfo(Opal_Device this, const Opal_AccelerationStructureBuildDesc *desc, Opal_AccelerationStructurePrebuildInfo *info)
{
	assert(this);
	assert(desc);
	assert(info);

	Metal_Device *device_ptr = (Metal_Device *)this;
	id<MTLDevice> metal_device = device_ptr->device;

	MTLAccelerationStructureSizes sizes = {0};
	@autoreleasepool
	{
		MTLAccelerationStructureDescriptor *build_info = NULL;

		if (desc->type == OPAL_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
		{
			uint32_t num_entries = desc->input.bottom_level.num_geometries;
			MTLPrimitiveAccelerationStructureDescriptor *blas_desc = [MTLPrimitiveAccelerationStructureDescriptor descriptor];

			NSMutableArray<MTLAccelerationStructureGeometryDescriptor*>* geometries = [NSMutableArray arrayWithCapacity: num_entries];

			for (uint32_t i = 0; i < num_entries; ++i)
			{
				const Opal_AccelerationStructureGeometry *opal_geometry = &desc->input.bottom_level.geometries[i];

				if (opal_geometry->type == OPAL_ACCELERATION_STRUCTURE_GEOMETRY_TYPE_TRIANGLES)
				{
					const Opal_AccelerationStructureGeometryDataTriangles *opal_triangles = &opal_geometry->data.triangles;
					MTLAccelerationStructureTriangleGeometryDescriptor *triangles = [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];

					triangles.vertexFormat = metal_helperToAttributeFormat(opal_triangles->vertex_format);
					triangles.vertexStride = opal_triangles->vertex_stride;
					triangles.indexType = metal_helperToIndexType(opal_triangles->index_format);

					triangles.triangleCount = opal_triangles->num_vertices / 3;

					if (opal_triangles->index_buffer.buffer != OPAL_NULL_HANDLE)
						triangles.triangleCount = opal_triangles->num_indices / 3;

					triangles.transformationMatrixLayout = MTLMatrixLayoutRowMajor;

					[geometries addObject: triangles];
				}
				else if (opal_geometry->type == OPAL_ACCELERATION_STRUCTURE_GEOMETRY_TYPE_AABBS)
				{
					const Opal_AccelerationStructureGeometryDataAABBs *opal_aabbs = &opal_geometry->data.aabbs;
					MTLAccelerationStructureBoundingBoxGeometryDescriptor *aabbs = [MTLAccelerationStructureBoundingBoxGeometryDescriptor descriptor];

					aabbs.boundingBoxStride = opal_aabbs->stride;
					aabbs.boundingBoxCount = opal_aabbs->num_entries;

					[geometries addObject: aabbs];
				}
				else
					assert(0);
			}

			blas_desc.geometryDescriptors = geometries;
			build_info = blas_desc;
		}
		else if (desc->type == OPAL_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL)
		{
			const Opal_AccelerationStructureBuildInputTopLevel *input = &desc->input.top_level;

			MTLInstanceAccelerationStructureDescriptor *tlas_desc = [MTLInstanceAccelerationStructureDescriptor descriptor];
			tlas_desc.instanceDescriptorType = MTLAccelerationStructureInstanceDescriptorTypeUserID;
			tlas_desc.instanceDescriptorStride = sizeof(MTLAccelerationStructureUserIDInstanceDescriptor);
			tlas_desc.instanceCount = input->num_instances;

			build_info = tlas_desc;
		}

		assert(build_info);
		sizes = [metal_device accelerationStructureSizesWithDescriptor: build_info];
	}

	info->acceleration_structure_size = sizes.accelerationStructureSize;
	info->build_scratch_size = sizes.buildScratchBufferSize;
	info->update_scratch_size = sizes.refitScratchBufferSize;
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceGetSupportedSurfaceFormats(Opal_Device this, Opal_Surface surface, uint32_t *num_formats, Opal_SurfaceFormat *formats)
{
	assert(this);
	assert(surface);
	assert(num_formats);

	static Opal_SurfaceFormat allowed_formats[] =
	{
		OPAL_TEXTURE_FORMAT_BGRA8_UNORM, OPAL_COLOR_SPACE_SRGB,
		OPAL_TEXTURE_FORMAT_BGRA8_UNORM_SRGB, OPAL_COLOR_SPACE_SRGB,
		// TODO: add rgb10a2 + hdr10 support
	};
	static uint32_t num_allowed_formats = sizeof(allowed_formats) / sizeof(Opal_SurfaceFormat);

	*num_formats = num_allowed_formats;

	if (formats)
		memcpy(formats, &allowed_formats, sizeof(Opal_SurfaceFormat) * num_allowed_formats);

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceGetSupportedPresentModes(Opal_Device this, Opal_Surface surface, uint32_t *num_present_modes, Opal_PresentMode *present_modes)
{
	assert(this);
	assert(surface);
	assert(num_present_modes);

	static Opal_PresentMode allowed_present_modes[] =
	{
		OPAL_PRESENT_MODE_FIFO,
		OPAL_PRESENT_MODE_IMMEDIATE,
	};
	static uint32_t num_allowed_present_modes = sizeof(allowed_present_modes) / sizeof(Opal_PresentMode);

	*num_present_modes = num_allowed_present_modes;

	if (present_modes)
		memcpy(present_modes, &allowed_present_modes, sizeof(Opal_PresentMode) * num_allowed_present_modes);

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceGetPreferredSurfaceFormat(Opal_Device this, Opal_Surface surface, Opal_SurfaceFormat *format)
{
	assert(this);
	assert(surface);
	assert(format);

	Metal_Device *device_ptr = (Metal_Device *)this;

	uint32_t num_formats = 0;
	metal_deviceGetSupportedSurfaceFormats(this, surface, &num_formats, NULL);

	if (num_formats == 0)
		return OPAL_SURFACE_NOT_DRAWABLE;

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(Opal_SurfaceFormat) * num_formats);
	Opal_SurfaceFormat *formats = (Opal_SurfaceFormat *)(device_ptr->bump.data);

	metal_deviceGetSupportedSurfaceFormats(this, surface, &num_formats, formats);

	Opal_TextureFormat optimal_format = OPAL_TEXTURE_FORMAT_BGRA8_UNORM;
	Opal_ColorSpace optimal_color_space = OPAL_COLOR_SPACE_SRGB;

	*format = formats[0];
	for (uint32_t i = 0; i < num_formats; ++i)
	{
		if (formats[i].texture_format == optimal_format && formats[i].color_space == optimal_color_space)
		{
			format->texture_format = optimal_format;
			format->color_space = optimal_color_space;
			break;
		}
	}

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceGetPreferredSurfacePresentMode(Opal_Device this, Opal_Surface surface, Opal_PresentMode *present_mode)
{
	assert(this);
	assert(surface);
	assert(present_mode);

	Metal_Device *device_ptr = (Metal_Device *)this;

	uint32_t num_present_modes = 0;
	metal_deviceGetSupportedPresentModes(this, surface, &num_present_modes, NULL);

	if (num_present_modes == 0)
		return OPAL_SURFACE_NOT_PRESENTABLE;

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(Opal_PresentMode) * num_present_modes);
	Opal_PresentMode *present_modes = (Opal_PresentMode *)(device_ptr->bump.data);

	metal_deviceGetSupportedPresentModes(this, surface, &num_present_modes, present_modes);

	Opal_PresentMode optimal_present_mode = OPAL_PRESENT_MODE_FIFO;

	*present_mode = present_modes[0];
	for (uint32_t i = 0; i < num_present_modes; ++i)
	{
		if (present_modes[i] == optimal_present_mode)
		{
			*present_mode = optimal_present_mode;
			break;
		}
	}

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateSemaphore(Opal_Device this, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore)
{
	assert(this);
	assert(desc);
	assert(semaphore);

	Metal_Device *device_ptr = (Metal_Device *)this;
	id<MTLDevice> metal_device = device_ptr->device;

	id<MTLEvent> metal_event = nil;
	id<MTLSharedEvent> metal_shared_event = nil;

	@autoreleasepool
	{
		if (desc->flags & OPAL_SEMAPHORE_CREATION_FLAGS_HOST_OPERATIONS)
		{
			metal_shared_event = [device_ptr->device newSharedEvent];
			metal_event = metal_shared_event;
		}
		else
			metal_event = [device_ptr->device newEvent];

		if (!metal_event)
			return OPAL_METAL_ERROR;

		if (desc->initial_value != 0)
		{
			id<MTLCommandQueue> metal_queue = [device_ptr->device newCommandQueue];
			if (!metal_queue)
				return OPAL_METAL_ERROR;

			id<MTLCommandBuffer> metal_command_buffer = [metal_queue commandBufferWithUnretainedReferences];
			if (!metal_command_buffer)
				return OPAL_METAL_ERROR;

			[metal_command_buffer encodeSignalEvent: metal_event value: desc->initial_value];
			[metal_command_buffer commit];
			[metal_command_buffer waitUntilCompleted];
		}

		[metal_event retain];
	}

	// create opal struct
	Metal_Semaphore result = {0};
	result.event = metal_event;
	result.shared_event = metal_shared_event;

	*semaphore = (Opal_Buffer)opal_poolAddElement(&device_ptr->semaphores, &result);
	return OPAL_SUCCESS;
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

	@autoreleasepool
	{
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

		[metal_buffer retain];
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
	MTLPixelFormat metal_format = metal_helperToPixelFormat(desc->format);

	@autoreleasepool
	{
		// fill texture info
		MTLTextureDescriptor *info = [[MTLTextureDescriptor alloc] init];

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
			return opal_result;

		assert(allocation.offset % allocation_info.align == 0);

		metal_texture = [allocation.memory newTextureWithDescriptor: info offset: allocation.offset];

		if (!metal_texture)
		{
			metal_allocatorFreeMemory(device_ptr, allocation);
			return OPAL_METAL_ERROR;
		}

		[metal_texture retain];
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

	id<MTLTexture> metal_texture_view = nil;
	
	@autoreleasepool
	{
		metal_texture_view = [texture_ptr->texture
			newTextureViewWithPixelFormat: texture_ptr->format
			textureType: metal_helperToTextureViewType(desc->type)
			levels: NSMakeRange(desc->base_mip, desc->mip_count)
			slices: NSMakeRange(desc->base_layer, desc->layer_count)
		];

		if (!metal_texture_view)
			return OPAL_METAL_ERROR;

		[metal_texture_view retain];
	}

	Metal_TextureView result = {0};
	result.texture_view = metal_texture_view;
	result.texture = desc->texture;
	result.base_mip = desc->base_mip;
	result.base_layer = desc->base_layer;

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

	@autoreleasepool
	{
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

		if (!metal_sampler)
			return OPAL_METAL_ERROR;

		[metal_sampler retain];
	}

	// create opal struct
	Metal_Sampler result = {0};
	result.sampler = metal_sampler;

	*sampler = (Opal_Texture)opal_poolAddElement(&device_ptr->samplers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateAccelerationStructure(Opal_Device this, const Opal_AccelerationStructureDesc *desc, Opal_AccelerationStructure *acceleration_structure)
{
	assert(this);
	assert(desc);
	assert(acceleration_structure);

	Metal_Device *device_ptr = (Metal_Device *)this;
	id<MTLDevice> metal_device = device_ptr->device;

	id<MTLAccelerationStructure> metal_acceleration_structure = nil;
	Metal_Allocation allocation = {0};

	@autoreleasepool
	{
		MTLSizeAndAlign allocation_info = [metal_device heapAccelerationStructureSizeAndAlignWithSize: desc->size];

		// fill allocation info
		Metal_AllocationDesc allocation_desc = {0};
		allocation_desc.size = allocation_info.size;
		allocation_desc.alignment = allocation_info.align;
		allocation_desc.allocation_type = OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL;
		allocation_desc.hint = OPAL_ALLOCATION_HINT_AUTO;

		Opal_Result opal_result = metal_deviceAllocateMemory(device_ptr, &allocation_desc, &allocation);
		if (opal_result != OPAL_SUCCESS)
			return opal_result;

		assert(allocation.offset % allocation_info.align == 0);

		metal_acceleration_structure = [allocation.memory newAccelerationStructureWithSize: allocation_info.size offset: allocation.offset];

		if (!metal_acceleration_structure)
		{
			metal_allocatorFreeMemory(device_ptr, allocation);
			return OPAL_METAL_ERROR;
		}

		[metal_acceleration_structure retain];
	}

	// create opal struct
	Metal_AccelerationStructure result = {0};
	result.acceleration_structure = metal_acceleration_structure;
	result.allocation = allocation;

	*acceleration_structure = (Opal_Texture)opal_poolAddElement(&device_ptr->acceleration_structures, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateShaderBindingTable(Opal_Device this, Opal_Pipeline pipeline, Opal_ShaderBindingTable *shader_binding_table)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline);
	OPAL_UNUSED(shader_binding_table);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateCommandAllocator(Opal_Device this, Opal_Queue queue, Opal_CommandAllocator *command_allocator)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(command_allocator);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_Queue *queue_ptr = (Metal_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queue);
	assert(queue_ptr);

	OPAL_UNUSED(queue_ptr);

	Metal_CommandAllocator result = {0};
	result.queue = queue;
	
	*command_allocator = (Opal_CommandAllocator)opal_poolAddElement(&device_ptr->command_allocators, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateCommandBuffer(Opal_Device this, Opal_CommandAllocator command_allocator, Opal_CommandBuffer *command_buffer)
{
	assert(this);
	assert(command_allocator);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandAllocator *command_allocator_ptr = (Metal_CommandAllocator *)opal_poolGetElement(&device_ptr->command_allocators, (Opal_PoolHandle)command_allocator);
	assert(command_allocator_ptr);

	Metal_Queue *queue_ptr = (Metal_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)command_allocator_ptr->queue);
	assert(queue_ptr);

	command_allocator_ptr->command_buffer_usage++;

	Metal_CommandBuffer result = {0};
	result.queue = command_allocator_ptr->queue;
	result.command_allocator = command_allocator;

	*command_buffer = (Opal_CommandBuffer)opal_poolAddElement(&device_ptr->command_buffers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateShader(Opal_Device this, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	assert(this);
	assert(desc);
	assert(shader);

	Metal_Device *device_ptr = (Metal_Device *)this;

	id<MTLLibrary> metal_library = nil;

	@autoreleasepool
	{
		dispatch_data_t data = dispatch_data_create(desc->data, desc->size, nil, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
		assert(data);

		NSError *error = nil;
		metal_library = [device_ptr->device newLibraryWithData: data error: &error];

		if (!metal_library)
			return OPAL_METAL_ERROR;

		assert(error == nil);
		[metal_library retain];
	}

	Metal_Shader result = {0};
	result.library = metal_library;

	*shader = (Opal_Shader)opal_poolAddElement(&device_ptr->shaders, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateDescriptorHeap(Opal_Device this, const Opal_DescriptorHeapDesc *desc, Opal_DescriptorHeap *descriptor_heap)
{
	assert(this);
	assert(desc);
	assert(descriptor_heap);

	Metal_Device *device_ptr = (Metal_Device *)this;
	id<MTLDevice> metal_device = device_ptr->device;

	id<MTLBuffer> metal_buffer = nil;
	Metal_Allocation allocation = {0};
	MTLSizeAndAlign allocation_info = {0};

	@autoreleasepool
	{
		// estimate argument buffer size
		MTLDataType types[] =
		{
			MTLDataTypePointer,
			MTLDataTypeSampler,
		};

		uint32_t counts[] =
		{
			desc->num_resource_descriptors,
			desc->num_sampler_descriptors,
		};

		// TODO: check if there's a better estimation, i.e. maybe it's possible to calculate descriptor set overhead (MoltenVK does that)
		allocation_info.align = 256;

		for (uint32_t i = 0; i < 2; ++i)
		{
			MTLArgumentDescriptor *arg = [MTLArgumentDescriptor argumentDescriptor];
			arg.dataType = types[i];
			arg.access = MTLBindingAccessReadWrite;
			arg.index = 0;
			arg.arrayLength = counts[i];
			arg.textureType = MTLTextureTypeCubeArray;

			NSArray *args = @[arg];
			id<MTLArgumentEncoder> metal_encoder = [metal_device newArgumentEncoderWithArguments: args];

			if (metal_encoder == nil)
				return OPAL_METAL_ERROR;

			allocation_info.size += metal_encoder.encodedLength;
		}

		allocation_info.size = alignUpul(allocation_info.size, allocation_info.align);

		// fill allocation info
		Metal_AllocationDesc allocation_desc = {0};
		allocation_desc.size = allocation_info.size;
		allocation_desc.alignment = allocation_info.align;
		allocation_desc.allocation_type = OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD;
		allocation_desc.hint = OPAL_ALLOCATION_HINT_AUTO;

		Opal_Result opal_result = metal_deviceAllocateMemory(device_ptr, &allocation_desc, &allocation);
		if (opal_result != OPAL_SUCCESS)
			return opal_result;

		assert(allocation.offset % allocation_info.align == 0);

		MTLResourceOptions options = MTLResourceStorageModeShared | MTLResourceCPUCacheModeDefaultCache;

		metal_buffer = [allocation.memory newBufferWithLength: allocation_desc.size options: options offset: allocation.offset];
		if (!metal_buffer)
		{
			metal_allocatorFreeMemory(device_ptr, allocation);
			return OPAL_METAL_ERROR;
		}

		[metal_buffer retain];
	}

	// create opal struct
	Metal_DescriptorHeap result = {0};
	result.buffer = metal_buffer;
	result.allocation = allocation;

	uint32_t num_blocks = (uint32_t)(allocation_info.size / allocation_info.align);
	Opal_Result opal_result = opal_heapInitialize(&result.heap, num_blocks, num_blocks);
	assert(opal_result == OPAL_SUCCESS);

	*descriptor_heap = (Opal_Buffer)opal_poolAddElement(&device_ptr->descriptor_heaps, &result);
	return opal_result;
}

static Opal_Result metal_deviceCreateDescriptorSetLayout(Opal_Device this, uint32_t num_entries, const Opal_DescriptorSetLayoutEntry *entries, Opal_DescriptorSetLayout *descriptor_set_layout)
{
	assert(this);
	assert(num_entries == 0 || entries);
	assert(descriptor_set_layout);

	Metal_Device *device_ptr = (Metal_Device *)this;
	id<MTLDevice> metal_device = device_ptr->device;

	id<MTLArgumentEncoder> metal_encoder = nil;

	uint32_t num_static_entries = 0;
	uint32_t num_dynamic_entries = 0;
	uint32_t num_blocks = 0;

	@autoreleasepool
	{
		NSMutableArray<MTLArgumentDescriptor*>* args = [NSMutableArray arrayWithCapacity: num_entries];

		for (uint32_t i = 0; i < num_entries; ++i)
		{
			const Opal_DescriptorSetLayoutEntry *entry = &entries[i];

			switch (entry->type)
			{
				case OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC:
				{
					num_dynamic_entries++;
				}
				break;

				default:
				{
					MTLArgumentDescriptor *arg = [MTLArgumentDescriptor argumentDescriptor];
					arg.dataType = metal_helperToArgumentDataType(entry->type);
					arg.access = MTLBindingAccessReadWrite;
					arg.index = entry->binding;
					arg.arrayLength = 1;
					arg.textureType = MTLTextureTypeCubeArray;

					[args addObject: arg];

					num_static_entries++;
				}
				break;
			}
		}

		assert([args count] == num_static_entries);
		assert(num_static_entries + num_dynamic_entries == num_entries);

		if (num_static_entries > 0)
		{
			NSSortDescriptor *argument_sort_desc = [[NSSortDescriptor alloc] initWithKey:@"index" ascending:YES];
			NSArray *sorted_args = [args sortedArrayUsingDescriptors:@[argument_sort_desc]];

			metal_encoder = [metal_device newArgumentEncoderWithArguments: sorted_args];

			if (metal_encoder == nil)
				return OPAL_METAL_ERROR;

			num_blocks = alignUpul(metal_encoder.encodedLength, 256) / 256;
		}
	}

	Metal_DescriptorInfo *metal_entries = NULL;
	if (num_entries > 0)
	{
		metal_entries = (Metal_DescriptorInfo *)malloc(sizeof(Metal_DescriptorInfo) * num_entries);
		memset(metal_entries, 0, sizeof(Metal_DescriptorInfo) * num_entries);
	}

	uint32_t static_entry_index = 0;
	uint32_t dynamic_entry_index = 0;
	for (uint32_t i = 0; i < num_entries; ++i)
	{
		const Opal_DescriptorSetLayoutEntry *entry = &entries[i];
		Metal_DescriptorInfo *metal_entry = NULL;

		switch (entry->type)
		{
			case OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC:
			{
				metal_entry = &metal_entries[num_static_entries + dynamic_entry_index];
				dynamic_entry_index++;
			}
			break;

			default:
			{
				metal_entry = &metal_entries[static_entry_index++];
			}
			break;
		}

		metal_entry->binding = entry->binding;
		metal_entry->opal_type = entry->type;
		metal_entry->api_type = metal_helperToArgumentDataType(entry->type);
	}

	Metal_DescriptorSetLayout result = {0};
	result.encoder = metal_encoder;
	result.num_blocks = num_blocks;
	result.num_static_descriptors = num_static_entries;
	result.num_dynamic_descriptors = num_dynamic_entries;
	result.num_descriptors = num_entries;
	result.descriptors = metal_entries;

	*descriptor_set_layout = (Opal_DescriptorSetLayout)opal_poolAddElement(&device_ptr->descriptor_set_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_descriptor_set_layouts, const Opal_DescriptorSetLayout *descriptor_set_layouts, Opal_PipelineLayout *pipeline_layout)
{
	assert(this);
	assert(num_descriptor_set_layouts == 0 || descriptor_set_layouts);
	assert(pipeline_layout);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_PipelineLayout result = {0};
	result.num_layouts = num_descriptor_set_layouts;

	if (result.num_layouts > 0)
	{
		result.layouts = (Opal_DescriptorSetLayout *)malloc(sizeof(Opal_DescriptorSetLayout) * result.num_layouts);
		memcpy(result.layouts, descriptor_set_layouts, sizeof(Opal_DescriptorSetLayout) * result.num_layouts);

		result.dynamic_binding_offsets = (uint32_t *)malloc(sizeof(uint32_t) * result.num_layouts);
		memset(result.dynamic_binding_offsets, 0, sizeof(uint32_t) * result.num_layouts);

		uint32_t offset = 0;
		for (uint32_t i = 0; i < result.num_layouts; ++i)
		{
			Metal_DescriptorSetLayout *descriptor_set_layout_ptr = (Metal_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_layouts[i]);
			assert(descriptor_set_layout_ptr);

			if (descriptor_set_layout_ptr->num_static_descriptors > 0)
				offset++;
		};

		for (uint32_t i = 0; i < result.num_layouts; ++i)
		{
			Metal_DescriptorSetLayout *descriptor_set_layout_ptr = (Metal_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_layouts[i]);
			assert(descriptor_set_layout_ptr);

			if (descriptor_set_layout_ptr->num_dynamic_descriptors > 0)
			{
				result.dynamic_binding_offsets[i] = offset;
				offset += descriptor_set_layout_ptr->num_dynamic_descriptors;
			}
		}

		result.vertex_binding_offset = offset;
	}

	*pipeline_layout = (Opal_PipelineLayout)opal_poolAddElement(&device_ptr->pipeline_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	assert(this);
	assert(desc);
	assert(pipeline);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_PipelineLayout *pipeline_layout_ptr = (Metal_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)desc->pipeline_layout);
	assert(pipeline_layout_ptr);

	id<MTLRenderPipelineState> metal_render_pipeline = nil;
	id<MTLDepthStencilState> metal_depth_stencil_state = nil;

	@autoreleasepool
	{
		MTLRenderPipelineDescriptor *info = [[MTLRenderPipelineDescriptor alloc] init];

		// shaders

		// TODO: use tessellation_control shader as post-tessellation vertex function

		Opal_Shader opal_shaders[2] =
		{
			desc->vertex_function.shader,
			desc->fragment_function.shader,
		};

		NSString *opal_names[2] =
		{
			[NSString stringWithUTF8String: desc->vertex_function.name],
			[NSString stringWithUTF8String: desc->fragment_function.name],
		};

		id<MTLFunction> metal_shader_functions[2] =
		{
			nil,
			nil,
		};
		
		for (uint32_t i = 0; i < 2; ++i)
		{
			Metal_Shader *shader_ptr = (Metal_Shader *)opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)opal_shaders[i]);
			if (shader_ptr == NULL)
				continue;

			metal_shader_functions[i] = [shader_ptr->library newFunctionWithName: opal_names[i]];
		}

		info.vertexFunction = metal_shader_functions[0];
		info.fragmentFunction = metal_shader_functions[1];

		// vertex input
		MTLVertexDescriptor *vertex_info = [MTLVertexDescriptor new];

		uint32_t num_attributes = 0;
		for (uint32_t i = 0; i < desc->num_vertex_streams; ++i)
		{
			const Opal_VertexStream *vertex_stream = &desc->vertex_streams[i];

			uint32_t buffer_index = pipeline_layout_ptr->vertex_binding_offset + i;
			vertex_info.layouts[buffer_index].stride = vertex_stream->stride;
			vertex_info.layouts[buffer_index].stepRate = 1;
			vertex_info.layouts[buffer_index].stepFunction = metal_helperToVertexStepFunction(vertex_stream->rate);

			for (uint32_t j = 0; j < vertex_stream->num_vertex_attributes; ++j)
			{
				const Opal_VertexAttribute *vertex_attribute = &vertex_stream->attributes[j];

				vertex_info.attributes[num_attributes].format = metal_helperToVertexFormat(vertex_attribute->format);
				vertex_info.attributes[num_attributes].offset = vertex_attribute->offset;
				vertex_info.attributes[num_attributes].bufferIndex = buffer_index;
				num_attributes++;
			}
		}

		info.vertexDescriptor = vertex_info;

		// color attachments
		for (uint32_t i = 0; i < desc->num_color_attachments; ++i)
		{
			const Opal_BlendState *opal_blend_state = &desc->color_blend_states[i];

			info.colorAttachments[i].pixelFormat = metal_helperToPixelFormat(desc->color_attachment_formats[i]);
			info.colorAttachments[i].blendingEnabled = opal_blend_state->enable;
			info.colorAttachments[i].sourceRGBBlendFactor = metal_helperToBlendFactor(opal_blend_state->src_color);
			info.colorAttachments[i].destinationRGBBlendFactor = metal_helperToBlendFactor(opal_blend_state->dst_color);
			info.colorAttachments[i].rgbBlendOperation = metal_helperToBlendOperation(opal_blend_state->color_op);
			info.colorAttachments[i].sourceAlphaBlendFactor = metal_helperToBlendFactor(opal_blend_state->src_alpha);
			info.colorAttachments[i].destinationAlphaBlendFactor = metal_helperToBlendFactor(opal_blend_state->dst_alpha);
			info.colorAttachments[i].alphaBlendOperation = metal_helperToBlendOperation(opal_blend_state->alpha_op);
		}

		// depthstencil attachment
		if (desc->depth_stencil_attachment_format)
		{
			MTLPixelFormat format = metal_helperToPixelFormat(*desc->depth_stencil_attachment_format);

			info.depthAttachmentPixelFormat = format;
			info.stencilAttachmentPixelFormat = format;
		}

		// rasterization state
		info.inputPrimitiveTopology = metal_helperToPrimitiveTopologyClass(desc->primitive_type);
		info.rasterSampleCount = metal_helperToSampleCount(desc->rasterization_samples);

		NSError *error = nil;
		metal_render_pipeline = [device_ptr->device newRenderPipelineStateWithDescriptor: info error: &error];

		if (!metal_render_pipeline)
			return OPAL_METAL_ERROR;

		MTLDepthStencilDescriptor *depth_stencil_info = [MTLDepthStencilDescriptor new];;
		assert(depth_stencil_info);
		
		if (desc->depth_enable)
		{
			depth_stencil_info.depthWriteEnabled = desc->depth_write;
			depth_stencil_info.depthCompareFunction = metal_helperToCompareFunction(desc->depth_compare_op);
		}

		MTLStencilDescriptor *stencil_front = nil;
		MTLStencilDescriptor *stencil_back = nil;

		if (desc->stencil_enable)
		{
			stencil_front = [MTLStencilDescriptor new];
			assert(stencil_front);

			stencil_front.depthStencilPassOperation = metal_helperToStencilOperation(desc->stencil_front.pass_op);
			stencil_front.depthFailureOperation = metal_helperToStencilOperation(desc->stencil_front.depth_fail_op);
			stencil_front.stencilFailureOperation = metal_helperToStencilOperation(desc->stencil_front.fail_op);
			stencil_front.stencilCompareFunction = metal_helperToCompareFunction(desc->stencil_front.compare_op);
			stencil_front.readMask = desc->stencil_read_mask;
			stencil_front.writeMask = desc->stencil_write_mask;

			depth_stencil_info.frontFaceStencil = stencil_front;

			stencil_back = [MTLStencilDescriptor new];
			assert(stencil_back);

			stencil_back.depthStencilPassOperation = metal_helperToStencilOperation(desc->stencil_back.pass_op);
			stencil_back.depthFailureOperation = metal_helperToStencilOperation(desc->stencil_back.depth_fail_op);
			stencil_back.stencilFailureOperation = metal_helperToStencilOperation(desc->stencil_back.fail_op);
			stencil_back.stencilCompareFunction = metal_helperToCompareFunction(desc->stencil_back.compare_op);
			stencil_back.readMask = desc->stencil_read_mask;
			stencil_back.writeMask = desc->stencil_write_mask;

			depth_stencil_info.backFaceStencil = stencil_back;
		}

		metal_depth_stencil_state = [device_ptr->device newDepthStencilStateWithDescriptor: depth_stencil_info];

		if (!metal_depth_stencil_state)
			return OPAL_METAL_ERROR;

		assert(error == nil);

		[metal_depth_stencil_state retain];
		[metal_render_pipeline retain];
	}

	// create opal struct
	Metal_Pipeline result = {0};
	result.render_pipeline = metal_render_pipeline;
	result.primitive_type = metal_helperToPrimitiveType(desc->primitive_type);
	result.cull_mode = metal_helperToCullMode(desc->cull_mode);
	result.winding = metal_helperToWinding(desc->front_face);
	result.depth_stencil_state = metal_depth_stencil_state;

	*pipeline = (Opal_Pipeline)opal_poolAddElement(&device_ptr->pipelines, &result);
	return OPAL_SUCCESS;
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
	assert(this);
	assert(desc);
	assert(pipeline);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_PipelineLayout *pipeline_layout_ptr = (Metal_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)desc->pipeline_layout);
	assert(pipeline_layout_ptr);

	id<MTLComputePipelineState> metal_compute_pipeline = nil;

	@autoreleasepool
	{
		Metal_Shader *shader_ptr = (Metal_Shader *)opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)desc->compute_function.shader);
		if (shader_ptr == NULL)
			return OPAL_METAL_ERROR;

		NSString *shader_function_name = [NSString stringWithUTF8String: desc->compute_function.name];
		id<MTLFunction> shader_function = [shader_ptr->library newFunctionWithName: shader_function_name];

		NSError *error = nil;
		metal_compute_pipeline = [device_ptr->device newComputePipelineStateWithFunction: shader_function error: &error];

		if (!metal_compute_pipeline)
			return OPAL_METAL_ERROR;

		assert(error == nil);

		[metal_compute_pipeline retain];
	}

	// create opal struct
	Metal_Pipeline result = {0};
	result.compute_pipeline = metal_compute_pipeline;
	result.threadgroup_size.width = desc->threadgroup_size_x;
	result.threadgroup_size.height = desc->threadgroup_size_y;
	result.threadgroup_size.depth = desc->threadgroup_size_z;

	*pipeline = (Opal_Pipeline)opal_poolAddElement(&device_ptr->pipelines, &result);
	return OPAL_SUCCESS;
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
	assert(this);
	assert(desc);
	assert(desc->surface);
	assert(swapchain);

	Metal_Device *device_ptr = (Metal_Device *)this;
	Metal_Instance *instance_ptr = device_ptr->instance;

	id<MTLCommandQueue> metal_queue = nil;
	CGColorSpaceRef metal_colorspace = nil;

	// surface
	Metal_Surface *surface_ptr = (Metal_Surface *)opal_poolGetElement(&instance_ptr->surfaces, (Opal_PoolHandle)desc->surface);
	assert(surface_ptr);

	CAMetalLayer *layer_ptr = surface_ptr->layer;
	assert(layer_ptr);

	@autoreleasepool
	{
		// surface capabilities
		if (desc->mode == OPAL_PRESENT_MODE_MAILBOX)
			return OPAL_SWAPCHAIN_PRESENT_MODE_NOT_SUPPORTED;

		uint32_t num_textures = (desc->mode == OPAL_PRESENT_MODE_IMMEDIATE) ? 1 : 2;

		// surface format
		MTLPixelFormat format = metal_helperToPixelFormat(desc->format.texture_format);
		if (format == MTLPixelFormatInvalid)
			return OPAL_SWAPCHAIN_FORMAT_NOT_SUPPORTED;

		CFStringRef colorspace_name = metal_helperToColorspaceName(desc->format.color_space);
		if (!colorspace_name)
			return OPAL_SWAPCHAIN_COLOR_SPACE_NOT_SUPPORTED;

		// present queue
		metal_queue = [device_ptr->device newCommandQueue];

		if (!metal_queue)
			return OPAL_METAL_ERROR;

		// config
		metal_colorspace = CGColorSpaceCreateWithName(colorspace_name);

		layer_ptr.device = device_ptr->device;
		layer_ptr.drawableSize = layer_ptr.frame.size;
		layer_ptr.pixelFormat = format;
		layer_ptr.colorspace = metal_colorspace;
		layer_ptr.framebufferOnly = (desc->usage == OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT);
		layer_ptr.displaySyncEnabled = (desc->mode == OPAL_PRESENT_MODE_IMMEDIATE);
		layer_ptr.maximumDrawableCount = num_textures;

		[metal_queue retain];
	}

	// create opal struct
	Metal_Swapchain result = {0};
	result.surface = desc->surface;
	result.queue = metal_queue;
	result.colorspace = metal_colorspace;

	*swapchain = (Opal_Swapchain)opal_poolAddElement(&device_ptr->swapchains, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroySemaphore(Opal_Device this, Opal_Semaphore semaphore)
{
	assert(this);
	assert(semaphore);

	Opal_PoolHandle handle = (Opal_PoolHandle)semaphore;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_Device *device_ptr = (Metal_Device *)this;
	Metal_Semaphore *semaphore_ptr = (Metal_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, handle);
	assert(semaphore_ptr);

	opal_poolRemoveElement(&device_ptr->semaphores, handle);

	metal_destroySemaphore(device_ptr, semaphore_ptr);
	return OPAL_SUCCESS;
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
	assert(this);
	assert(acceleration_structure);

	Opal_PoolHandle handle = (Opal_PoolHandle)acceleration_structure;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_Device *device_ptr = (Metal_Device *)this;
	Metal_AccelerationStructure *acceleration_structure_ptr = (Metal_AccelerationStructure *)opal_poolGetElement(&device_ptr->acceleration_structures, handle);
	assert(acceleration_structure_ptr);

	opal_poolRemoveElement(&device_ptr->acceleration_structures, handle);

	metal_destroyAccelerationStructure(device_ptr, acceleration_structure_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroyShaderBindingTable(Opal_Device this, Opal_ShaderBindingTable shader_binding_table)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(shader_binding_table);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyCommandAllocator(Opal_Device this, Opal_CommandAllocator command_allocator)
{
	assert(this);
	assert(command_allocator);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Opal_PoolHandle handle = (Opal_PoolHandle)command_allocator;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_CommandAllocator *command_allocator_ptr = (Metal_CommandAllocator *)opal_poolGetElement(&device_ptr->command_allocators, handle);
	assert(command_allocator_ptr);

	metal_destroyCommandAllocator(device_ptr, command_allocator_ptr);

	opal_poolRemoveElement(&device_ptr->command_allocators, handle);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroyCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	Metal_CommandAllocator *command_allocator_ptr = (Metal_CommandAllocator *)opal_poolGetElement(&device_ptr->command_allocators, (Opal_PoolHandle)command_buffer_ptr->command_allocator);
	assert(command_allocator_ptr);

	assert(command_allocator_ptr->command_buffer_usage > 0);
	command_allocator_ptr->command_buffer_usage--;

	metal_destroyCommandBuffer(device_ptr, command_buffer_ptr);

	opal_poolRemoveElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	assert(this);
	assert(shader);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Opal_PoolHandle handle = (Opal_PoolHandle)shader;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_Shader *shader_ptr = (Metal_Shader *)opal_poolGetElement(&device_ptr->shaders, handle);
	assert(shader_ptr);

	metal_destroyShader(device_ptr, shader_ptr);

	opal_poolRemoveElement(&device_ptr->shaders, handle);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroyDescriptorHeap(Opal_Device this, Opal_DescriptorHeap descriptor_heap)
{
	assert(this);
	assert(descriptor_heap);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Opal_PoolHandle handle = (Opal_PoolHandle)descriptor_heap;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_DescriptorHeap *descriptor_heap_ptr = (Metal_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, handle);
	assert(descriptor_heap_ptr);

	metal_destroyDescriptorHeap(device_ptr, descriptor_heap_ptr);

	opal_poolRemoveElement(&device_ptr->descriptor_heaps, handle);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroyDescriptorSetLayout(Opal_Device this, Opal_DescriptorSetLayout descriptor_set_layout)
{
	assert(this);
	assert(descriptor_set_layout);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Opal_PoolHandle handle = (Opal_PoolHandle)descriptor_set_layout;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_DescriptorSetLayout *descriptor_set_layout_ptr = (Metal_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, handle);
	assert(descriptor_set_layout_ptr);

	metal_destroyDescriptorSetLayout(device_ptr, descriptor_set_layout_ptr);

	opal_poolRemoveElement(&device_ptr->descriptor_set_layouts, handle);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	assert(this);
	assert(pipeline_layout);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Opal_PoolHandle handle = (Opal_PoolHandle)pipeline_layout;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_PipelineLayout *pipeline_layout_ptr = (Metal_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, handle);
	assert(pipeline_layout_ptr);

	metal_destroyPipelineLayout(device_ptr, pipeline_layout_ptr);

	opal_poolRemoveElement(&device_ptr->pipeline_layouts, handle);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroyPipeline(Opal_Device this, Opal_Pipeline pipeline)
{
	assert(this);
	assert(pipeline);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Opal_PoolHandle handle = (Opal_PoolHandle)pipeline;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_Pipeline *pipeline_ptr = (Metal_Pipeline *)opal_poolGetElement(&device_ptr->pipelines, handle);
	assert(pipeline_ptr);

	metal_destroyPipeline(device_ptr, pipeline_ptr);

	opal_poolRemoveElement(&device_ptr->pipelines, handle);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroySwapchain(Opal_Device this, Opal_Swapchain swapchain)
{
	assert(this);
	assert(swapchain);

	Opal_PoolHandle handle = (Opal_PoolHandle)swapchain;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_Device *device_ptr = (Metal_Device *)this;
	Metal_Swapchain *swapchain_ptr = (Metal_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, handle);
	assert(swapchain_ptr);

	opal_poolRemoveElement(&device_ptr->swapchains, handle);

	metal_destroySwapchain(device_ptr, swapchain_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceDestroy(Opal_Device this)
{
	assert(this);

	Metal_Device *ptr = (Metal_Device *)this;

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->swapchains);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_Swapchain *swapchain_ptr = (Metal_Swapchain *)opal_poolGetElementByIndex(&ptr->swapchains, head);
			metal_destroySwapchain(ptr, swapchain_ptr);

			head = opal_poolGetNextIndex(&ptr->swapchains, head);
		}

		opal_poolShutdown(&ptr->swapchains);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->pipelines);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_Pipeline *pipeline_ptr = (Metal_Pipeline *)opal_poolGetElementByIndex(&ptr->pipelines, head);
			metal_destroyPipeline(ptr, pipeline_ptr);

			head = opal_poolGetNextIndex(&ptr->pipelines, head);
		}

		opal_poolShutdown(&ptr->pipelines);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->pipeline_layouts);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_PipelineLayout *pipeline_layout_ptr = (Metal_PipelineLayout *)opal_poolGetElementByIndex(&ptr->pipeline_layouts, head);
			metal_destroyPipelineLayout(ptr, pipeline_layout_ptr);

			head = opal_poolGetNextIndex(&ptr->pipeline_layouts, head);
		}

		opal_poolShutdown(&ptr->pipeline_layouts);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->descriptor_sets);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_DescriptorSet *descriptor_set_ptr = (Metal_DescriptorSet *)opal_poolGetElementByIndex(&ptr->descriptor_sets, head);
			metal_destroyDescriptorSet(ptr, descriptor_set_ptr);

			head = opal_poolGetNextIndex(&ptr->descriptor_sets, head);
		}

		opal_poolShutdown(&ptr->descriptor_sets);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->descriptor_set_layouts);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_DescriptorSetLayout *descriptor_set_layout_ptr = (Metal_DescriptorSetLayout *)opal_poolGetElementByIndex(&ptr->descriptor_set_layouts, head);
			metal_destroyDescriptorSetLayout(ptr, descriptor_set_layout_ptr);

			head = opal_poolGetNextIndex(&ptr->descriptor_set_layouts, head);
		}

		opal_poolShutdown(&ptr->descriptor_set_layouts);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->descriptor_heaps);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_DescriptorHeap *descriptor_heap_ptr = (Metal_DescriptorHeap *)opal_poolGetElementByIndex(&ptr->descriptor_heaps, head);
			metal_destroyDescriptorHeap(ptr, descriptor_heap_ptr);

			head = opal_poolGetNextIndex(&ptr->descriptor_heaps, head);
		}

		opal_poolShutdown(&ptr->descriptor_heaps);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->shaders);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_Shader *shader_ptr = (Metal_Shader *)opal_poolGetElementByIndex(&ptr->shaders, head);
			metal_destroyShader(ptr, shader_ptr);

			head = opal_poolGetNextIndex(&ptr->shaders, head);
		}

		opal_poolShutdown(&ptr->shaders);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->command_buffers);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElementByIndex(&ptr->command_buffers, head);
			metal_destroyCommandBuffer(ptr, command_buffer_ptr);

			head = opal_poolGetNextIndex(&ptr->command_buffers, head);
		}

		opal_poolShutdown(&ptr->command_buffers);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->command_allocators);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_CommandAllocator *command_allocator_ptr = (Metal_CommandAllocator *)opal_poolGetElementByIndex(&ptr->command_allocators, head);
			metal_destroyCommandAllocator(ptr, command_allocator_ptr);

			head = opal_poolGetNextIndex(&ptr->command_allocators, head);
		}

		opal_poolShutdown(&ptr->command_allocators);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->acceleration_structures);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_AccelerationStructure *acceleration_structure_ptr = (Metal_AccelerationStructure *)opal_poolGetElementByIndex(&ptr->acceleration_structures, head);
			metal_destroyAccelerationStructure(ptr, acceleration_structure_ptr);

			head = opal_poolGetNextIndex(&ptr->acceleration_structures, head);
		}

		opal_poolShutdown(&ptr->acceleration_structures);
	}

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

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->semaphores);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_Semaphore *semaphore_ptr = (Metal_Semaphore *)opal_poolGetElementByIndex(&ptr->semaphores, head);
			metal_destroySemaphore(ptr, semaphore_ptr);

			head = opal_poolGetNextIndex(&ptr->semaphores, head);
		}

		opal_poolShutdown(&ptr->semaphores);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->queues);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Metal_Queue *queue_ptr = (Metal_Queue *)opal_poolGetElementByIndex(&ptr->queues, head);
			metal_destroyQueue(ptr, queue_ptr);

			head = opal_poolGetNextIndex(&ptr->queues, head);
		}

		opal_poolShutdown(&ptr->queues);
	}

	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
		free(ptr->queue_handles[i]);

	opal_bumpShutdown(&ptr->bump);

	@autoreleasepool
	{
		Opal_Result result = metal_allocatorShutdown(ptr);
		assert(result == OPAL_SUCCESS);

		OPAL_UNUSED(result);

		[ptr->device release];
	}

	free(ptr);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceBuildShaderBindingTable(Opal_Device this, Opal_ShaderBindingTable shader_binding_table, const Opal_ShaderBindingTableBuildDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(shader_binding_table);
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
	assert(this);
	assert(command_allocator);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandAllocator *command_allocator_ptr = (Metal_CommandAllocator *)opal_poolGetElement(&device_ptr->command_allocators, (Opal_PoolHandle)command_allocator);
	assert(command_allocator_ptr);

	// TODO: fix memory leak caused by orphaned Metal_CommandBuffer instances

	OPAL_UNUSED(command_allocator_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceAllocateDescriptorSet(Opal_Device this, const Opal_DescriptorSetAllocationDesc *desc, Opal_DescriptorSet *descriptor_set)
{
	assert(this);
	assert(desc);
	assert(descriptor_set);

	Metal_Device *device_ptr = (Metal_Device *)this;
	id<MTLDevice> metal_device = device_ptr->device;

	Metal_DescriptorHeap *descriptor_heap_ptr = (Metal_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)desc->heap);
	assert(descriptor_heap_ptr);

	Metal_DescriptorSetLayout *descriptor_set_layout_ptr = (Metal_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)desc->layout);
	assert(descriptor_set_layout_ptr);

	Metal_DescriptorSet result = {0};

	uint32_t num_static_descriptors = descriptor_set_layout_ptr->num_static_descriptors;
	if (num_static_descriptors > 0)
	{
		result.num_static_descriptors = num_static_descriptors;

		Opal_Result opal_result = opal_heapAlloc(&descriptor_heap_ptr->heap, descriptor_set_layout_ptr->num_blocks, &result.allocation);
		if (opal_result != OPAL_SUCCESS)
			return opal_result;

		uint32_t block_alignment = 256;
		result.buffer_offset = result.allocation.offset * block_alignment;
	}

	uint32_t num_dynamic_descriptors = descriptor_set_layout_ptr->num_dynamic_descriptors;
	if (num_dynamic_descriptors > 0)
	{
		result.num_dynamic_descriptors = num_dynamic_descriptors;
		result.dynamic_descriptors = (Opal_DescriptorSetEntry *)malloc(sizeof(Opal_DescriptorSetEntry) * num_dynamic_descriptors);
	}

	result.layout = desc->layout;
	result.heap = desc->heap;

	*descriptor_set = (Opal_DescriptorSet)opal_poolAddElement(&device_ptr->descriptor_sets, &result);

	if (desc->num_entries == 0)
		return OPAL_SUCCESS;

	assert(desc->entries);

	Opal_Result opal_result = metal_deviceUpdateDescriptorSet(this, *descriptor_set, desc->num_entries, desc->entries);
	if (opal_result != OPAL_SUCCESS)
	{
		metal_deviceFreeDescriptorSet(this, *descriptor_set);
		*descriptor_set = OPAL_NULL_HANDLE;
	}

	return opal_result;
}

static Opal_Result metal_deviceFreeDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set)
{
	assert(this);
	assert(descriptor_set);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_DescriptorSet *descriptor_set_ptr = (Metal_DescriptorSet *)opal_poolGetElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	assert(descriptor_set_ptr);

	metal_destroyDescriptorSet(device_ptr, descriptor_set_ptr);

	opal_poolRemoveElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	return OPAL_SUCCESS;
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
	assert(this);
	assert(descriptor_set);
 
	Metal_Device *device_ptr = (Metal_Device *)this;
	id<MTLDevice> metal_device = device_ptr->device;

	Metal_DescriptorSet *descriptor_set_ptr = (Metal_DescriptorSet *)opal_poolGetElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	assert(descriptor_set_ptr);

	Metal_DescriptorSetLayout *descriptor_set_layout_ptr = (Metal_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_ptr->layout);
	assert(descriptor_set_layout_ptr);

	Metal_DescriptorHeap *descriptor_heap_ptr = (Metal_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)descriptor_set_ptr->heap);
	assert(descriptor_heap_ptr);

	typedef struct DescriptorIndices_t
	{
		uint32_t layout;
		uint32_t entry;
	} DescriptorIndices;

	opal_bumpReset(&device_ptr->bump);
	uint32_t static_indices_ptr_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(DescriptorIndices *) * num_entries);
	uint32_t dynamic_indices_ptr_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(DescriptorIndices *) * num_entries);

	uint32_t num_static_indices = 0;
	uint32_t num_dynamic_indices = 0;
	DescriptorIndices *static_layout_indices = (DescriptorIndices *)(device_ptr->bump.data + static_indices_ptr_offset);
	DescriptorIndices *dynamic_layout_indices = (DescriptorIndices *)(device_ptr->bump.data + dynamic_indices_ptr_offset);

	// TODO: replace naive O(N) layout <-> binding search loop by O(1) hashmap lookup
	for (uint32_t i = 0; i < num_entries; ++i)
	{
		const Opal_DescriptorSetEntry *entry = &entries[i];

		uint32_t index = UINT32_MAX;
		for (uint32_t j = 0; j < descriptor_set_layout_ptr->num_descriptors; ++j)
		{
			const Metal_DescriptorInfo *info = &descriptor_set_layout_ptr->descriptors[j];
			if (info->binding == entry->binding)
			{
				index = j;
				break;
			}
		}

		assert(index != UINT32_MAX);
		if (index < descriptor_set_layout_ptr->num_static_descriptors)
		{
			static_layout_indices[num_static_indices].entry = i;
			static_layout_indices[num_static_indices].layout = index;
			num_static_indices++;
		}
		else
		{
			dynamic_layout_indices[num_dynamic_indices].entry = i;
			dynamic_layout_indices[num_dynamic_indices].layout = index;
			num_dynamic_indices++;
		}
	}

	@autoreleasepool
	{
		id<MTLArgumentEncoder> metal_encoder = descriptor_set_layout_ptr->encoder;

		if (num_static_indices > 0)
		{
			if (metal_encoder == nil)
				return OPAL_METAL_ERROR;

			[metal_encoder setArgumentBuffer: descriptor_heap_ptr->buffer offset: descriptor_set_ptr->buffer_offset];
		}

		for (uint32_t i = 0; i < num_static_indices; ++i)
		{
			uint32_t layout_index = static_layout_indices[i].layout;
			uint32_t entry_index = static_layout_indices[i].entry;

			const Metal_DescriptorInfo *info = &descriptor_set_layout_ptr->descriptors[layout_index];
			assert(info->api_type != MTLDataTypeNone);

			const Opal_DescriptorSetEntry *entry = &entries[entry_index];
			assert(info->binding == entry->binding);

			uint32_t binding = entry->binding;
		
			switch (info->api_type)
			{
				case MTLDataTypePointer:
				{
					Opal_Buffer buffer = OPAL_NULL_HANDLE;
					uint64_t offset = 0;

					if (info->opal_type == OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
					{
						buffer = entry->data.buffer_view.buffer;
						offset = entry->data.buffer_view.offset;
					}
					else
					{
						buffer = entry->data.storage_buffer_view.buffer;
						offset = entry->data.storage_buffer_view.offset;
					}

					Metal_Buffer *buffer_ptr = (Metal_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer);
					assert(buffer_ptr);

					[metal_encoder setBuffer: buffer_ptr->buffer offset: offset atIndex: binding];
				}
				break;

				case MTLDataTypeTexture:
				{
					Opal_TextureView data = entry->data.texture_view;

					Metal_TextureView *texture_view_ptr = (Metal_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)data);
					assert(texture_view_ptr);

					[metal_encoder setTexture: texture_view_ptr->texture_view atIndex: binding];
				}
				break;

				case MTLDataTypeSampler:
				{
					Opal_Sampler data = entry->data.sampler;

					Metal_Sampler *sampler_ptr = (Metal_Sampler *)opal_poolGetElement(&device_ptr->samplers, (Opal_PoolHandle)data);
					assert(sampler_ptr);

					[metal_encoder setSamplerState: sampler_ptr->sampler atIndex: binding];
				}
				break;

				case MTLDataTypeInstanceAccelerationStructure:
				{
					Opal_AccelerationStructure data = entry->data.acceleration_structure;

					// TODO:
					// Metal_AccelerationStructure *acceleration_structure_ptr = (Metal_AccelerationStructure *)opal_poolGetElement(&device_ptr->acceleration_structures, (Opal_PoolHandle)data);
					// assert(acceleration_structure_ptr);

					// [metal_encoder setAccelerationStructure: acceleration_structure_ptr->acceleration_structure atIndex: binding];
				}
				break;

				default:
				{
					assert(false);
				}
				return OPAL_METAL_ERROR;
			}
		}

		for (uint32_t i = 0; i < num_dynamic_indices; ++i)
		{
			uint32_t layout_index = dynamic_layout_indices[i].layout;
			uint32_t entry_index = dynamic_layout_indices[i].entry;

			const Metal_DescriptorInfo *info = &descriptor_set_layout_ptr->descriptors[layout_index];
			assert(info->api_type == MTLDataTypeNone);

			const Opal_DescriptorSetEntry *entry = &entries[entry_index];
			assert(info->binding == entry->binding);

			memcpy(&descriptor_set_ptr->dynamic_descriptors[i], entry, sizeof(Opal_DescriptorSetEntry));
		}
	}

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceBeginCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->pool == nil);
	assert(command_buffer_ptr->command_buffer == nil);
	assert(command_buffer_ptr->graphics_pass_encoder == nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	Metal_Queue *queue_ptr = (Metal_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)command_buffer_ptr->queue);
	assert(queue_ptr);

	id<MTLCommandBuffer> metal_command_buffer = nil;

	@autoreleasepool
	{
		metal_command_buffer = [queue_ptr->queue commandBufferWithUnretainedReferences];
		if (!metal_command_buffer)
			return OPAL_METAL_ERROR;

		[metal_command_buffer retain];
	}

	command_buffer_ptr->command_buffer = metal_command_buffer;
	command_buffer_ptr->pool = [[NSAutoreleasePool alloc] init];
	command_buffer_ptr->pipeline_layout = OPAL_NULL_HANDLE;

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->pool);
	assert(command_buffer_ptr->graphics_pass_encoder == nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	[command_buffer_ptr->pool drain];
	command_buffer_ptr->pool = nil;
	command_buffer_ptr->pipeline_layout = OPAL_NULL_HANDLE;

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceQuerySemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t *value)
{
	assert(this);
	assert(semaphore);
	assert(value);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_Semaphore *semaphore_ptr = (Metal_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);

	if (!semaphore_ptr->shared_event)
		return OPAL_METAL_ERROR;

	*value = semaphore_ptr->shared_event.signaledValue;
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceSignalSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value)
{
	assert(this);
	assert(semaphore);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_Semaphore *semaphore_ptr = (Metal_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);

	if (!semaphore_ptr->shared_event)
		return OPAL_METAL_ERROR;

	semaphore_ptr->shared_event.signaledValue = value;
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceWaitSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds)
{
	assert(this);
	assert(semaphore);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_Semaphore *semaphore_ptr = (Metal_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);

	if (!semaphore_ptr->shared_event)
		return OPAL_METAL_ERROR;

	@autoreleasepool
	{
		[semaphore_ptr->shared_event waitUntilSignaledValue: value timeoutMS: timeout_milliseconds];
	}

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceWaitQueue(Opal_Device this, Opal_Queue queue)
{
	assert(this);
	assert(queue);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_Queue *queue_ptr = (Metal_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queue);
	assert(queue_ptr);

	@autoreleasepool
	{
		id<MTLCommandBuffer> wait_command_buffer = [queue_ptr->queue commandBufferWithUnretainedReferences];

		[wait_command_buffer commit];
		[wait_command_buffer waitUntilCompleted];
	}

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceWaitIdle(Opal_Device this)
{
	assert(this);
	Metal_Device *device_ptr = (Metal_Device *)this;

	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
	{
		uint32_t queue_count = device_ptr->device_engines_info.queue_counts[i];
		const Opal_Queue *queues = device_ptr->queue_handles[i];

		for (uint32_t j = 0; j < queue_count; ++j)
			metal_deviceWaitQueue(this, queues[j]);
	}

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceSubmit(Opal_Device this, Opal_Queue queue, const Opal_SubmitDesc *desc)
{
	assert(this);
	assert(queue);
	assert(desc);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_Queue *queue_ptr = (Metal_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queue);
	assert(queue_ptr);

	@autoreleasepool
	{
		if (desc->num_wait_semaphores > 0 || desc->num_wait_swapchains > 0)
		{
			id<MTLCommandBuffer> wait_command_buffer = [queue_ptr->queue commandBufferWithUnretainedReferences];

			for (uint32_t i = 0; i < desc->num_wait_semaphores; ++i)
			{
				Metal_Semaphore *semaphore_ptr = (Metal_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)desc->wait_semaphores[i]);
				assert(semaphore_ptr);

				uint64_t value = desc->wait_values[i];

				[wait_command_buffer encodeWaitForEvent: semaphore_ptr->event value: value];
			}

			for (uint32_t i = 0; i < desc->num_wait_swapchains; ++i)
			{
				Metal_Swapchain *swapchain_ptr = (Metal_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)desc->wait_swapchains[i]);
				assert(swapchain_ptr);

				// TODO: encode wait command
			}

			[wait_command_buffer commit];
		}

		for (uint32_t i = 0; i < desc->num_command_buffers; ++i)
		{
			Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)desc->command_buffers[i]);
			assert(command_buffer_ptr);
			assert(command_buffer_ptr->command_buffer);
			assert(command_buffer_ptr->queue == queue);

			[command_buffer_ptr->command_buffer commit];
			[command_buffer_ptr->command_buffer release];
			command_buffer_ptr->command_buffer = nil;
		}

		if (desc->num_signal_semaphores > 0 || desc->num_signal_swapchains > 0)
		{
			id<MTLCommandBuffer> signal_command_buffer = [queue_ptr->queue commandBufferWithUnretainedReferences];

			for (uint32_t i = 0; i < desc->num_signal_semaphores; ++i)
			{
				Metal_Semaphore *semaphore_ptr = (Metal_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)desc->signal_semaphores[i]);
				assert(semaphore_ptr);

				uint64_t value = desc->signal_values[i];

				[signal_command_buffer encodeSignalEvent: semaphore_ptr->event value: value];
			}

			for (uint32_t i = 0; i < desc->num_signal_swapchains; ++i)
			{
				Metal_Swapchain *swapchain_ptr = (Metal_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)desc->signal_swapchains[i]);
				assert(swapchain_ptr);

				// TODO: encode signal command
			}

			[signal_command_buffer commit];
		}
	}

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceAcquire(Opal_Device this, Opal_Swapchain swapchain, Opal_TextureView *texture_view)
{
	assert(this);
	assert(swapchain);
	assert(texture_view);

	Metal_Device *device_ptr = (Metal_Device *)this;
	Metal_Instance *instance_ptr = device_ptr->instance;

	Metal_Swapchain *swapchain_ptr = (Metal_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)swapchain);
	assert(swapchain_ptr);

	Metal_Surface *surface_ptr = (Metal_Surface *)opal_poolGetElement(&instance_ptr->surfaces, (Opal_PoolHandle)swapchain_ptr->surface);
	assert(surface_ptr);

	// TODO: sync?
	// TODO: multiple acquires?

	assert(swapchain_ptr->current_texture_view == OPAL_NULL_HANDLE);
	assert(swapchain_ptr->current_drawable == nil);

	@autoreleasepool
	{
		id<CAMetalDrawable> drawable = [surface_ptr->layer nextDrawable];
		if (!drawable)
			return OPAL_METAL_ERROR;

		Metal_TextureView result = {0};
		result.texture_view = drawable.texture;

		Opal_PoolHandle handle = opal_poolAddElement(&device_ptr->texture_views, &result);
		assert(handle != OPAL_POOL_HANDLE_NULL);

		swapchain_ptr->current_texture_view = (Opal_TextureView)handle;
		swapchain_ptr->current_drawable = [drawable retain];
	}

	*texture_view = swapchain_ptr->current_texture_view;
	return OPAL_SUCCESS;
}

static Opal_Result metal_devicePresent(Opal_Device this, Opal_Swapchain swapchain)
{
	assert(this);
	assert(swapchain);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_Swapchain *swapchain_ptr = (Metal_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)swapchain);
	assert(swapchain_ptr);

	@autoreleasepool
	{
		id<MTLCommandBuffer> present_command_buffer = [swapchain_ptr->queue commandBufferWithUnretainedReferences];
		if (!present_command_buffer)
			return OPAL_METAL_ERROR;

		[present_command_buffer presentDrawable: swapchain_ptr->current_drawable];
		[present_command_buffer commit];

		[swapchain_ptr->current_drawable release];
	}

	Opal_Result opal_result = opal_poolRemoveElement(&device_ptr->texture_views, swapchain_ptr->current_texture_view);
	assert(opal_result == OPAL_SUCCESS);

	swapchain_ptr->current_texture_view = OPAL_NULL_HANDLE;
	swapchain_ptr->current_drawable = nil;

	// TODO: sync

	return opal_result;
}

static Opal_Result metal_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depth_stencil_attachment)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->graphics_pass_encoder == nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	MTLRenderPassDescriptor *info = [MTLRenderPassDescriptor new];
	if (!info)
		return OPAL_METAL_ERROR;

	for (uint32_t i = 0; i < num_color_attachments; ++i)
	{
		const Opal_FramebufferAttachment *opal_attachment = &color_attachments[i];

		Metal_TextureView *texture_view_ptr = (Metal_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)opal_attachment->texture_view);
		assert(texture_view_ptr);

		Metal_TextureView *resolve_texture_view_ptr = (Metal_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)opal_attachment->resolve_texture_view);

		info.colorAttachments[i].texture = texture_view_ptr->texture_view;
		info.colorAttachments[i].level = texture_view_ptr->base_mip;
		info.colorAttachments[i].slice = texture_view_ptr->base_layer;
		info.colorAttachments[i].loadAction = metal_helperToLoadAction(opal_attachment->load_op);
		info.colorAttachments[i].storeAction = metal_helperToStoreAction(opal_attachment->store_op);

		if (resolve_texture_view_ptr)
		{
			info.colorAttachments[i].resolveTexture = resolve_texture_view_ptr->texture_view;
			info.colorAttachments[i].resolveLevel = resolve_texture_view_ptr->base_mip;
			info.colorAttachments[i].resolveSlice = resolve_texture_view_ptr->base_layer;
		}

		Opal_ClearColor clear_color = opal_attachment->clear_value.color;
		info.colorAttachments[i].clearColor = MTLClearColorMake(clear_color.f[0], clear_color.f[1], clear_color.f[2], clear_color.f[3]);
	}

	if (depth_stencil_attachment)
	{
		const Opal_FramebufferAttachment *opal_attachment = depth_stencil_attachment;

		Metal_TextureView *texture_view_ptr = (Metal_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)opal_attachment->texture_view);
		assert(texture_view_ptr);

		Metal_TextureView *resolve_texture_view_ptr = (Metal_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)opal_attachment->resolve_texture_view);

		info.depthAttachment.texture = texture_view_ptr->texture_view;
		info.depthAttachment.level = texture_view_ptr->base_mip;
		info.depthAttachment.slice = texture_view_ptr->base_layer;
		info.depthAttachment.loadAction = metal_helperToLoadAction(opal_attachment->load_op);
		info.depthAttachment.storeAction = metal_helperToStoreAction(opal_attachment->store_op);
		info.depthAttachment.clearDepth = opal_attachment->clear_value.depth_stencil.depth;

		info.stencilAttachment.texture = texture_view_ptr->texture_view;
		info.stencilAttachment.level = texture_view_ptr->base_mip;
		info.stencilAttachment.slice = texture_view_ptr->base_layer;
		info.stencilAttachment.loadAction = metal_helperToLoadAction(opal_attachment->load_op);
		info.stencilAttachment.storeAction = metal_helperToStoreAction(opal_attachment->store_op);
		info.stencilAttachment.clearStencil = opal_attachment->clear_value.depth_stencil.stencil;

		if (resolve_texture_view_ptr)
		{
			info.depthAttachment.resolveTexture = resolve_texture_view_ptr->texture_view;
			info.depthAttachment.resolveLevel = resolve_texture_view_ptr->base_mip;
			info.depthAttachment.resolveSlice = resolve_texture_view_ptr->base_layer;

			info.stencilAttachment.resolveTexture = resolve_texture_view_ptr->texture_view;
			info.stencilAttachment.resolveLevel = resolve_texture_view_ptr->base_mip;
			info.stencilAttachment.resolveSlice = resolve_texture_view_ptr->base_layer;
		}
	}

	id<MTLRenderCommandEncoder> metal_pass_encoder = [command_buffer_ptr->command_buffer renderCommandEncoderWithDescriptor: info];
	[info release];

	if (!metal_pass_encoder)
		return OPAL_METAL_ERROR;

	command_buffer_ptr->graphics_pass_encoder = metal_pass_encoder;
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder != nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	[command_buffer_ptr->graphics_pass_encoder endEncoding];
	command_buffer_ptr->graphics_pass_encoder = nil;

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdBeginComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->graphics_pass_encoder == nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	id<MTLComputeCommandEncoder> metal_pass_encoder = [command_buffer_ptr->command_buffer computeCommandEncoder];
	if (!metal_pass_encoder)
		return OPAL_METAL_ERROR;

	command_buffer_ptr->compute_pass_encoder = metal_pass_encoder;
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdEndComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder == nil);
	assert(command_buffer_ptr->compute_pass_encoder != nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	[command_buffer_ptr->compute_pass_encoder endEncoding];
	command_buffer_ptr->compute_pass_encoder = nil;

	return OPAL_SUCCESS;
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

static Opal_Result metal_deviceCmdBeginCopyPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder == nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	id<MTLBlitCommandEncoder> metal_pass_encoder = [command_buffer_ptr->command_buffer blitCommandEncoder];
	if (metal_pass_encoder == nil)
		return OPAL_METAL_ERROR;

	command_buffer_ptr->copy_pass_encoder = metal_pass_encoder;
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdEndCopyPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder == nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder != nil);

	[command_buffer_ptr->copy_pass_encoder endEncoding];
	command_buffer_ptr->copy_pass_encoder = nil;

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdSetDescriptorHeap(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_DescriptorHeap descriptor_heap)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(descriptor_heap);

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdSetPipelineLayout(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout)
{
	assert(this);
	assert(command_buffer);
	assert(pipeline_layout);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder != nil || command_buffer_ptr->compute_pass_encoder != nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	Metal_PipelineLayout *pipeline_layout_ptr = (Metal_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)pipeline_layout);
	assert(pipeline_layout_ptr);

	command_buffer_ptr->pipeline_layout = pipeline_layout;
	command_buffer_ptr->vertex_binding_offset = pipeline_layout_ptr->vertex_binding_offset;

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdSetPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Pipeline pipeline)
{
	assert(this);
	assert(command_buffer);
	assert(pipeline);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->pipeline_layout != OPAL_NULL_HANDLE);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder != nil || command_buffer_ptr->compute_pass_encoder != nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	Metal_Pipeline *pipeline_ptr = (Metal_Pipeline *)opal_poolGetElement(&device_ptr->pipelines, (Opal_PoolHandle)pipeline);
	assert(pipeline_ptr);

	if (command_buffer_ptr->graphics_pass_encoder)
	{
		[command_buffer_ptr->graphics_pass_encoder setRenderPipelineState: pipeline_ptr->render_pipeline];
		[command_buffer_ptr->graphics_pass_encoder setDepthStencilState: pipeline_ptr->depth_stencil_state];
		[command_buffer_ptr->graphics_pass_encoder setFrontFacingWinding: pipeline_ptr->winding];
		[command_buffer_ptr->graphics_pass_encoder setCullMode: pipeline_ptr->cull_mode];

		command_buffer_ptr->primitive_type = pipeline_ptr->primitive_type;
	}

	if (command_buffer_ptr->compute_pass_encoder)
	{
		[command_buffer_ptr->compute_pass_encoder setComputePipelineState: pipeline_ptr->compute_pipeline];
		
		command_buffer_ptr->threadgroup_size = pipeline_ptr->threadgroup_size;
	}

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdSetDescriptorSet(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets)
{
	assert(this);
	assert(command_buffer);
	assert(descriptor_set);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->pipeline_layout != OPAL_NULL_HANDLE);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder != nil || command_buffer_ptr->compute_pass_encoder != nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	Metal_DescriptorSet *descriptor_set_ptr = (Metal_DescriptorSet *)opal_poolGetElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	assert(descriptor_set_ptr);

	uint32_t static_descriptors = descriptor_set_ptr->num_static_descriptors;
	uint32_t dynamic_descriptors = descriptor_set_ptr->num_dynamic_descriptors;

	if (static_descriptors > 0)
	{
		Metal_DescriptorHeap *descriptor_heap_ptr = (Metal_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)descriptor_set_ptr->heap);
		assert(descriptor_heap_ptr);

		id<MTLBuffer> heap_buffer = descriptor_heap_ptr->buffer;
		uint64_t heap_offset = descriptor_set_ptr->buffer_offset;

		if (command_buffer_ptr->graphics_pass_encoder != NULL)
		{
			[command_buffer_ptr->graphics_pass_encoder setVertexBuffer: heap_buffer offset: heap_offset atIndex: index];
			[command_buffer_ptr->graphics_pass_encoder setFragmentBuffer: heap_buffer offset: heap_offset atIndex: index];
		}

		if (command_buffer_ptr->compute_pass_encoder != NULL)
		{
			[command_buffer_ptr->compute_pass_encoder setBuffer: heap_buffer offset: heap_offset atIndex: index];
		}
	}

	if (num_dynamic_offsets > 0)
	{
		Metal_PipelineLayout *pipeline_layout_ptr = (Metal_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)command_buffer_ptr->pipeline_layout);
		assert(pipeline_layout_ptr);
		assert(index < pipeline_layout_ptr->num_layouts);

		Metal_DescriptorSetLayout *descriptor_set_layout_ptr = (Metal_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_ptr->layout);
		assert(descriptor_set_layout_ptr);

		assert(dynamic_offsets);
		assert(descriptor_set_layout_ptr->num_dynamic_descriptors == num_dynamic_offsets);
		assert(descriptor_set_layout_ptr->num_dynamic_descriptors == descriptor_set_ptr->num_dynamic_descriptors);

		uint32_t info_offset = descriptor_set_layout_ptr->num_static_descriptors;
		uint32_t binding_offset = pipeline_layout_ptr->dynamic_binding_offsets[index];

		for (uint32_t i = 0; i < num_dynamic_offsets; ++i)
		{
			const Metal_DescriptorInfo *info = &descriptor_set_layout_ptr->descriptors[info_offset + i];
			const Opal_DescriptorSetEntry *data = &descriptor_set_ptr->dynamic_descriptors[i];

			assert(info->api_type == MTLDataTypeNone);

			id<MTLBuffer> metal_buffer = nil;
			uint64_t buffer_offset = 0;
			uint32_t buffer_binding = binding_offset + i;

			switch (info->opal_type)
			{
				case OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				{
					Opal_BufferView buffer_view = data->data.buffer_view;
					Metal_Buffer *buffer_ptr = (Metal_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer_view.buffer);
					assert(buffer_ptr);

					metal_buffer = buffer_ptr->buffer;
					buffer_offset = buffer_view.offset + dynamic_offsets[i];
				}
				break;

				case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC:
				{
					Opal_StorageBufferView buffer_view = data->data.storage_buffer_view;
					Metal_Buffer *buffer_ptr = (Metal_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer_view.buffer);
					assert(buffer_ptr);

					metal_buffer = buffer_ptr->buffer;
					buffer_offset = buffer_view.offset + dynamic_offsets[i];
				}
				break;

				default: assert(0); break;
			}

			assert(metal_buffer != nil);

			if (command_buffer_ptr->graphics_pass_encoder != NULL)
			{
				[command_buffer_ptr->graphics_pass_encoder setVertexBuffer: metal_buffer offset: buffer_offset atIndex: buffer_binding];
				[command_buffer_ptr->graphics_pass_encoder setFragmentBuffer: metal_buffer offset: buffer_offset atIndex: buffer_binding];
			}

			if (command_buffer_ptr->compute_pass_encoder != NULL)
			{
				[command_buffer_ptr->compute_pass_encoder setBuffer: metal_buffer offset: buffer_offset atIndex: buffer_binding];
			}
		}
	}

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdSetShaderBindingTable(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTable shader_binding_table)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(shader_binding_table);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_VertexBufferView *vertex_buffers)
{
	assert(this);
	assert(command_buffer);
	assert(num_vertex_buffers == 0 || vertex_buffers);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->pipeline_layout != OPAL_NULL_HANDLE);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder != nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	for (uint32_t i = 0; i < num_vertex_buffers; ++i)
	{
		Metal_Buffer *buffer_ptr = (Metal_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)vertex_buffers[i].buffer);
		assert(buffer_ptr);

		[command_buffer_ptr->graphics_pass_encoder
			setVertexBuffer: buffer_ptr->buffer
			offset: vertex_buffers[i].offset
			atIndex: command_buffer_ptr->vertex_binding_offset + i
		];
	}

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_IndexBufferView index_buffer)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->pipeline_layout != OPAL_NULL_HANDLE);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder != nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	assert(index_buffer.offset % 4 == 0);

	command_buffer_ptr->index_buffer_view = index_buffer;
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdSetViewport(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Viewport viewport)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder != nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	MTLViewport metal_viewport = {};
	metal_viewport.originX = viewport.x;
	metal_viewport.originY = viewport.y;
	metal_viewport.width = viewport.width;
	metal_viewport.height = viewport.height;
	metal_viewport.znear = viewport.min_depth;
	metal_viewport.zfar = viewport.max_depth;

	[command_buffer_ptr->graphics_pass_encoder setViewport: metal_viewport];
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdSetScissor(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder != nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	MTLScissorRect metal_rect = {};
	metal_rect.x = x;
	metal_rect.y = y;
	metal_rect.width = width;
	metal_rect.height = height;

	[command_buffer_ptr->graphics_pass_encoder setScissorRect: metal_rect];
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdDraw(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertices, uint32_t num_instances, uint32_t base_vertex, uint32_t base_instance)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder != nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	[command_buffer_ptr->graphics_pass_encoder
		drawPrimitives: command_buffer_ptr->primitive_type
		vertexStart: base_vertex
		vertexCount: num_vertices
		instanceCount: num_instances
		baseInstance: base_instance
	];
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdDrawIndexed(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t num_instances, uint32_t base_index, int32_t vertex_offset, uint32_t base_instance)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder != nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);

	Opal_IndexBufferView index_buffer = command_buffer_ptr->index_buffer_view;
	assert(index_buffer.buffer);

	Metal_Buffer *index_buffer_ptr = (Metal_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)index_buffer.buffer);
	assert(index_buffer_ptr);

	[command_buffer_ptr->graphics_pass_encoder
		drawIndexedPrimitives: command_buffer_ptr->primitive_type
		indexCount: num_indices
		indexType: metal_helperToIndexType(index_buffer.format)
		indexBuffer: index_buffer_ptr->buffer
		indexBufferOffset: index_buffer.offset + base_index * metal_helperToIndexSize(index_buffer.format)
		instanceCount: num_instances
		baseVertex: vertex_offset
		baseInstance: base_instance
	];
	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdMeshletDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_threadgroups_x, uint32_t num_threadgroups_y, uint32_t num_threadgroups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_threadgroups_x);
	OPAL_UNUSED(num_threadgroups_y);
	OPAL_UNUSED(num_threadgroups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdComputeDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_threadgroups_x, uint32_t num_threadgroups_y, uint32_t num_threadgroups_z)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder == nil);
	assert(command_buffer_ptr->compute_pass_encoder != nil);
	assert(command_buffer_ptr->copy_pass_encoder == nil);
	assert(command_buffer_ptr->threadgroup_size.width != 0);
	assert(command_buffer_ptr->threadgroup_size.height != 0);
	assert(command_buffer_ptr->threadgroup_size.depth != 0);

	MTLSize numThreadgroups = {0};
	numThreadgroups.width = num_threadgroups_x;
	numThreadgroups.height = num_threadgroups_y;
	numThreadgroups.depth = num_threadgroups_z;

	[command_buffer_ptr->compute_pass_encoder
		dispatchThreadgroups: numThreadgroups
		threadsPerThreadgroup: command_buffer_ptr->threadgroup_size
	];

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdRaytraceDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t width, uint32_t height, uint32_t depth)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
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
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder == nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder != nil);

	Metal_Buffer *src_buffer_ptr = (Metal_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)src_buffer);
	assert(src_buffer_ptr);
	assert(src_buffer_ptr->buffer);

	Metal_Buffer *dst_buffer_ptr = (Metal_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)dst_buffer);
	assert(dst_buffer_ptr);
	assert(dst_buffer_ptr->buffer);

	[command_buffer_ptr->copy_pass_encoder
		copyFromBuffer: src_buffer_ptr->buffer
		sourceOffset: (NSUInteger)src_offset
		toBuffer: dst_buffer_ptr->buffer
		destinationOffset: (NSUInteger)dst_offset
		size: (NSUInteger)size];

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferTextureRegion src, Opal_TextureRegion dst, Opal_Extent3D size)
{
	assert(this);
	assert(command_buffer);

	Metal_Device *device_ptr = (Metal_Device *)this;

	Metal_CommandBuffer *command_buffer_ptr = (Metal_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->command_buffer);
	assert(command_buffer_ptr->graphics_pass_encoder == nil);
	assert(command_buffer_ptr->compute_pass_encoder == nil);
	assert(command_buffer_ptr->copy_pass_encoder != nil);

	Metal_Buffer *src_buffer_ptr = (Metal_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)src.buffer);
	assert(src_buffer_ptr);
	assert(src_buffer_ptr->buffer);

	Metal_TextureView *dst_texture_view_ptr = (Metal_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)dst.texture_view);
	assert(dst_texture_view_ptr);
	assert(dst_texture_view_ptr->texture);

	MTLSize src_size = {0};
	src_size.width = size.width;
	src_size.height = size.height;
	src_size.depth = size.depth;

	MTLOrigin dst_origin = {0};
	dst_origin.x = dst.offset.x;
	dst_origin.y = dst.offset.y;
	dst_origin.z = dst.offset.z;

	[command_buffer_ptr->copy_pass_encoder
		copyFromBuffer: src_buffer_ptr->buffer
		sourceOffset: (NSUInteger)src.offset
		sourceBytesPerRow: (NSUInteger)src.row_size
		sourceBytesPerImage: (NSUInteger)0
		sourceSize: (MTLSize)src_size
		toTexture: dst_texture_view_ptr->texture_view
		destinationSlice: 0
		destinationLevel: 0
		destinationOrigin: dst_origin];

	return OPAL_SUCCESS;
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

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdBufferQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdBufferQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdTextureTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdTextureQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(queue);

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCmdTextureQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
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
	metal_deviceGetInfo,
	metal_deviceGetQueue,
	metal_deviceGetAccelerationStructurePrebuildInfo,

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
	metal_deviceCreateShaderBindingTable,
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
	metal_deviceDestroyShaderBindingTable,
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
	metal_deviceCmdBeginCopyPass,
	metal_deviceCmdEndCopyPass,
	metal_deviceCmdSetDescriptorHeap,
	metal_deviceCmdSetPipelineLayout,
	metal_deviceCmdSetPipeline,
	metal_deviceCmdSetDescriptorSet,
	metal_deviceCmdSetShaderBindingTable,
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

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	device_ptr->instance = instance_ptr;
	device_ptr->device = metal_device;

	// allocator
	Opal_Result result = metal_allocatorInitialize(device_ptr, instance_ptr->heap_size, instance_ptr->max_heap_allocations, instance_ptr->max_heaps);
	assert(result == OPAL_SUCCESS);

	// device engine info
	result = metal_helperFillDeviceEnginesInfo(&device_ptr->device_engines_info);
	assert(result == OPAL_SUCCESS);

	// bump
	opal_bumpInitialize(&device_ptr->bump, 256);

	// pools
	opal_poolInitialize(&device_ptr->queues, sizeof(Metal_Queue), 32);
	opal_poolInitialize(&device_ptr->semaphores, sizeof(Metal_Semaphore), 32);
	opal_poolInitialize(&device_ptr->buffers, sizeof(Metal_Buffer), 32);
	opal_poolInitialize(&device_ptr->textures, sizeof(Metal_Texture), 32);
	opal_poolInitialize(&device_ptr->texture_views, sizeof(Metal_TextureView), 32);
	opal_poolInitialize(&device_ptr->samplers, sizeof(Metal_Sampler), 32);
	opal_poolInitialize(&device_ptr->acceleration_structures, sizeof(Metal_AccelerationStructure), 32);
	opal_poolInitialize(&device_ptr->command_allocators, sizeof(Metal_CommandAllocator), 32);
	opal_poolInitialize(&device_ptr->command_buffers, sizeof(Metal_CommandBuffer), 32);
	opal_poolInitialize(&device_ptr->shaders, sizeof(Metal_Shader), 32);
	opal_poolInitialize(&device_ptr->descriptor_heaps, sizeof(Metal_DescriptorHeap), 32);
	opal_poolInitialize(&device_ptr->descriptor_set_layouts, sizeof(Metal_DescriptorSetLayout), 32);
	opal_poolInitialize(&device_ptr->descriptor_sets, sizeof(Metal_DescriptorSet), 32);
	opal_poolInitialize(&device_ptr->pipeline_layouts, sizeof(Metal_PipelineLayout), 32);
	opal_poolInitialize(&device_ptr->pipelines, sizeof(Metal_Pipeline), 32);
	opal_poolInitialize(&device_ptr->swapchains, sizeof(Metal_Swapchain), 32);

	// queues
	const Metal_DeviceEnginesInfo *engines_info = &device_ptr->device_engines_info;

	@autoreleasepool
	{
		for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
		{
			uint32_t queue_count = engines_info->queue_counts[i];

			Metal_Queue queue = {0};

			Opal_Queue *queue_handles = (Opal_Queue *)malloc(sizeof(Opal_Queue) * queue_count);

			for (uint32_t j = 0; j < queue_count; j++)
			{
				queue.queue = [[device_ptr->device newCommandQueue] retain];
				assert(queue.queue);

				[queue.queue addResidencySet: device_ptr->allocator.residency_set];
				queue_handles[j] = (Opal_Queue)opal_poolAddElement(&device_ptr->queues, &queue);
			}

			device_ptr->queue_handles[i] = queue_handles;
		}
	}

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
