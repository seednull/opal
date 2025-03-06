#include "directx12_internal.h"
#include <assert.h>

/*
 */
static Opal_Result directx12_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view);
static Opal_Result directx12_deviceFreeDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set);
static Opal_Result directx12_deviceUpdateDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set, uint32_t num_entries, const Opal_DescriptorSetEntry *entries);

/*
 */
static int directx12_compareInlineDescriptors(const void *d0, const void *d1)
{
	UINT b0 = ((const DirectX12_DescriptorInfo *)d0)->binding;
	UINT b1 = ((const DirectX12_DescriptorInfo *)d1)->binding;

	return (b0 > b1) - (b0 < b1);
}

/*
 */
static Opal_Result directx12_createFramebufferDescriptorHeap(DirectX12_Device *device_ptr, DirectX12_FramebufferDescriptorHeap *heap_ptr)
{
	assert(device_ptr);
	assert(heap_ptr);

	ID3D12Device *d3d12_device = device_ptr->device;

	D3D12_DESCRIPTOR_HEAP_DESC heap_info = {0};
	heap_info.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heap_info.NumDescriptors = 8;
	heap_info.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = ID3D12Device_CreateDescriptorHeap(d3d12_device, &heap_info, &IID_ID3D12DescriptorHeap, &heap_ptr->rtv_heap);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	heap_info.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heap_info.NumDescriptors = 1;

	hr = ID3D12Device_CreateDescriptorHeap(d3d12_device, &heap_info, &IID_ID3D12DescriptorHeap, &heap_ptr->dsv_heap);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	return OPAL_SUCCESS;
}

static void directx12_destroyFramebufferDescriptorHeap(DirectX12_Device *device_ptr, DirectX12_FramebufferDescriptorHeap *heap_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(heap_ptr);

	ID3D12DescriptorHeap_Release(heap_ptr->rtv_heap);
	ID3D12DescriptorHeap_Release(heap_ptr->dsv_heap);
}

static void directx12_destroyQueue(DirectX12_Device *device_ptr, DirectX12_Queue *queue_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(queue_ptr);

	CloseHandle(queue_ptr->event);
	ID3D12Fence_Release(queue_ptr->fence);
	ID3D12CommandQueue_Release(queue_ptr->queue);
}

static void directx12_destroySemaphore(DirectX12_Device *device_ptr, DirectX12_Semaphore *semaphore_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(semaphore_ptr);

	CloseHandle(semaphore_ptr->event);
	ID3D12Fence_Release(semaphore_ptr->fence);
}

static void directx12_destroyBuffer(DirectX12_Device *device_ptr, DirectX12_Buffer *buffer_ptr)
{
	assert(device_ptr);
	assert(buffer_ptr);

	ID3D12Resource_Release(buffer_ptr->buffer);
	directx12_allocatorFreeMemory(device_ptr, buffer_ptr->allocation);
}

static void directx12_destroyTexture(DirectX12_Device *device_ptr, DirectX12_Texture *texture_ptr)
{
	assert(device_ptr);
	assert(texture_ptr);

	ID3D12Resource_Release(texture_ptr->texture);
	directx12_allocatorFreeMemory(device_ptr, texture_ptr->allocation);
}

static void directx12_destroyTextureView(DirectX12_Device *device_ptr, DirectX12_TextureView *texture_view_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(texture_view_ptr);

	ID3D12Resource_Release(texture_view_ptr->texture);
}

static void directx12_destroySampler(DirectX12_Device *device_ptr, DirectX12_Sampler *sampler_ptr)
{
	OPAL_UNUSED(device_ptr);
	OPAL_UNUSED(sampler_ptr);

	// do nothing
}

static void directx12_destroyCommandPool(DirectX12_Device *device_ptr, DirectX12_CommandPool *command_pool_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(command_pool_ptr);

	for (uint32_t i = 0; i < D3D12_MAX_COMMAND_POOL_ALLOCATORS; ++i)
		ID3D12CommandAllocator_Release(command_pool_ptr->allocators[i]);
}

static void directx12_destroyCommandBuffer(DirectX12_Device *device_ptr, DirectX12_CommandBuffer *command_buffer_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(command_buffer_ptr);

	ID3D12GraphicsCommandList4_Release(command_buffer_ptr->list);
}

static void directx12_destroyShader(DirectX12_Device *device_ptr, DirectX12_Shader *shader_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(shader_ptr);

	free(shader_ptr->data);
}

static void directx12_destroyDescriptorHeap(DirectX12_Device *device_ptr, DirectX12_DescriptorHeap *descriptor_heap_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(descriptor_heap_ptr);

	if (descriptor_heap_ptr->resource_memory)
	{
		ID3D12DescriptorHeap_Release(descriptor_heap_ptr->resource_memory);
		opal_heapShutdown(&descriptor_heap_ptr->resource_heap);
	}

	if (descriptor_heap_ptr->sampler_memory)
	{
		ID3D12DescriptorHeap_Release(descriptor_heap_ptr->sampler_memory);
		opal_heapShutdown(&descriptor_heap_ptr->sampler_heap);
	}
}

static void directx12_destroyDescriptorSetLayout(DirectX12_Device *device_ptr, DirectX12_DescriptorSetLayout *descriptor_set_layout_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(descriptor_set_layout_ptr);

	free(descriptor_set_layout_ptr->descriptors);
	descriptor_set_layout_ptr->descriptors = NULL;
}

static void directx12_destroyPipelineLayout(DirectX12_Device *device_ptr, DirectX12_PipelineLayout *pipeline_layout_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(pipeline_layout_ptr);

	if (pipeline_layout_ptr->num_layout_table_offsets > 0)
	{
		free(pipeline_layout_ptr->layout_table_offsets);
		pipeline_layout_ptr->layout_table_offsets = NULL;
	}

	ID3D12RootSignature_Release(pipeline_layout_ptr->root_signature);
}

static void directx12_destroyPipeline(DirectX12_Device *device_ptr, DirectX12_Pipeline *pipeline_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(pipeline_ptr);

	ID3D12RootSignature_Release(pipeline_ptr->root_signature);
	ID3D12PipelineState_Release(pipeline_ptr->pipeline_state);
}

static void directx12_destroySwapchain(DirectX12_Device *device_ptr, DirectX12_Swapchain *swapchain_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(swapchain_ptr);

	free(swapchain_ptr->texture_views);

	IDXGISwapChain3_Release(swapchain_ptr->swapchain);
}

/*
 */
static Opal_Result directx12_deviceGetInfo(Opal_Device this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	DirectX12_Device *ptr = (DirectX12_Device *)this;
	return directx12_helperFillDeviceInfo(ptr->adapter, ptr->device, info);
}

static Opal_Result directx12_deviceGetQueue(Opal_Device this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	assert(this);
	assert(queue);
	assert(engine_type < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	DirectX12_Device *ptr = (DirectX12_Device *)this;
	uint32_t queue_count = ptr->device_engines_info.queue_counts[engine_type];

	if (index >= queue_count)
		return OPAL_INVALID_QUEUE_INDEX;

	Opal_Queue *queue_handles = ptr->queue_handles[engine_type];
	assert(queue_handles);

	*queue = queue_handles[index];
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceGetAccelerationStructurePrebuildInfo(Opal_Device this, const Opal_AccelerationStructureBuildDesc *desc, Opal_AccelerationStructurePrebuildInfo *info)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(info);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceGetShaderBindingTablePrebuildInfo(Opal_Device this, const Opal_ShaderBindingTableBuildDesc *desc, Opal_ShaderBindingTablePrebuildInfo *info)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(info);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceGetSupportedSurfaceFormats(Opal_Device this, Opal_Surface surface, uint32_t *num_formats, Opal_SurfaceFormat *formats)
{
	assert(this);
	assert(surface);
	assert(num_formats);

	static Opal_SurfaceFormat allowed_formats[] =
	{
		OPAL_TEXTURE_FORMAT_BGRA8_UNORM, OPAL_COLOR_SPACE_SRGB,
		OPAL_TEXTURE_FORMAT_BGRA8_UNORM_SRGB, OPAL_COLOR_SPACE_SRGB,
		OPAL_TEXTURE_FORMAT_RGBA8_UNORM, OPAL_COLOR_SPACE_SRGB,
		OPAL_TEXTURE_FORMAT_RGBA8_UNORM_SRGB, OPAL_COLOR_SPACE_SRGB,
		// TODO: add rgb10a2 + hdr10 support
	};
	static uint32_t num_allowed_formats = sizeof(allowed_formats) / sizeof(Opal_SurfaceFormat);

	*num_formats = num_allowed_formats;

	if (formats)
		memcpy(formats, &allowed_formats, sizeof(Opal_SurfaceFormat) * num_allowed_formats);

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceGetSupportedPresentModes(Opal_Device this, Opal_Surface surface, uint32_t *num_present_modes, Opal_PresentMode *present_modes)
{
	assert(this);
	assert(surface);
	assert(num_present_modes);

	static Opal_PresentMode allowed_present_modes[] =
	{
		OPAL_PRESENT_MODE_FIFO,
		OPAL_PRESENT_MODE_IMMEDIATE,
		OPAL_PRESENT_MODE_MAILBOX,
	};
	static uint32_t num_allowed_present_modes = sizeof(allowed_present_modes) / sizeof(Opal_PresentMode);

	*num_present_modes = num_allowed_present_modes;

	if (present_modes)
		memcpy(present_modes, &allowed_present_modes, sizeof(Opal_PresentMode) * num_allowed_present_modes);

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceGetPreferredSurfaceFormat(Opal_Device this, Opal_Surface surface, Opal_SurfaceFormat *format)
{
	assert(this);
	assert(surface);
	assert(format);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	uint32_t num_formats = 0;
	directx12_deviceGetSupportedSurfaceFormats(this, surface, &num_formats, NULL);

	if (num_formats == 0)
		return OPAL_SURFACE_NOT_DRAWABLE;

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(Opal_SurfaceFormat) * num_formats);
	Opal_SurfaceFormat *formats = (Opal_SurfaceFormat *)(device_ptr->bump.data);

	directx12_deviceGetSupportedSurfaceFormats(this, surface, &num_formats, formats);

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

static Opal_Result directx12_deviceGetPreferredSurfacePresentMode(Opal_Device this, Opal_Surface surface, Opal_PresentMode *present_mode)
{
	assert(this);
	assert(surface);
	assert(present_mode);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	uint32_t num_present_modes = 0;
	directx12_deviceGetSupportedPresentModes(this, surface, &num_present_modes, NULL);

	if (num_present_modes == 0)
		return OPAL_SURFACE_NOT_PRESENTABLE;

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(Opal_PresentMode) * num_present_modes);
	Opal_PresentMode *present_modes = (Opal_PresentMode *)(device_ptr->bump.data);

	directx12_deviceGetSupportedPresentModes(this, surface, &num_present_modes, present_modes);

	// TODO: it's probably a good idea to search for mailbox for high performance device,
	//       but for low power device fifo will drain less battery by adding latency
	Opal_PresentMode optimal_present_mode = OPAL_PRESENT_MODE_MAILBOX;

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

static Opal_Result directx12_deviceCreateSemaphore(Opal_Device this, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore)
{
	assert(this);
	assert(desc);
	assert(semaphore);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	ID3D12Device *d3d12_device = device_ptr->device;

	ID3D12Fence *d3d12_fence = NULL;

	D3D12_FENCE_FLAGS flags = D3D12_FENCE_FLAG_NONE;
	HRESULT hr = ID3D12Device_CreateFence(d3d12_device, desc->initial_value, flags, &IID_ID3D12Fence, &d3d12_fence);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	DirectX12_Semaphore result = {0};
	result.fence = d3d12_fence;
	result.event = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);

	*semaphore = (Opal_Semaphore)opal_poolAddElement(&device_ptr->semaphores, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	assert(this);
	assert(desc);
	assert(buffer);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	ID3D12Device *d3d12_device = device_ptr->device;

	ID3D12Resource *d3d12_buffer = NULL;
	DirectX12_Allocation allocation = {0};

	// fill buffer info
	D3D12_RESOURCE_DESC buffer_info = {0};
	buffer_info.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buffer_info.Alignment = 0;
	buffer_info.Width = desc->size;
	buffer_info.Height = 1;
	buffer_info.DepthOrArraySize = 1;
	buffer_info.MipLevels = 1;
	buffer_info.Format = DXGI_FORMAT_UNKNOWN;
	buffer_info.SampleDesc.Count = 1;
	buffer_info.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	if (desc->usage & OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE)
		buffer_info.Flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;

	if (desc->usage & OPAL_BUFFER_USAGE_STORAGE)
		buffer_info.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_RESOURCE_ALLOCATION_INFO allocation_info = {0};
	ID3D12Device_GetResourceAllocationInfo(d3d12_device, &allocation_info, 0, 1, &buffer_info);

	// fill allocation info
	DirectX12_AllocationDesc allocation_desc = {0};
	allocation_desc.size = allocation_info.SizeInBytes;
	allocation_desc.alignment = allocation_info.Alignment;
	allocation_desc.resource_type = DIRECTX12_RESOURCE_TYPE_BUFFER;
	allocation_desc.allocation_type = desc->memory_type;
	allocation_desc.hint = desc->hint;

	Opal_Result opal_result = directx12_deviceAllocateMemory(device_ptr, &allocation_desc, &allocation);
	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	assert(allocation.offset % allocation_info.Alignment == 0);

	D3D12_RESOURCE_STATES initial_state = directx12_helperToInitialBufferResourceState(desc->memory_type, desc->usage);
	HRESULT hr = ID3D12Device_CreatePlacedResource(d3d12_device, allocation.memory, allocation.offset, &buffer_info, initial_state, NULL, &IID_ID3D12Resource, &d3d12_buffer);
	if (!SUCCEEDED(hr))
	{
		directx12_allocatorFreeMemory(device_ptr, allocation);
		return OPAL_DIRECTX12_ERROR;
	}

	// create opal struct
	DirectX12_Buffer result = {0};
	result.buffer = d3d12_buffer;
	result.address = ID3D12Resource_GetGPUVirtualAddress(d3d12_buffer);
	result.allocation = allocation;

	*buffer = (Opal_Buffer)opal_poolAddElement(&device_ptr->buffers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	assert(this);
	assert(desc);
	assert(texture);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	ID3D12Device *d3d12_device = device_ptr->device;

	ID3D12Resource *d3d12_texture = NULL;
	DirectX12_Allocation allocation = {0};

	// fill texture info
	D3D12_RESOURCE_DESC texture_info = {0};
	texture_info.Dimension = directx12_helperToTextureDimension(desc->type);
	texture_info.Alignment = 0;
	texture_info.Width = desc->width;
	texture_info.Height = desc->height;
	texture_info.DepthOrArraySize = (UINT16)((desc->type != OPAL_TEXTURE_TYPE_3D) ? desc->layer_count : desc->depth);
	texture_info.MipLevels = (UINT16)desc->mip_count;
	texture_info.Format = directx12_helperToDXGITextureFormat(desc->format);
	texture_info.SampleDesc.Count = directx12_helperToSampleCount(desc->samples);
	texture_info.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texture_info.Flags = directx12_helperToTextureFlags(desc->usage, desc->format);

	D3D12_RESOURCE_ALLOCATION_INFO allocation_info = {0};
	ID3D12Device_GetResourceAllocationInfo(d3d12_device, &allocation_info, 0, 1, &texture_info);

	// fill allocation info
	DirectX12_AllocationDesc allocation_desc = {0};
	allocation_desc.size = allocation_info.SizeInBytes;
	allocation_desc.alignment = allocation_info.Alignment;
	allocation_desc.resource_type = directx12_helperToTextureResourceType(desc->usage, desc->samples);
	allocation_desc.allocation_type = OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL;
	allocation_desc.hint = desc->hint;

	Opal_Result opal_result = directx12_deviceAllocateMemory(device_ptr, &allocation_desc, &allocation);
	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	assert(allocation.offset % allocation_info.Alignment == 0);

	D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	HRESULT hr = ID3D12Device_CreatePlacedResource(d3d12_device, allocation.memory, allocation.offset, &texture_info, initial_state, NULL, &IID_ID3D12Resource, &d3d12_texture);
	if (!SUCCEEDED(hr))
	{
		directx12_allocatorFreeMemory(device_ptr, allocation);
		return OPAL_DIRECTX12_ERROR;
	}

	// create opal struct
	DirectX12_Texture result = {0};
	result.texture = d3d12_texture;
	result.format = texture_info.Format;
	result.width = texture_info.Width;
	result.height = texture_info.Height;
	result.depth = (desc->type != OPAL_TEXTURE_TYPE_3D) ? texture_info.DepthOrArraySize : 1;
	result.samples = texture_info.SampleDesc.Count;
	result.allocation = allocation;

	*texture = (Opal_Texture)opal_poolAddElement(&device_ptr->textures, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	assert(this);
	assert(desc);
	assert(texture_view);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_Texture *texture_ptr = (DirectX12_Texture *)opal_poolGetElement(&device_ptr->textures, (Opal_PoolHandle)desc->texture);
	assert(texture_ptr);

	DXGI_FORMAT format = texture_ptr->format;
	UINT samples = texture_ptr->samples;

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {0};
	srv_desc.Format = format;
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {0};
	uav_desc.Format = format;

	D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {0};
	rtv_desc.Format = format;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {0};
	dsv_desc.Format = format;
	dsv_desc.Flags = D3D12_DSV_FLAG_NONE;

	switch (desc->type)
	{
		case OPAL_TEXTURE_VIEW_TYPE_1D:
		{
			srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			srv_desc.Texture1D.MostDetailedMip = desc->base_mip;
			srv_desc.Texture1D.MipLevels = desc->mip_count;
			srv_desc.Texture1D.ResourceMinLODClamp = 0.0f;

			uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			uav_desc.Texture1D.MipSlice = desc->base_mip;

			rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
			rtv_desc.Texture1D.MipSlice = desc->base_mip;

			dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
			dsv_desc.Texture1D.MipSlice = desc->base_mip;
		}
		break;

		case OPAL_TEXTURE_VIEW_TYPE_2D:
		{
			if (samples == 1)
			{
				srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srv_desc.Texture2D.MostDetailedMip = desc->base_mip;
				srv_desc.Texture2D.MipLevels = desc->mip_count;
				srv_desc.Texture2D.PlaneSlice = 0;
				srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

				uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uav_desc.Texture2D.MipSlice = desc->base_mip;
				uav_desc.Texture2D.PlaneSlice = 0;

				rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				rtv_desc.Texture2D.MipSlice = desc->base_mip;
				rtv_desc.Texture2D.PlaneSlice = 0;

				dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				dsv_desc.Texture2D.MipSlice = desc->base_mip;
			}
			else
			{
				srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
				rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
				dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
			}
		}
		break;

		case OPAL_TEXTURE_VIEW_TYPE_2D_ARRAY:
		{
			if (samples == 1)
			{
				srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srv_desc.Texture2DArray.MostDetailedMip = desc->base_mip;
				srv_desc.Texture2DArray.MipLevels = desc->mip_count;
				srv_desc.Texture2DArray.FirstArraySlice = desc->base_layer;
				srv_desc.Texture2DArray.ArraySize = desc->layer_count;
				srv_desc.Texture2DArray.PlaneSlice = 0;
				srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;

				uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uav_desc.Texture2DArray.MipSlice = desc->base_mip;
				uav_desc.Texture2DArray.FirstArraySlice = desc->base_layer;
				uav_desc.Texture2DArray.ArraySize = desc->layer_count;
				uav_desc.Texture2DArray.PlaneSlice = 0;

				rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtv_desc.Texture2DArray.MipSlice = desc->base_mip;
				rtv_desc.Texture2DArray.FirstArraySlice = desc->base_layer;
				rtv_desc.Texture2DArray.ArraySize = desc->layer_count;
				rtv_desc.Texture2DArray.PlaneSlice = 0;

				dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				dsv_desc.Texture2DArray.MipSlice = desc->base_mip;
				dsv_desc.Texture2DArray.FirstArraySlice = desc->base_layer;
				dsv_desc.Texture2DArray.ArraySize = desc->layer_count;
			}
			else
			{
				srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
				srv_desc.Texture2DMSArray.FirstArraySlice = desc->base_layer;
				srv_desc.Texture2DMSArray.ArraySize = desc->layer_count;

				rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtv_desc.Texture2DMSArray.FirstArraySlice = desc->base_layer;
				rtv_desc.Texture2DMSArray.ArraySize = desc->layer_count;

				dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				dsv_desc.Texture2DMSArray.FirstArraySlice = desc->base_layer;
				dsv_desc.Texture2DMSArray.ArraySize = desc->layer_count;
			}
		}
		break;

		case OPAL_TEXTURE_VIEW_TYPE_CUBE:
		{
			srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srv_desc.TextureCube.MostDetailedMip = desc->base_mip;
			srv_desc.TextureCube.MipLevels = desc->mip_count;
			srv_desc.TextureCube.ResourceMinLODClamp = 0.0f;
		}
		break;

		case OPAL_TEXTURE_VIEW_TYPE_CUBE_ARRAY:
		{
			srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			srv_desc.TextureCubeArray.MostDetailedMip = desc->base_mip;
			srv_desc.TextureCubeArray.MipLevels = desc->mip_count;
			srv_desc.TextureCubeArray.First2DArrayFace = desc->base_layer;
			srv_desc.TextureCubeArray.NumCubes = desc->layer_count;
			srv_desc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
		}
		break;

		case OPAL_TEXTURE_VIEW_TYPE_3D:
		{
			srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			srv_desc.Texture3D.MostDetailedMip = desc->base_mip;
			srv_desc.Texture3D.MipLevels = desc->mip_count;
			srv_desc.Texture3D.ResourceMinLODClamp = 0.0f;

			uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			uav_desc.Texture3D.MipSlice = desc->base_mip;
			uav_desc.Texture3D.FirstWSlice = 0;
			uav_desc.Texture3D.WSize = (UINT)-1;

			rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			rtv_desc.Texture3D.MipSlice = desc->base_mip;
			rtv_desc.Texture3D.FirstWSlice = 0;
			rtv_desc.Texture3D.WSize = (UINT)-1;
		}
		break;
	}

	ID3D12Resource_AddRef(texture_ptr->texture);

	// create opal struct
	DirectX12_TextureView result = {0};
	result.texture = texture_ptr->texture;
	memcpy(&result.srv_desc, &srv_desc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	memcpy(&result.uav_desc, &uav_desc, sizeof(D3D12_UNORDERED_ACCESS_VIEW_DESC));
	memcpy(&result.rtv_desc, &rtv_desc, sizeof(D3D12_RENDER_TARGET_VIEW_DESC));
	memcpy(&result.dsv_desc, &dsv_desc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));

	*texture_view = (Opal_TextureView)opal_poolAddElement(&device_ptr->texture_views, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateSampler(Opal_Device this, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	assert(this);
	assert(desc);
	assert(sampler);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	// create opal struct
	DirectX12_Sampler result = {0};
	result.desc.Filter = directx12_helperToSamplerFilter(desc->min_filter, desc->mag_filter, desc->mip_filter);
	result.desc.AddressU = directx12_helperToSamplerAddressMode(desc->address_mode_u);
	result.desc.AddressV = directx12_helperToSamplerAddressMode(desc->address_mode_v);
	result.desc.AddressW = directx12_helperToSamplerAddressMode(desc->address_mode_w);
	result.desc.MaxAnisotropy = desc->max_anisotropy;
	result.desc.MinLOD = desc->min_lod;
	result.desc.MaxLOD = desc->max_lod;

	if (desc->compare_enable)
		result.desc.ComparisonFunc = directx12_helperToComparisonFunc(desc->compare_op);

	*sampler = (Opal_TextureView)opal_poolAddElement(&device_ptr->samplers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateAccelerationStructure(Opal_Device this, const Opal_AccelerationStructureDesc *desc, Opal_AccelerationStructure *acceleration_structure)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(acceleration_structure);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCreateCommandPool(Opal_Device this, Opal_Queue queue, Opal_CommandPool *command_pool)
{
	assert(this);
	assert(queue);
	assert(command_pool);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	ID3D12Device *d3d12_device = device_ptr->device;

	DirectX12_Queue *queue_ptr = (DirectX12_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queue);
	assert(queue_ptr);

	DirectX12_CommandPool result = {0};
	result.type = queue_ptr->type;

	uint32_t created_allocators = 0;
	for (uint32_t i = 0; i < D3D12_MAX_COMMAND_POOL_ALLOCATORS; ++i)
	{
		HRESULT hr = ID3D12Device_CreateCommandAllocator(d3d12_device, queue_ptr->type, &IID_ID3D12CommandAllocator, &result.allocators[i]);
		if (!SUCCEEDED(hr))
			break;

		created_allocators++;
	}

	if (created_allocators != D3D12_MAX_COMMAND_POOL_ALLOCATORS)
	{
		for (uint32_t i = 0; i < created_allocators; ++i)
			ID3D12CommandAllocator_Release(result.allocators[i]);

		return OPAL_DIRECTX12_ERROR;
	}

	*command_pool = (Opal_CommandPool)opal_poolAddElement(&device_ptr->command_pools, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateShader(Opal_Device this, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	assert(this);
	assert(desc);
	assert(shader);
	assert(desc->type == OPAL_SHADER_SOURCE_TYPE_DXIL_BINARY);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_Shader result = {0};
	result.data = malloc(desc->size);
	result.size = desc->size;

	memcpy(result.data, desc->data, desc->size);

	*shader = (Opal_CommandPool)opal_poolAddElement(&device_ptr->shaders, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateDescriptorHeap(Opal_Device this, const Opal_DescriptorHeapDesc *desc, Opal_DescriptorHeap *descriptor_heap)
{
	assert(this);
	assert(desc);
	assert(descriptor_heap);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	ID3D12Device *d3d12_device = device_ptr->device;

	DirectX12_DescriptorHeap result = {0};

	uint32_t num_fails = 0;

	if (desc->num_resource_descriptors > 0)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heap_info = {0};
		heap_info.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heap_info.NumDescriptors = desc->num_resource_descriptors;
		heap_info.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		HRESULT hr = ID3D12Device_CreateDescriptorHeap(d3d12_device, &heap_info, &IID_ID3D12DescriptorHeap, &result.resource_memory);
		if (!SUCCEEDED(hr))
			num_fails++;

		Opal_Result opal_result = opal_heapInitialize(&result.resource_heap, desc->num_resource_descriptors, desc->num_resource_descriptors);
		OPAL_UNUSED(opal_result);
	}

	if (desc->num_sampler_descriptors > 0)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heap_info = {0};
		heap_info.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		heap_info.NumDescriptors = desc->num_sampler_descriptors;
		heap_info.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		HRESULT hr = ID3D12Device_CreateDescriptorHeap(d3d12_device, &heap_info, &IID_ID3D12DescriptorHeap, &result.sampler_memory);
		if (!SUCCEEDED(hr))
			num_fails++;

		Opal_Result opal_result = opal_heapInitialize(&result.sampler_heap, desc->num_sampler_descriptors, desc->num_sampler_descriptors);
		OPAL_UNUSED(opal_result);
	}

	if (num_fails > 0)
	{
		directx12_destroyDescriptorHeap(device_ptr, &result);
		return OPAL_DIRECTX12_ERROR;
	}

	*descriptor_heap = (Opal_DescriptorHeap)opal_poolAddElement(&device_ptr->descriptor_heaps, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateDescriptorSetLayout(Opal_Device this, uint32_t num_entries, const Opal_DescriptorSetLayoutEntry *entries, Opal_DescriptorSetLayout *descriptor_set_layout)
{
	assert(this);
	assert(num_entries > 0);
	assert(entries);
	assert(descriptor_set_layout);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_DescriptorSetLayout result = {0};

	for (uint32_t i = 0; i < num_entries; ++i)
	{
		switch (entries[i].type)
		{
			case OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			{
				result.num_table_cbv_descriptors++;
			}
			break;

			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_1D:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_2D:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_2D_ARRAY:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_CUBE:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_CUBE_ARRAY:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_3D:
			case OPAL_DESCRIPTOR_TYPE_MULTISAMPLED_TEXTURE_2D:
			case OPAL_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE:
			{
				result.num_table_srv_descriptors++;
			}
			break;

			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_1D:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_2D:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_2D_ARRAY:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_3D:
			{
				result.num_table_uav_descriptors++;
			}
			break;

			case OPAL_DESCRIPTOR_TYPE_SAMPLER:
			case OPAL_DESCRIPTOR_TYPE_COMPARE_SAMPLER:
			{
				result.num_table_sampler_descriptors++;
			}
			break;

			case OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			{
				result.num_inline_descriptors++;
			}
			break;

			default: assert(0); break;
		}
	}

	uint32_t table_cbv_descriptor_offset = 0;
	uint32_t table_srv_descriptor_offset = table_cbv_descriptor_offset + result.num_table_cbv_descriptors;
	uint32_t table_uav_descriptor_offset = table_srv_descriptor_offset + result.num_table_srv_descriptors;
	uint32_t table_sampler_descriptor_offset = table_uav_descriptor_offset + result.num_table_uav_descriptors;

	uint32_t inline_descriptor_offset = table_sampler_descriptor_offset + result.num_table_sampler_descriptors;

	result.descriptors = (DirectX12_DescriptorInfo *)malloc(num_entries * sizeof(DirectX12_DescriptorInfo));

	DirectX12_DescriptorInfo *inline_descriptors = result.descriptors + inline_descriptor_offset;

	for (uint32_t i = 0; i < num_entries; ++i)
	{
		uint32_t index = 0;
		D3D12_DESCRIPTOR_RANGE_TYPE type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;

		switch (entries[i].type)
		{
			case OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			{
				index = table_cbv_descriptor_offset++;
				type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			}
			break;

			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_1D:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_2D:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_2D_ARRAY:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_CUBE:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_CUBE_ARRAY:
			case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_3D:
			case OPAL_DESCRIPTOR_TYPE_MULTISAMPLED_TEXTURE_2D:
			case OPAL_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE:
			{
				index = table_srv_descriptor_offset++;
				type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			}
			break;

			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_1D:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_2D:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_2D_ARRAY:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_3D:
			{
				index = table_uav_descriptor_offset++;
				type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			}
			break;

			case OPAL_DESCRIPTOR_TYPE_SAMPLER:
			case OPAL_DESCRIPTOR_TYPE_COMPARE_SAMPLER:
			{
				index = table_sampler_descriptor_offset++;
				type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			}
			break;

			case OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			{
				index = inline_descriptor_offset++;
				type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			}
			break;

			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC:
			{
				index = inline_descriptor_offset++;
				type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			}
			break;

			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			{
				index = inline_descriptor_offset++;
				type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			}
			break;

			default: assert(0); break;
		}

		result.descriptors[index].type = entries[i].type;
		result.descriptors[index].api_type = type;
		result.descriptors[index].binding = entries[i].binding;
	}

	if (result.num_inline_descriptors > 0)
		qsort(inline_descriptors, result.num_inline_descriptors, sizeof(DirectX12_DescriptorInfo), directx12_compareInlineDescriptors);

	*descriptor_set_layout = (Opal_DescriptorSetLayout)opal_poolAddElement(&device_ptr->descriptor_set_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_descriptor_set_layouts, const Opal_DescriptorSetLayout *descriptor_set_layouts, Opal_PipelineLayout *pipeline_layout)
{
	assert(this);
	assert(pipeline_layout);
	assert(num_descriptor_set_layouts == 0 || descriptor_set_layouts);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	ID3D12Device *d3d12_device = device_ptr->device;

	D3D12_ROOT_PARAMETER *parameters = NULL;
	D3D12_DESCRIPTOR_RANGE *ranges = NULL;
	uint32_t num_resource_descriptors = 0;
	uint32_t num_resource_tables = 0;

	uint32_t num_sampler_descriptors = 0;
	uint32_t num_sampler_tables = 0;

	uint32_t num_inline_descriptors = 0;

	uint32_t *src_table_offsets = NULL;

	if (num_descriptor_set_layouts > 0)
	{
		opal_bumpReset(&device_ptr->bump);

		uint32_t table_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(uint32_t) * num_descriptor_set_layouts * 2);

		src_table_offsets = (uint32_t *)(device_ptr->bump.data + table_offset);
		memset(src_table_offsets, UINT32_MAX, sizeof(uint32_t) * num_descriptor_set_layouts * 2);

		for (uint32_t i = 0; i < num_descriptor_set_layouts; ++i)
		{
			DirectX12_DescriptorSetLayout *descriptor_set_layout_ptr = (DirectX12_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_layouts[i]);
			assert(descriptor_set_layout_ptr);
			assert(descriptor_set_layout_ptr->descriptors);

			uint32_t num_set_resource_descriptors = 0;
			num_set_resource_descriptors += descriptor_set_layout_ptr->num_table_cbv_descriptors;
			num_set_resource_descriptors += descriptor_set_layout_ptr->num_table_srv_descriptors;
			num_set_resource_descriptors += descriptor_set_layout_ptr->num_table_uav_descriptors;

			uint32_t num_set_sampler_descriptors = descriptor_set_layout_ptr->num_table_sampler_descriptors;
			uint32_t num_set_inline_descriptors = descriptor_set_layout_ptr->num_inline_descriptors;

			if (num_set_resource_descriptors > 0)
			{
				src_table_offsets[i * 2 + 0] = i;
				num_resource_tables++;
			}

			if (num_set_sampler_descriptors > 0)
			{
				src_table_offsets[i * 2 + 1] = i;
				num_sampler_tables++;
			}

			num_resource_descriptors += num_set_resource_descriptors;
			num_sampler_descriptors += num_set_sampler_descriptors;
			num_inline_descriptors += num_set_inline_descriptors;
		}

		for (uint32_t i = 0; i < num_descriptor_set_layouts; ++i)
		{
			uint32_t index = i * 2 + 1;
			uint32_t offset = src_table_offsets[index];

			if (offset != UINT32_MAX)
				src_table_offsets[index] += num_resource_tables;
		}

		uint32_t parameters_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(D3D12_ROOT_PARAMETER) * (num_resource_tables + num_sampler_tables + num_inline_descriptors));
		uint32_t ranges_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(D3D12_DESCRIPTOR_RANGE) * (num_resource_descriptors + num_sampler_descriptors));

		parameters = (D3D12_ROOT_PARAMETER *)(device_ptr->bump.data + parameters_offset);
		ranges = (D3D12_DESCRIPTOR_RANGE *)(device_ptr->bump.data + ranges_offset);

		D3D12_ROOT_PARAMETER *current_parameter = parameters;
		for (uint32_t i = 0; i < num_descriptor_set_layouts; ++i)
		{
			DirectX12_DescriptorSetLayout *descriptor_set_layout_ptr = (DirectX12_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_layouts[i]);
			assert(descriptor_set_layout_ptr);
			assert(descriptor_set_layout_ptr->descriptors);

			uint32_t num_set_resource_descriptors = 0;
			num_set_resource_descriptors += descriptor_set_layout_ptr->num_table_cbv_descriptors;
			num_set_resource_descriptors += descriptor_set_layout_ptr->num_table_srv_descriptors;
			num_set_resource_descriptors += descriptor_set_layout_ptr->num_table_uav_descriptors;

			if (num_set_resource_descriptors == 0)
				continue;

			DirectX12_DescriptorInfo *current_descriptor = descriptor_set_layout_ptr->descriptors;

			uint32_t num_ranges = 0;

			for (uint32_t j = 0; j < descriptor_set_layout_ptr->num_table_cbv_descriptors; ++j)
			{
				assert(current_descriptor->api_type == D3D12_DESCRIPTOR_RANGE_TYPE_CBV);

				D3D12_DESCRIPTOR_RANGE *current_range = &ranges[num_ranges];
				current_range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				current_range->NumDescriptors = 1;
				current_range->BaseShaderRegister = current_descriptor->binding;
				current_range->RegisterSpace = i;
				current_range->OffsetInDescriptorsFromTableStart = num_ranges;

				num_ranges++;
				current_descriptor++;
			}

			for (uint32_t j = 0; j < descriptor_set_layout_ptr->num_table_srv_descriptors; ++j)
			{
				assert(current_descriptor->api_type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV);

				D3D12_DESCRIPTOR_RANGE *current_range = &ranges[num_ranges];
				current_range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				current_range->NumDescriptors = 1;
				current_range->BaseShaderRegister = current_descriptor->binding;
				current_range->RegisterSpace = i;
				current_range->OffsetInDescriptorsFromTableStart = num_ranges;

				num_ranges++;
				current_descriptor++;
			}

			for (uint32_t j = 0; j < descriptor_set_layout_ptr->num_table_uav_descriptors; ++j)
			{
				assert(current_descriptor->api_type == D3D12_DESCRIPTOR_RANGE_TYPE_UAV);

				D3D12_DESCRIPTOR_RANGE *current_range = &ranges[num_ranges];
				current_range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				current_range->NumDescriptors = 1;
				current_range->BaseShaderRegister = current_descriptor->binding;
				current_range->RegisterSpace = i;
				current_range->OffsetInDescriptorsFromTableStart = num_ranges;

				num_ranges++;
				current_descriptor++;
			}

			current_parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			current_parameter->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			current_parameter->DescriptorTable.NumDescriptorRanges = num_ranges;
			current_parameter->DescriptorTable.pDescriptorRanges = ranges;

			current_parameter++;
			ranges += num_ranges;
		}

		for (uint32_t i = 0; i < num_descriptor_set_layouts; ++i)
		{
			DirectX12_DescriptorSetLayout *descriptor_set_layout_ptr = (DirectX12_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_layouts[i]);
			assert(descriptor_set_layout_ptr);
			assert(descriptor_set_layout_ptr->descriptors);

			uint32_t num_set_sampler_descriptors = 0;
			num_set_sampler_descriptors += descriptor_set_layout_ptr->num_table_sampler_descriptors;

			if (num_set_sampler_descriptors == 0)
				continue;

			uint32_t offset = 0;
			offset += descriptor_set_layout_ptr->num_table_cbv_descriptors;
			offset += descriptor_set_layout_ptr->num_table_srv_descriptors;
			offset += descriptor_set_layout_ptr->num_table_uav_descriptors;

			DirectX12_DescriptorInfo *current_descriptor = descriptor_set_layout_ptr->descriptors + offset;

			uint32_t num_ranges = 0;

			for (uint32_t j = 0; j < descriptor_set_layout_ptr->num_table_sampler_descriptors; ++j)
			{
				assert(current_descriptor->api_type == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);

				D3D12_DESCRIPTOR_RANGE *current_range = &ranges[num_ranges];
				current_range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				current_range->NumDescriptors = 1;
				current_range->BaseShaderRegister = current_descriptor->binding;
				current_range->RegisterSpace = i;
				current_range->OffsetInDescriptorsFromTableStart = num_ranges;

				num_ranges++;
				current_descriptor++;
			}

			current_parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			current_parameter->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			current_parameter->DescriptorTable.NumDescriptorRanges = num_ranges;
			current_parameter->DescriptorTable.pDescriptorRanges = ranges;

			current_parameter++;
			ranges += num_ranges;
		}

		for (uint32_t i = 0; i < num_descriptor_set_layouts; ++i)
		{
			DirectX12_DescriptorSetLayout *descriptor_set_layout_ptr = (DirectX12_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_layouts[i]);
			assert(descriptor_set_layout_ptr);
			assert(descriptor_set_layout_ptr->descriptors);

			uint32_t num_set_inline_descriptors = descriptor_set_layout_ptr->num_inline_descriptors;

			if (num_set_inline_descriptors == 0)
				continue;

			uint32_t offset = 0;
			offset += descriptor_set_layout_ptr->num_table_cbv_descriptors;
			offset += descriptor_set_layout_ptr->num_table_srv_descriptors;
			offset += descriptor_set_layout_ptr->num_table_uav_descriptors;
			offset += descriptor_set_layout_ptr->num_table_sampler_descriptors;

			DirectX12_DescriptorInfo *current_descriptor = descriptor_set_layout_ptr->descriptors + offset;

			for (uint32_t j = 0; j < num_set_inline_descriptors; ++j)
			{
				current_parameter->ParameterType = current_descriptor->api_type;
				current_parameter->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				current_parameter->Descriptor.RegisterSpace = i;
				current_parameter->Descriptor.ShaderRegister = current_descriptor->binding;

				current_parameter++;
				current_descriptor++;
			}
		}
	}

	D3D12_ROOT_SIGNATURE_DESC layout_info = {0};
	layout_info.NumParameters = num_resource_tables + num_sampler_tables + num_inline_descriptors;
	layout_info.pParameters = parameters;
	layout_info.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob *blob = NULL;
	HRESULT hr = opal_d3d12SerializeRootSignature(&layout_info, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, NULL);

	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	const void *data = ID3D10Blob_GetBufferPointer(blob);
	SIZE_T size = ID3D10Blob_GetBufferSize(blob);

	ID3D12RootSignature *d3d12_root_signature = NULL;

	hr = ID3D12Device_CreateRootSignature(d3d12_device, 0, data, size, &IID_ID3D12RootSignature, &d3d12_root_signature);
	ID3D10Blob_Release(blob);

	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	DirectX12_PipelineLayout result = {0};
	result.root_signature = d3d12_root_signature;

	if (num_descriptor_set_layouts > 0)
	{
		result.num_layout_table_offsets = num_descriptor_set_layouts * 2;
		result.layout_table_offsets = (uint32_t *)malloc(sizeof(uint32_t) * result.num_layout_table_offsets);

		memcpy(result.layout_table_offsets, src_table_offsets, sizeof(uint32_t) * result.num_layout_table_offsets);
	}

	result.num_inline_descriptors = num_inline_descriptors;
	result.inline_offset = num_resource_tables + num_sampler_tables;

	*pipeline_layout = (Opal_DescriptorSetLayout)opal_poolAddElement(&device_ptr->pipeline_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	assert(this);
	assert(desc);
	assert(pipeline);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	ID3D12Device *d3d12_device = device_ptr->device;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_info = {0};

	// pipeline layout
	DirectX12_PipelineLayout *pipeline_layout_ptr = (DirectX12_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)desc->pipeline_layout);
	assert(pipeline_layout_ptr);

	pipeline_info.pRootSignature = pipeline_layout_ptr->root_signature;

	// shaders
	Opal_Shader shaders[5] =
	{
		desc->vertex_shader,
		desc->tessellation_control_shader,
		desc->tessellation_evaluation_shader,
		desc->geometry_shader,
		desc->fragment_shader
	};

	D3D12_SHADER_BYTECODE *shader_bytecodes[5] =
	{
		&pipeline_info.VS,
		&pipeline_info.HS,
		&pipeline_info.DS,
		&pipeline_info.GS,
		&pipeline_info.PS,
	};

	for (uint32_t i = 0; i < 5; ++i)
	{
		DirectX12_Shader *shader_ptr = (DirectX12_Shader *)opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)shaders[i]);
		if (shader_ptr == NULL)
			continue;

		shader_bytecodes[i]->pShaderBytecode = shader_ptr->data;
		shader_bytecodes[i]->BytecodeLength = shader_ptr->size;
	}


	// blend state
	for (uint32_t i = 0; i < desc->num_color_attachments; ++i)
	{
		D3D12_RENDER_TARGET_BLEND_DESC *blend_state = &pipeline_info.BlendState.RenderTarget[i];
		const Opal_BlendState *opal_blend_state = &desc->color_blend_states[i];

		blend_state->BlendEnable = opal_blend_state->enable;
		blend_state->SrcBlend = directx12_helperToBlendFactor(opal_blend_state->src_color);
		blend_state->DestBlend = directx12_helperToBlendFactor(opal_blend_state->dst_color);
		blend_state->BlendOp = directx12_helperToBlendOp(opal_blend_state->color_op);
		blend_state->SrcBlendAlpha = directx12_helperToBlendFactor(opal_blend_state->src_alpha);
		blend_state->DestBlendAlpha = directx12_helperToBlendFactor(opal_blend_state->dst_alpha);
		blend_state->BlendOpAlpha = directx12_helperToBlendOp(opal_blend_state->alpha_op);
		blend_state->RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	pipeline_info.SampleMask = 0xFFFFFFFF;

	// rasterizer state
	pipeline_info.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pipeline_info.RasterizerState.CullMode = directx12_helperToCullMode(desc->cull_mode);
	pipeline_info.RasterizerState.FrontCounterClockwise = (desc->front_face == OPAL_FRONT_FACE_COUNTER_CLOCKWISE);

	// multisample state
	pipeline_info.RasterizerState.MultisampleEnable = (desc->rasterization_samples != OPAL_SAMPLES_1);
	pipeline_info.SampleDesc.Count = directx12_helperToSampleCount(desc->rasterization_samples);

	// depthstencil state
	pipeline_info.DepthStencilState.DepthEnable = desc->depth_enable;
	pipeline_info.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	pipeline_info.DepthStencilState.DepthFunc = directx12_helperToComparisonFunc(desc->depth_compare_op);
	pipeline_info.DepthStencilState.StencilEnable = desc->stencil_enable;
	pipeline_info.DepthStencilState.StencilReadMask = (UINT8)desc->stencil_read_mask;
	pipeline_info.DepthStencilState.StencilWriteMask = (UINT8)desc->stencil_write_mask;
	pipeline_info.DepthStencilState.FrontFace.StencilFailOp = directx12_helperToStencilOp(desc->stencil_front.fail_op);
	pipeline_info.DepthStencilState.FrontFace.StencilDepthFailOp = directx12_helperToStencilOp(desc->stencil_front.depth_fail_op);
	pipeline_info.DepthStencilState.FrontFace.StencilPassOp = directx12_helperToStencilOp(desc->stencil_front.pass_op);
	pipeline_info.DepthStencilState.FrontFace.StencilFunc = directx12_helperToComparisonFunc(desc->stencil_front.compare_op);
	pipeline_info.DepthStencilState.BackFace.StencilFailOp = directx12_helperToStencilOp(desc->stencil_back.fail_op);
	pipeline_info.DepthStencilState.BackFace.StencilDepthFailOp = directx12_helperToStencilOp(desc->stencil_back.depth_fail_op);
	pipeline_info.DepthStencilState.BackFace.StencilPassOp = directx12_helperToStencilOp(desc->stencil_back.pass_op);
	pipeline_info.DepthStencilState.BackFace.StencilFunc = directx12_helperToComparisonFunc(desc->stencil_back.compare_op);
	
	// vertex input state
	uint32_t num_vertex_attributes = 0;
	for (uint32_t i = 0; i < desc->num_vertex_streams; ++i)
		num_vertex_attributes += desc->vertex_streams[i].num_vertex_attributes;

	opal_bumpReset(&device_ptr->bump);
	uint32_t vertex_attributes_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(D3D12_INPUT_ELEMENT_DESC) * num_vertex_attributes);

	D3D12_INPUT_ELEMENT_DESC *vertex_attributes = (D3D12_INPUT_ELEMENT_DESC *)(device_ptr->bump.data + vertex_attributes_offset);

	uint32_t num_attributes = 0;
	for (uint32_t i = 0; i < desc->num_vertex_streams; ++i)
	{
		const Opal_VertexStream *vertex_stream = &desc->vertex_streams[i];
		for (uint32_t j = 0; j < vertex_stream->num_vertex_attributes; ++j)
		{
			const Opal_VertexAttribute *vertex_attribute = &vertex_stream->attributes[j];

			vertex_attributes[num_attributes].SemanticName = "LOCATION";
			vertex_attributes[num_attributes].SemanticIndex = num_attributes;
			vertex_attributes[num_attributes].Format = directx12_helperToDXGIVertexFormat(vertex_attribute->format);
			vertex_attributes[num_attributes].InputSlot = i;
			vertex_attributes[num_attributes].AlignedByteOffset = vertex_attribute->offset;
			vertex_attributes[num_attributes].InputSlotClass = directx12_helperToInputSlotClass(vertex_stream->rate);
			vertex_attributes[num_attributes].InstanceDataStepRate = directx12_helperToInstanceDataStepRate(vertex_stream->rate);
			num_attributes++;
		}
	}

	pipeline_info.InputLayout.NumElements = num_attributes;
	pipeline_info.InputLayout.pInputElementDescs = vertex_attributes;

	// primitive state
	pipeline_info.PrimitiveTopologyType = directx12_helperToPrimitiveTopologyType(desc->primitive_type);

	if (desc->primitive_type == OPAL_PRIMITIVE_TYPE_LINE_STRIP || desc->primitive_type == OPAL_PRIMITIVE_TYPE_TRIANGLE_STRIP)
		pipeline_info.IBStripCutValue = directx12_helperToStripCutValue(desc->strip_index_format);

	// render target state
	pipeline_info.NumRenderTargets = desc->num_color_attachments;

	for (uint32_t i = 0; i < desc->num_color_attachments; ++i)
		pipeline_info.RTVFormats[i] = directx12_helperToDXGITextureFormat(desc->color_attachment_formats[i]);

	if (desc->depth_stencil_attachment_format)
		pipeline_info.DSVFormat = directx12_helperToDXGITextureFormat(*desc->depth_stencil_attachment_format);

	ID3D12PipelineState *d3d12_pipeline_state = NULL;
	HRESULT hr = ID3D12Device_CreateGraphicsPipelineState(d3d12_device, &pipeline_info, &IID_ID3D12PipelineState, &d3d12_pipeline_state);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	ID3D12RootSignature_AddRef(pipeline_layout_ptr->root_signature);

	DirectX12_Pipeline result = {0};
	result.pipeline_state = d3d12_pipeline_state;
	result.root_signature = pipeline_layout_ptr->root_signature;
	result.primitive_topology = directx12_helperToPrimitiveTopology(desc->primitive_type);

	*pipeline = (Opal_DescriptorSetLayout)opal_poolAddElement(&device_ptr->pipelines, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateMeshletPipeline(Opal_Device this, const Opal_MeshletPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCreateComputePipeline(Opal_Device this, const Opal_ComputePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	assert(this);
	assert(desc);
	assert(pipeline);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	ID3D12Device *d3d12_device = device_ptr->device;

	DirectX12_PipelineLayout *pipeline_layout_ptr = (DirectX12_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)desc->pipeline_layout);
	assert(pipeline_layout_ptr);

	DirectX12_Shader *shader_ptr = (DirectX12_Shader *)opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)desc->compute_shader);
	assert(shader_ptr);

	D3D12_COMPUTE_PIPELINE_STATE_DESC pipeline_info = {0};
	pipeline_info.pRootSignature = pipeline_layout_ptr->root_signature;
	pipeline_info.CS.pShaderBytecode = shader_ptr->data;
	pipeline_info.CS.BytecodeLength = shader_ptr->size;

	ID3D12PipelineState *d3d12_pipeline_state = NULL;
	HRESULT hr = ID3D12Device_CreateComputePipelineState(d3d12_device, &pipeline_info, &IID_ID3D12PipelineState, &d3d12_pipeline_state);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	ID3D12RootSignature_AddRef(pipeline_layout_ptr->root_signature);

	DirectX12_Pipeline result = {0};
	result.pipeline_state = d3d12_pipeline_state;
	result.root_signature = pipeline_layout_ptr->root_signature;
	result.primitive_topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;

	*pipeline = (Opal_DescriptorSetLayout)opal_poolAddElement(&device_ptr->pipelines, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateRaytracePipeline(Opal_Device this, const Opal_RaytracePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCreateSwapchain(Opal_Device this, const Opal_SwapchainDesc *desc, Opal_Swapchain *swapchain)
{
	assert(this);
	assert(desc);
	assert(desc->surface);
	assert(swapchain);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_Instance *instance_ptr = device_ptr->instance;

	IDXGIFactory2 *d3d12_factory = instance_ptr->factory;

	// surface
	DirectX12_Surface *surface_ptr = (DirectX12_Surface *)opal_poolGetElement(&instance_ptr->surfaces, (Opal_PoolHandle)desc->surface);
	assert(surface_ptr);

	// surface capabilities
	uint32_t num_textures = (desc->mode == OPAL_PRESENT_MODE_MAILBOX) ? 3 : 2;

	// present queue
	Opal_Queue *queues = device_ptr->queue_handles[OPAL_DEVICE_ENGINE_TYPE_MAIN];
	assert(device_ptr->device_engines_info.queue_counts[OPAL_DEVICE_ENGINE_TYPE_MAIN] != 0);

	DirectX12_Queue *queue_ptr = (DirectX12_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queues[0]);
	assert(queue_ptr);

	// present mode
	UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	if (desc->mode == OPAL_PRESENT_MODE_IMMEDIATE)
		flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	// surface format
	DXGI_FORMAT format = directx12_helperToDXGITextureFormat(desc->format.texture_format);
	if (format == DXGI_FORMAT_UNKNOWN)
		return OPAL_SWAPCHAIN_FORMAT_NOT_SUPPORTED;

	DXGI_SWAP_CHAIN_DESC1 swapchain_info = {0};
	swapchain_info.Format = format;
	swapchain_info.Stereo = FALSE;
	swapchain_info.SampleDesc.Count = 1;
	swapchain_info.SampleDesc.Quality = 0;
	swapchain_info.BufferUsage = directx12_helperToDXGIUsage(desc->usage);
	swapchain_info.BufferCount = num_textures;
	swapchain_info.Scaling = DXGI_SCALING_STRETCH;
	swapchain_info.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchain_info.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapchain_info.Flags = flags;

	IDXGISwapChain1 *d3d12_swapchain1 = NULL;
	HRESULT hr = IDXGIFactory2_CreateSwapChainForHwnd(d3d12_factory, (IUnknown *)queue_ptr->queue, surface_ptr->handle, &swapchain_info, NULL, NULL, &d3d12_swapchain1);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	IDXGISwapChain3 *d3d12_swapchain3 = NULL;
	hr = IDXGISwapChain1_QueryInterface(d3d12_swapchain1, &IID_IDXGISwapChain3, &d3d12_swapchain3);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	IDXGISwapChain1_Release(d3d12_swapchain1);

	DXGI_COLOR_SPACE_TYPE color_space = directx12_helperToDXGIColorSpace(desc->format.color_space);
	UINT color_space_support = 0;

	hr = IDXGISwapChain3_CheckColorSpaceSupport(d3d12_swapchain3, color_space, &color_space_support);
	if (!SUCCEEDED(hr))
	{
		IDXGISwapChain3_Release(d3d12_swapchain3);
		return OPAL_DIRECTX12_ERROR;
	}

	if (color_space_support == 0)
	{
		IDXGISwapChain3_Release(d3d12_swapchain3);
		return OPAL_SWAPCHAIN_COLOR_SPACE_NOT_SUPPORTED;
	}

	hr = IDXGISwapChain3_SetColorSpace1(d3d12_swapchain3, color_space);
	if (!SUCCEEDED(hr))
	{
		IDXGISwapChain3_Release(d3d12_swapchain3);
		return OPAL_DIRECTX12_ERROR;
	}

	Opal_TextureView *texture_views = (Opal_TextureView *)malloc(sizeof(Opal_TextureView) * num_textures);
	memset(texture_views, OPAL_NULL_HANDLE, sizeof(Opal_PoolHandle) * num_textures);

	BOOL success = TRUE;
	for (uint32_t i = 0; i < num_textures; ++i)
	{
		DirectX12_TextureView result = {0};

		hr = IDXGISwapChain3_GetBuffer(d3d12_swapchain3, i, &IID_ID3D12Resource, &result.texture);
		if (!SUCCEEDED(hr))
		{
			success = FALSE;
			break;
		}

		result.srv_desc.Format = format;
		result.srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		result.srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		result.srv_desc.Texture2D.MipLevels = 1;

		result.uav_desc.Format = format;
		result.uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		result.rtv_desc.Format = format;
		result.rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		result.dsv_desc.Format = format;
		result.dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
		result.dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		texture_views[i] = (Opal_TextureView)opal_poolAddElement(&device_ptr->texture_views, &result);
	}

	if (success != TRUE)
	{
		for (uint32_t i = 0; i < num_textures; ++i)
			directx12_deviceDestroyTextureView(this, texture_views[i]);

		free(texture_views);
		IDXGISwapChain3_Release(d3d12_swapchain3);
		return OPAL_DIRECTX12_ERROR;
	}

	// create opal struct
	DirectX12_Swapchain result = {0};
	result.swapchain = d3d12_swapchain3;
	result.texture_views = texture_views;
	result.current_index = IDXGISwapChain3_GetCurrentBackBufferIndex(d3d12_swapchain3);
	result.num_textures = num_textures;

	if (desc->mode == OPAL_PRESENT_MODE_IMMEDIATE)
		result.present_flags = DXGI_PRESENT_ALLOW_TEARING;

	*swapchain = (Opal_Buffer)opal_poolAddElement(&device_ptr->swapchains, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroySemaphore(Opal_Device this, Opal_Semaphore semaphore)
{
	assert(this);
	assert(semaphore);

	Opal_PoolHandle handle = (Opal_PoolHandle)semaphore;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_Semaphore *semaphore_ptr = (DirectX12_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, handle);
	assert(semaphore_ptr);

	opal_poolRemoveElement(&device_ptr->semaphores, handle);

	directx12_destroySemaphore(device_ptr, semaphore_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer)
{
	assert(this);
	assert(buffer);

	Opal_PoolHandle handle = (Opal_PoolHandle)buffer;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, handle);
	assert(buffer_ptr);

	opal_poolRemoveElement(&device_ptr->buffers, handle);

	directx12_destroyBuffer(device_ptr, buffer_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroyTexture(Opal_Device this, Opal_Texture texture)
{
	assert(this);
	assert(texture);

	Opal_PoolHandle handle = (Opal_PoolHandle)texture;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_Texture *texture_ptr = (DirectX12_Texture *)opal_poolGetElement(&device_ptr->textures, handle);
	assert(texture_ptr);

	opal_poolRemoveElement(&device_ptr->textures, handle);

	directx12_destroyTexture(device_ptr, texture_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	assert(this);
	assert(texture_view);

	Opal_PoolHandle handle = (Opal_PoolHandle)texture_view;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_TextureView *texture_view_ptr = (DirectX12_TextureView *)opal_poolGetElement(&device_ptr->texture_views, handle);
	assert(texture_view_ptr);

	opal_poolRemoveElement(&device_ptr->texture_views, handle);

	directx12_destroyTextureView(device_ptr, texture_view_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroySampler(Opal_Device this, Opal_Sampler sampler)
{
	assert(this);
	assert(sampler);

	Opal_PoolHandle handle = (Opal_PoolHandle)sampler;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_Sampler *sampler_ptr = (DirectX12_Sampler *)opal_poolGetElement(&device_ptr->samplers, handle);
	assert(sampler_ptr);

	opal_poolRemoveElement(&device_ptr->samplers, handle);

	directx12_destroySampler(device_ptr, sampler_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroyAccelerationStructure(Opal_Device this, Opal_AccelerationStructure acceleration_structure)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(acceleration_structure);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceDestroyCommandPool(Opal_Device this, Opal_CommandPool command_pool)
{
	assert(this);
	assert(command_pool);

	Opal_PoolHandle handle = (Opal_PoolHandle)command_pool;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_CommandPool *command_pool_ptr = (DirectX12_CommandPool *)opal_poolGetElement(&device_ptr->command_pools, handle);
	assert(command_pool_ptr);

	// TODO: fix memory leak caused by orphaned DirectX12_CommandBuffer instances

	opal_poolRemoveElement(&device_ptr->command_pools, handle);

	directx12_destroyCommandPool(device_ptr, command_pool_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	assert(this);
	assert(shader);

	Opal_PoolHandle handle = (Opal_PoolHandle)shader;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_Shader *shader_ptr = (DirectX12_Shader *)opal_poolGetElement(&device_ptr->shaders, handle);
	assert(shader_ptr);

	opal_poolRemoveElement(&device_ptr->shaders, handle);

	directx12_destroyShader(device_ptr, shader_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroyDescriptorHeap(Opal_Device this, Opal_DescriptorHeap descriptor_heap)
{
	assert(this);
	assert(descriptor_heap);

	Opal_PoolHandle handle = (Opal_PoolHandle)descriptor_heap;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_DescriptorHeap *descriptor_heap_ptr = (DirectX12_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, handle);
	assert(descriptor_heap_ptr);

	opal_poolRemoveElement(&device_ptr->descriptor_heaps, handle);

	directx12_destroyDescriptorHeap(device_ptr, descriptor_heap_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroyDescriptorSetLayout(Opal_Device this, Opal_DescriptorSetLayout descriptor_set_layout)
{
	assert(this);
	assert(descriptor_set_layout);

	Opal_PoolHandle handle = (Opal_PoolHandle)descriptor_set_layout;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_DescriptorSetLayout *descriptor_set_layout_ptr = (DirectX12_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, handle);
	assert(descriptor_set_layout_ptr);

	opal_poolRemoveElement(&device_ptr->descriptor_set_layouts, handle);

	directx12_destroyDescriptorSetLayout(device_ptr, descriptor_set_layout_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	assert(this);
	assert(pipeline_layout);

	Opal_PoolHandle handle = (Opal_PoolHandle)pipeline_layout;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_PipelineLayout *pipeline_layout_ptr = (DirectX12_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, handle);
	assert(pipeline_layout_ptr);

	opal_poolRemoveElement(&device_ptr->pipeline_layouts, handle);

	directx12_destroyPipelineLayout(device_ptr, pipeline_layout_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroyPipeline(Opal_Device this, Opal_Pipeline pipeline)
{
	assert(this);
	assert(pipeline);

	Opal_PoolHandle handle = (Opal_PoolHandle)pipeline;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_Pipeline *pipeline_ptr = (DirectX12_Pipeline *)opal_poolGetElement(&device_ptr->pipelines, handle);
	assert(pipeline_ptr);

	opal_poolRemoveElement(&device_ptr->pipelines, handle);

	directx12_destroyPipeline(device_ptr, pipeline_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroySwapchain(Opal_Device this, Opal_Swapchain swapchain)
{
	assert(this);
	assert(swapchain);

	Opal_PoolHandle handle = (Opal_PoolHandle)swapchain;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_Swapchain *swapchain_ptr = (DirectX12_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, handle);
	assert(swapchain_ptr);

	for (uint32_t i = 0; i < swapchain_ptr->num_textures; ++i)
		directx12_deviceDestroyTextureView(this, swapchain_ptr->texture_views[i]);

	opal_poolRemoveElement(&device_ptr->swapchains, handle);

	directx12_destroySwapchain(device_ptr, swapchain_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroy(Opal_Device this)
{
	assert(this);

	DirectX12_Device *ptr = (DirectX12_Device *)this;

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->swapchains);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_Swapchain *swapchain_ptr = (DirectX12_Swapchain *)opal_poolGetElementByIndex(&ptr->swapchains, head);
			directx12_destroySwapchain(ptr, swapchain_ptr);

			head = opal_poolGetNextIndex(&ptr->swapchains, head);
		}

		opal_poolShutdown(&ptr->swapchains);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->pipelines);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_Pipeline *pipeline_ptr = (DirectX12_Pipeline *)opal_poolGetElementByIndex(&ptr->pipelines, head);
			directx12_destroyPipeline(ptr, pipeline_ptr);

			head = opal_poolGetNextIndex(&ptr->pipelines, head);
		}

		opal_poolShutdown(&ptr->pipelines);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->pipeline_layouts);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_PipelineLayout *pipeline_layout_ptr = (DirectX12_PipelineLayout *)opal_poolGetElementByIndex(&ptr->pipeline_layouts, head);
			directx12_destroyPipelineLayout(ptr, pipeline_layout_ptr);

			head = opal_poolGetNextIndex(&ptr->pipeline_layouts, head);
		}

		opal_poolShutdown(&ptr->pipeline_layouts);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->descriptor_set_layouts);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_DescriptorSetLayout *descriptor_set_layout_ptr = (DirectX12_DescriptorSetLayout *)opal_poolGetElementByIndex(&ptr->descriptor_set_layouts, head);
			directx12_destroyDescriptorSetLayout(ptr, descriptor_set_layout_ptr);

			head = opal_poolGetNextIndex(&ptr->descriptor_set_layouts, head);
		}

		opal_poolShutdown(&ptr->descriptor_set_layouts);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->descriptor_heaps);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_DescriptorHeap *descriptor_heap_ptr = (DirectX12_DescriptorHeap *)opal_poolGetElementByIndex(&ptr->descriptor_heaps, head);
			directx12_destroyDescriptorHeap(ptr, descriptor_heap_ptr);

			head = opal_poolGetNextIndex(&ptr->descriptor_heaps, head);
		}

		opal_poolShutdown(&ptr->descriptor_heaps);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->shaders);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_Shader *shader_ptr = (DirectX12_Shader *)opal_poolGetElementByIndex(&ptr->shaders, head);
			directx12_destroyShader(ptr, shader_ptr);

			head = opal_poolGetNextIndex(&ptr->shaders, head);
		}

		opal_poolShutdown(&ptr->shaders);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->command_buffers);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElementByIndex(&ptr->command_buffers, head);
			directx12_destroyCommandBuffer(ptr, command_buffer_ptr);

			head = opal_poolGetNextIndex(&ptr->command_buffers, head);
		}

		opal_poolShutdown(&ptr->command_buffers);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->command_pools);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_CommandPool *command_pool_ptr = (DirectX12_CommandPool *)opal_poolGetElementByIndex(&ptr->command_pools, head);
			directx12_destroyCommandPool(ptr, command_pool_ptr);

			head = opal_poolGetNextIndex(&ptr->command_pools, head);
		}

		opal_poolShutdown(&ptr->command_pools);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->samplers);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_Sampler *sampler_ptr = (DirectX12_Sampler *)opal_poolGetElementByIndex(&ptr->samplers, head);
			directx12_destroySampler(ptr, sampler_ptr);

			head = opal_poolGetNextIndex(&ptr->samplers, head);
		}

		opal_poolShutdown(&ptr->samplers);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->texture_views);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_TextureView *texture_view_ptr = (DirectX12_TextureView *)opal_poolGetElementByIndex(&ptr->texture_views, head);
			directx12_destroyTextureView(ptr, texture_view_ptr);

			head = opal_poolGetNextIndex(&ptr->texture_views, head);
		}

		opal_poolShutdown(&ptr->texture_views);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->textures);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_Texture *texture_ptr = (DirectX12_Texture *)opal_poolGetElementByIndex(&ptr->textures, head);
			directx12_destroyTexture(ptr, texture_ptr);

			head = opal_poolGetNextIndex(&ptr->textures, head);
		}

		opal_poolShutdown(&ptr->textures);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->buffers);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElementByIndex(&ptr->buffers, head);
			directx12_destroyBuffer(ptr, buffer_ptr);

			head = opal_poolGetNextIndex(&ptr->buffers, head);
		}

		opal_poolShutdown(&ptr->buffers);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->semaphores);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_Semaphore *semaphore_ptr = (DirectX12_Semaphore *)opal_poolGetElementByIndex(&ptr->semaphores, head);
			directx12_destroySemaphore(ptr, semaphore_ptr);

			head = opal_poolGetNextIndex(&ptr->semaphores, head);
		}

		opal_poolShutdown(&ptr->semaphores);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->queues);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_Queue *queue_ptr = (DirectX12_Queue *)opal_poolGetElementByIndex(&ptr->queues, head);
			directx12_destroyQueue(ptr, queue_ptr);

			head = opal_poolGetNextIndex(&ptr->queues, head);
		}

		opal_poolShutdown(&ptr->queues);
	}

	opal_poolShutdown(&ptr->descriptor_sets);

	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
		free(ptr->queue_handles[i]);

	opal_bumpShutdown(&ptr->bump);

	directx12_destroyFramebufferDescriptorHeap(ptr, &ptr->framebuffer_descriptor_heap);

	Opal_Result result = directx12_allocatorShutdown(ptr);
	assert(result == OPAL_SUCCESS);

	OPAL_UNUSED(result);

	IDXGIAdapter1_Release(ptr->adapter);
	ID3D12Device_Release(ptr->device);

	free(ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceBuildShaderBindingTable(Opal_Device this, const Opal_ShaderBindingTableBuildDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceBuildAccelerationStructureInstanceBuffer(Opal_Device this, const Opal_AccelerationStructureInstanceBufferBuildDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceAllocateCommandBuffer(Opal_Device this, Opal_CommandPool command_pool, Opal_CommandBuffer *command_buffer)
{
	assert(this);
	assert(command_pool);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	ID3D12Device *d3d12_device = device_ptr->device;

	DirectX12_CommandPool *command_pool_ptr = (DirectX12_CommandPool *)opal_poolGetElement(&device_ptr->command_pools, (Opal_PoolHandle)command_pool);
	assert(command_pool_ptr);

	ID3D12CommandAllocator *d3d12_command_allocator = NULL;
	uint32_t index = 0;
	for (uint32_t i = 0; i < D3D12_MAX_COMMAND_POOL_ALLOCATORS; ++i)
	{
		if (command_pool_ptr->usages[i] != 0)
			continue;

		d3d12_command_allocator = command_pool_ptr->allocators[i];
		index = i;
		break;
	}

	if (d3d12_command_allocator == NULL)
		return OPAL_NO_POOL_MEMORY;

	ID3D12GraphicsCommandList4 *d3d12_command_list = NULL;
	HRESULT hr = ID3D12Device_CreateCommandList(d3d12_device, 0, command_pool_ptr->type, d3d12_command_allocator, NULL, &IID_ID3D12GraphicsCommandList4, &d3d12_command_list);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	command_pool_ptr->usages[index] = 1;

	DirectX12_CommandBuffer result = {0};
	result.list = d3d12_command_list;
	result.pool = command_pool;
	result.index = index;
	result.recording = 1;

	// TODO: add handle to DirectX12_CommandPool instance

	*command_buffer = (Opal_CommandBuffer)opal_poolAddElement(&device_ptr->command_buffers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceFreeCommandBuffer(Opal_Device this, Opal_CommandPool command_pool, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_pool);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandPool *command_pool_ptr = (DirectX12_CommandPool *)opal_poolGetElement(&device_ptr->command_pools, (Opal_PoolHandle)command_pool);
	assert(command_pool_ptr);

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	uint32_t index = command_buffer_ptr->index;
	assert(index < D3D12_MAX_COMMAND_POOL_ALLOCATORS);
	command_pool_ptr->usages[index] = 0;

	// TODO: remove handle from Vulkan_CommandPool instance

	directx12_destroyCommandBuffer(device_ptr, command_buffer_ptr);

	opal_poolRemoveElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceResetCommandPool(Opal_Device this, Opal_CommandPool command_pool)
{
	assert(this);
	assert(command_pool);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandPool *command_pool_ptr = (DirectX12_CommandPool *)opal_poolGetElement(&device_ptr->command_pools, (Opal_PoolHandle)command_pool);
	assert(command_pool_ptr);

	for (uint32_t i = 0; i < D3D12_MAX_COMMAND_POOL_ALLOCATORS; ++i)
	{
		command_pool_ptr->usages[i] = 0;

		HRESULT hr = ID3D12CommandAllocator_Reset(command_pool_ptr->allocators[i]);
		if (!SUCCEEDED(hr))
			return OPAL_DIRECTX12_ERROR;
	}

	// FIXME: reset all related command allocators
	// TODO: fix memory leak caused by orphaned Vulkan_CommandBuffer instances

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceResetCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	DirectX12_CommandPool *command_pool_ptr = (DirectX12_CommandPool *)opal_poolGetElement(&device_ptr->command_pools, (Opal_PoolHandle)command_buffer_ptr->pool);
	assert(command_pool_ptr);

	uint32_t index = command_buffer_ptr->index;
	assert(index < D3D12_MAX_COMMAND_POOL_ALLOCATORS);

	if (command_buffer_ptr->recording)
	{
		HRESULT hr = ID3D12GraphicsCommandList4_Close(command_buffer_ptr->list);
		if (!SUCCEEDED(hr))
			return OPAL_DIRECTX12_ERROR;
	}

	// FIXME: reset command allocator
	HRESULT hr = ID3D12GraphicsCommandList4_Reset(command_buffer_ptr->list, command_pool_ptr->allocators[index], NULL);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	command_buffer_ptr->recording = 1;
	command_buffer_ptr->root_signature = NULL;
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceAllocateDescriptorSet(Opal_Device this, const Opal_DescriptorSetAllocationDesc *desc, Opal_DescriptorSet *descriptor_set)
{
	assert(this);
	assert(desc);
	assert(descriptor_set);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_DescriptorSetLayout *descriptor_set_layout_ptr = (DirectX12_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)desc->layout);
	assert(descriptor_set_layout_ptr);

	uint32_t num_resource_descriptors = 0;
	num_resource_descriptors += descriptor_set_layout_ptr->num_table_cbv_descriptors;
	num_resource_descriptors += descriptor_set_layout_ptr->num_table_srv_descriptors;
	num_resource_descriptors += descriptor_set_layout_ptr->num_table_uav_descriptors;

	uint32_t num_sampler_descriptors = descriptor_set_layout_ptr->num_table_sampler_descriptors;
	uint32_t num_inline_descriptors = descriptor_set_layout_ptr->num_inline_descriptors;

	DirectX12_DescriptorSet result = {0};

	DirectX12_DescriptorHeap *heap_ptr = (DirectX12_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)desc->heap);
	assert(heap_ptr);

	if (num_resource_descriptors > 0)
	{
		Opal_Result opal_result = opal_heapAlloc(&heap_ptr->resource_heap, num_resource_descriptors, &result.resource_allocation);
		if (opal_result != OPAL_SUCCESS)
			return opal_result;

		UINT stride = ID3D12Device_GetDescriptorHandleIncrementSize(device_ptr->device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart(heap_ptr->resource_memory, &result.resource_handle);
		result.resource_handle.ptr += stride * result.resource_allocation.offset;
	}

	if (num_sampler_descriptors > 0)
	{
		Opal_Result opal_result = opal_heapAlloc(&heap_ptr->sampler_heap, num_sampler_descriptors, &result.sampler_allocation);
		if (opal_result != OPAL_SUCCESS)
			return opal_result;

		UINT stride = ID3D12Device_GetDescriptorHandleIncrementSize(device_ptr->device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart(heap_ptr->sampler_memory, &result.sampler_handle);
		result.sampler_handle.ptr += stride * result.sampler_allocation.offset;
	}


	if (num_inline_descriptors > 0)
	{
		result.num_inline_descriptors = num_inline_descriptors;
		result.inline_descriptors = (Opal_DescriptorSetEntry *)malloc(sizeof(Opal_DescriptorSetEntry) * num_inline_descriptors);
	}

	result.layout = desc->layout;
	result.heap = desc->heap;

	*descriptor_set = (Opal_DescriptorSet)opal_poolAddElement(&device_ptr->descriptor_sets, &result);

	if (desc->num_entries == 0)
		return OPAL_SUCCESS;

	assert(desc->entries);

	Opal_Result opal_result = directx12_deviceUpdateDescriptorSet(this, *descriptor_set, desc->num_entries, desc->entries);
	if (opal_result != OPAL_SUCCESS)
	{
		directx12_deviceFreeDescriptorSet(this, *descriptor_set);
		*descriptor_set = OPAL_NULL_HANDLE;
	}

	return opal_result;
}

static Opal_Result directx12_deviceFreeDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set)
{
	assert(this);
	assert(descriptor_set);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_DescriptorSet *descriptor_set_ptr = (DirectX12_DescriptorSet *)opal_poolGetElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	assert(descriptor_set_ptr);

	DirectX12_DescriptorHeap *heap_ptr = (DirectX12_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)descriptor_set_ptr->heap);
	assert(heap_ptr);
	
	if (descriptor_set_ptr->resource_handle.ptr > 0)
	{
		Opal_Result opal_result = opal_heapFree(&heap_ptr->resource_heap, descriptor_set_ptr->resource_allocation);
		assert(opal_result == OPAL_SUCCESS);
	}

	if (descriptor_set_ptr->sampler_handle.ptr > 0)
	{
		Opal_Result opal_result = opal_heapFree(&heap_ptr->sampler_heap, descriptor_set_ptr->sampler_allocation);
		assert(opal_result == OPAL_SUCCESS);
	}

	if (descriptor_set_ptr->num_inline_descriptors > 0)
	{
		free(descriptor_set_ptr->inline_descriptors);
		descriptor_set_ptr->inline_descriptors = NULL;
	}

	opal_poolRemoveElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr)
{
	assert(this);
	assert(buffer);
	assert(ptr);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer);
	assert(buffer_ptr);

	HRESULT hr = ID3D12Resource_Map(buffer_ptr->buffer, 0, NULL, ptr);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer)
{
	assert(this);
	assert(buffer);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer);
	assert(buffer_ptr);

	ID3D12Resource_Unmap(buffer_ptr->buffer, 0, NULL);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceWriteBuffer(Opal_Device this, Opal_Buffer buffer, uint64_t offset, const void *data, uint64_t size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(offset);
	OPAL_UNUSED(data);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceUpdateDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set, uint32_t num_entries, const Opal_DescriptorSetEntry *entries)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(descriptor_set);
	OPAL_UNUSED(num_entries);
	OPAL_UNUSED(entries);

	assert(this);
	assert(descriptor_set);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_DescriptorSet *descriptor_set_ptr = (DirectX12_DescriptorSet *)opal_poolGetElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	assert(descriptor_set_ptr);

	DirectX12_DescriptorSetLayout *descriptor_set_layout_ptr = (DirectX12_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_ptr->layout);
	assert(descriptor_set_layout_ptr);

	DirectX12_DescriptorHeap *descriptor_heap_ptr = (DirectX12_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)descriptor_set_ptr->heap);
	assert(descriptor_heap_ptr);

	uint32_t num_descriptors = 0;
	num_descriptors += descriptor_set_layout_ptr->num_table_cbv_descriptors;
	num_descriptors += descriptor_set_layout_ptr->num_table_srv_descriptors;
	num_descriptors += descriptor_set_layout_ptr->num_table_uav_descriptors;

	uint32_t table_sampler_offset = num_descriptors;
	num_descriptors += descriptor_set_layout_ptr->num_table_sampler_descriptors;

	uint32_t inline_offset = num_descriptors;
	num_descriptors += descriptor_set_layout_ptr->num_inline_descriptors;

	typedef struct DescriptorIndices_t
	{
		uint32_t layout;
		uint32_t entry;
	} DescriptorIndices;

	opal_bumpReset(&device_ptr->bump);
	uint32_t resources_ptr_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(DescriptorIndices *) * num_entries);
	uint32_t samplers_ptr_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(DescriptorIndices *) * num_entries);
	uint32_t inline_ptr_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(DescriptorIndices *) * num_entries);
	uint32_t src_handles_ptr_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * num_entries);
	uint32_t dst_handles_ptr_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * num_entries);

	uint32_t num_resources = 0;
	uint32_t num_samplers = 0;
	uint32_t num_inlines = 0;
	DescriptorIndices *layout_resource_indices = (DescriptorIndices *)(device_ptr->bump.data + resources_ptr_offset);
	DescriptorIndices *layout_sampler_indices = (DescriptorIndices *)(device_ptr->bump.data + samplers_ptr_offset);
	DescriptorIndices *layout_inline_indices = (DescriptorIndices *)(device_ptr->bump.data + inline_ptr_offset);

	// TODO: replace naive O(N) layout <-> binding search loop by O(1) hashmap lookup
	for (uint32_t i = 0; i < num_entries; ++i)
	{
		const Opal_DescriptorSetEntry *entry = &entries[i];

		uint32_t index = UINT32_MAX;
		for (uint32_t j = 0; j < num_descriptors; ++j)
		{
			const DirectX12_DescriptorInfo *info = &descriptor_set_layout_ptr->descriptors[j];
			if (info->binding == entry->binding)
			{
				index = j;
				break;
			}
		}

		assert(index != UINT32_MAX);
		if (index < table_sampler_offset)
		{
			layout_resource_indices[num_resources].entry = i;
			layout_resource_indices[num_resources].layout = index;
			num_resources++;
		}
		else if (index < inline_offset)
		{
			layout_sampler_indices[num_samplers].entry = i;
			layout_sampler_indices[num_samplers].layout = index;
			num_samplers++;
		}
		else
		{
			layout_inline_indices[num_inlines].entry = i;
			layout_inline_indices[num_inlines].layout = index;
			num_inlines++;
		}
	}

	if (num_resources > 0)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heap_info = {0};
		heap_info.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heap_info.NumDescriptors = num_entries;
		heap_info.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		// FIXME: replace by device cbv_srv_uav shader non-visible heap
		ID3D12DescriptorHeap *src_heap = NULL;
		HRESULT hr = ID3D12Device_CreateDescriptorHeap(device_ptr->device, &heap_info, &IID_ID3D12DescriptorHeap, &src_heap);
		if (!SUCCEEDED(hr))
			return OPAL_DIRECTX12_ERROR;

		UINT offset = descriptor_set_ptr->resource_allocation.offset;
		UINT stride = ID3D12Device_GetDescriptorHandleIncrementSize(device_ptr->device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_CPU_DESCRIPTOR_HANDLE src_start = {0};
		ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(src_heap, &src_start);

		D3D12_CPU_DESCRIPTOR_HANDLE dst_start = {0};
		ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(descriptor_heap_ptr->resource_memory, &dst_start);
		dst_start.ptr += stride * offset;

		D3D12_CPU_DESCRIPTOR_HANDLE *src_handles = (D3D12_CPU_DESCRIPTOR_HANDLE *)(device_ptr->bump.data + src_handles_ptr_offset);
		D3D12_CPU_DESCRIPTOR_HANDLE *dst_handles = (D3D12_CPU_DESCRIPTOR_HANDLE *)(device_ptr->bump.data + dst_handles_ptr_offset);
		memset(src_handles, 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * num_resources);
		memset(dst_handles, 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * num_resources);

		for (uint32_t i = 0; i < num_resources; ++i)
		{
			uint32_t layout_index = layout_resource_indices[i].layout;
			uint32_t entry_index = layout_resource_indices[i].entry;

			src_handles[i].ptr = src_start.ptr + stride * layout_index;
			dst_handles[i].ptr = dst_start.ptr + stride * layout_index;

			const DirectX12_DescriptorInfo *info = &descriptor_set_layout_ptr->descriptors[layout_index];
			const Opal_DescriptorSetEntry *entry = &entries[entry_index];
			assert(info->binding == entry->binding);

			switch (info->type)
			{
				case OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				{
					Opal_BufferView buffer_view = entry->data.buffer_view;
					DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer_view.buffer);
					assert(buffer_ptr);

					D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {0};
					desc.BufferLocation = buffer_ptr->address + buffer_view.offset;
					desc.SizeInBytes = (UINT)buffer_view.size;

					ID3D12Device_CreateConstantBufferView(device_ptr->device, &desc, src_handles[i]);
				}
				break;

				case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY:
				{
					Opal_StorageBufferView buffer_view = entry->data.storage_buffer_view;
					DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer_view.buffer);
					assert(buffer_ptr);

					D3D12_SHADER_RESOURCE_VIEW_DESC desc = {0};
					desc.Format = DXGI_FORMAT_UNKNOWN;
					desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
					desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					desc.Buffer.FirstElement = buffer_view.offset / buffer_view.element_size;
					desc.Buffer.NumElements = buffer_view.num_elements;
					desc.Buffer.StructureByteStride = buffer_view.element_size;
					desc.Buffer.Flags = 0;

					ID3D12Device_CreateShaderResourceView(device_ptr->device, buffer_ptr->buffer, &desc, src_handles[i]);
				}
				break;

				case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_1D:
				case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_2D:
				case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_2D_ARRAY:
				case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_CUBE:
				case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_CUBE_ARRAY:
				case OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_3D:
				case OPAL_DESCRIPTOR_TYPE_MULTISAMPLED_TEXTURE_2D:
				{
					DirectX12_TextureView *texture_view_ptr = (DirectX12_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)entry->data.texture_view);
					assert(texture_view_ptr);

					ID3D12Device_CreateShaderResourceView(device_ptr->device, texture_view_ptr->texture, &texture_view_ptr->srv_desc, src_handles[i]);
				}
				break;

				case OPAL_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE:
				{
					// TODO:
				}
				break;

				case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				{
					Opal_StorageBufferView buffer_view = entry->data.storage_buffer_view;
					DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer_view.buffer);
					assert(buffer_ptr);

					D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {0};
					desc.Format = DXGI_FORMAT_UNKNOWN;
					desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
					desc.Buffer.FirstElement = buffer_view.offset / buffer_view.element_size;
					desc.Buffer.NumElements = buffer_view.num_elements;
					desc.Buffer.StructureByteStride = buffer_view.element_size;
					desc.Buffer.CounterOffsetInBytes = 0;
					desc.Buffer.Flags = 0;

					ID3D12Device_CreateUnorderedAccessView(device_ptr->device, buffer_ptr->buffer, NULL, &desc, src_handles[i]);
				}
				break;

				case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_1D:
				case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_2D:
				case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_2D_ARRAY:
				case OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_3D:
				{
					DirectX12_TextureView *texture_view_ptr = (DirectX12_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)entry->data.texture_view);
					assert(texture_view_ptr);

					ID3D12Device_CreateUnorderedAccessView(device_ptr->device, texture_view_ptr->texture, NULL, &texture_view_ptr->uav_desc, src_handles[i]);
				}
				break;

				default: assert(0); break;
			}
		}

		ID3D12Device_CopyDescriptors(device_ptr->device, num_resources, dst_handles, NULL, num_resources, src_handles, NULL, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// FIXME: replace by device cbv_srv_uav shader non-visible heap
		ID3D12DescriptorHeap_Release(src_heap);
	}

	if (num_samplers > 0)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heap_info = {0};
		heap_info.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		heap_info.NumDescriptors = num_entries;
		heap_info.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		// FIXME: replace by device sampler shader non-visible heap
		ID3D12DescriptorHeap *src_heap = NULL;
		HRESULT hr = ID3D12Device_CreateDescriptorHeap(device_ptr->device, &heap_info, &IID_ID3D12DescriptorHeap, &src_heap);
		if (!SUCCEEDED(hr))
			return OPAL_DIRECTX12_ERROR;

		UINT offset = descriptor_set_ptr->sampler_allocation.offset;
		UINT stride = ID3D12Device_GetDescriptorHandleIncrementSize(device_ptr->device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		D3D12_CPU_DESCRIPTOR_HANDLE src_start = {0};
		ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(src_heap, &src_start);

		D3D12_CPU_DESCRIPTOR_HANDLE dst_start = {0};
		ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(descriptor_heap_ptr->sampler_memory, &dst_start);
		dst_start.ptr += stride * offset;

		D3D12_CPU_DESCRIPTOR_HANDLE *src_handles = (D3D12_CPU_DESCRIPTOR_HANDLE *)(device_ptr->bump.data + src_handles_ptr_offset);
		D3D12_CPU_DESCRIPTOR_HANDLE *dst_handles = (D3D12_CPU_DESCRIPTOR_HANDLE *)(device_ptr->bump.data + dst_handles_ptr_offset);
		memset(src_handles, 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * num_samplers);
		memset(dst_handles, 0, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * num_samplers);

		for (uint32_t i = 0; i < num_samplers; ++i)
		{
			uint32_t layout_index = layout_sampler_indices[i].layout;
			uint32_t entry_index = layout_sampler_indices[i].entry;

			assert(layout_index >= num_resources);

			src_handles[i].ptr = src_start.ptr + stride * (layout_index - num_resources);
			dst_handles[i].ptr = dst_start.ptr + stride * (layout_index - num_resources);

			const DirectX12_DescriptorInfo *info = &descriptor_set_layout_ptr->descriptors[layout_index];
			const Opal_DescriptorSetEntry *entry = &entries[entry_index];
			assert(info->binding == entry->binding);
			assert(info->type == OPAL_DESCRIPTOR_TYPE_SAMPLER);

			Opal_Sampler sampler = entry->data.sampler;
			DirectX12_Sampler *sampler_ptr = (DirectX12_Sampler *)opal_poolGetElement(&device_ptr->samplers, (Opal_PoolHandle)sampler);
			assert(sampler_ptr);

			ID3D12Device_CreateSampler(device_ptr->device, &sampler_ptr->desc, src_handles[i]);
		}

		ID3D12Device_CopyDescriptors(device_ptr->device, num_samplers, dst_handles, NULL, num_samplers, src_handles, NULL, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		// FIXME: replace by device sampler shader non-visible heap
		ID3D12DescriptorHeap_Release(src_heap);
	}

	for (uint32_t i = 0; i < num_inlines; ++i)
	{
		uint32_t layout_index = layout_inline_indices[i].layout;
		uint32_t entry_index = layout_inline_indices[i].entry;

		const DirectX12_DescriptorInfo *info = &descriptor_set_layout_ptr->descriptors[layout_index];
		const Opal_DescriptorSetEntry *entry = &entries[entry_index];
		assert(info->binding == entry->binding);

		memcpy(&descriptor_set_ptr->inline_descriptors[i], entry, sizeof(Opal_DescriptorSetEntry));
	}

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceBeginCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	DirectX12_CommandPool *command_pool_ptr = (DirectX12_CommandPool *)opal_poolGetElement(&device_ptr->command_pools, (Opal_PoolHandle)command_buffer_ptr->pool);
	assert(command_pool_ptr);

	uint32_t index = command_buffer_ptr->index;
	assert(index < D3D12_MAX_COMMAND_POOL_ALLOCATORS);

	if (command_buffer_ptr->recording)
		return OPAL_SUCCESS;

	HRESULT hr = ID3D12GraphicsCommandList4_Reset(command_buffer_ptr->list, command_pool_ptr->allocators[index], NULL);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	command_buffer_ptr->recording = 1;
	command_buffer_ptr->root_signature = NULL;
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	if (command_buffer_ptr->recording == 0)
		return OPAL_SUCCESS;

	HRESULT hr = ID3D12GraphicsCommandList4_Close(command_buffer_ptr->list);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	command_buffer_ptr->recording = 0;
	command_buffer_ptr->root_signature = NULL;
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceQuerySemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t *value)
{
	assert(this);
	assert(semaphore);
	assert(value);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_Semaphore *semaphore_ptr = (DirectX12_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);

	*value = ID3D12Fence_GetCompletedValue(semaphore_ptr->fence);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceSignalSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value)
{
	assert(this);
	assert(semaphore);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_Semaphore *semaphore_ptr = (DirectX12_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);

	HRESULT hr = ID3D12Fence_Signal(semaphore_ptr->fence, value);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceWaitSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds)
{
	assert(this);
	assert(semaphore);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_Semaphore *semaphore_ptr = (DirectX12_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);

	ID3D12Fence_SetEventOnCompletion(semaphore_ptr->fence, value, semaphore_ptr->event);
	HRESULT hr = WaitForSingleObjectEx(semaphore_ptr->event, (DWORD)timeout_milliseconds, FALSE);
	if (hr == WAIT_TIMEOUT)
		return OPAL_WAIT_TIMEOUT;

	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceWaitQueue(Opal_Device this, Opal_Queue queue)
{
	assert(this);
	assert(queue);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_Queue *queue_ptr = (DirectX12_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queue);
	assert(queue_ptr);

	queue_ptr->wanted_value++;
	ID3D12CommandQueue_Signal(queue_ptr->queue, queue_ptr->fence, queue_ptr->wanted_value);
	ID3D12Fence_SetEventOnCompletion(queue_ptr->fence, queue_ptr->wanted_value, queue_ptr->event);
	WaitForSingleObjectEx(queue_ptr->event, INFINITE, FALSE);

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceWaitIdle(Opal_Device this)
{
	assert(this);
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
	{
		uint32_t queue_count = device_ptr->device_engines_info.queue_counts[i];
		const Opal_Queue *queues = device_ptr->queue_handles[i];

		for (uint32_t j = 0; j < queue_count; ++j)
			directx12_deviceWaitQueue(this, queues[j]);
	}

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceSubmit(Opal_Device this, Opal_Queue queue, const Opal_SubmitDesc *desc)
{
	assert(this);
	assert(queue);
	assert(desc);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_Queue *queue_ptr = (DirectX12_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queue);
	assert(queue_ptr);

	// NOTE: it seems that IDXGISwapChain is internally synchronized,
	//       so no need to emulate it via waiting or signaling fences

	for (uint32_t i = 0; i < desc->num_wait_semaphores; ++i)
	{
		DirectX12_Semaphore *semaphore_ptr = (DirectX12_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)desc->wait_semaphores[i]);
		assert(semaphore_ptr);

		ID3D12CommandQueue_Wait(queue_ptr->queue, semaphore_ptr->fence, desc->wait_values[i]);
	}

	if (desc->num_command_buffers > 0)
	{
		opal_bumpReset(&device_ptr->bump);
		opal_bumpAlloc(&device_ptr->bump, sizeof(ID3D12CommandList *) * desc->num_command_buffers);
		ID3D12CommandList **submit_command_buffers = (ID3D12CommandList **)(device_ptr->bump.data);

		for (uint32_t i = 0; i < desc->num_command_buffers; ++i)
		{
			DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)desc->command_buffers[i]);
			assert(command_buffer_ptr);

			submit_command_buffers[i] = (ID3D12CommandList *)command_buffer_ptr->list;
		}

		ID3D12CommandQueue_ExecuteCommandLists(queue_ptr->queue, desc->num_command_buffers, submit_command_buffers);
	}

	for (uint32_t i = 0; i < desc->num_signal_semaphores; ++i)
	{
		DirectX12_Semaphore *semaphore_ptr = (DirectX12_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)desc->signal_semaphores[i]);
		assert(semaphore_ptr);

		ID3D12CommandQueue_Signal(queue_ptr->queue, semaphore_ptr->fence, desc->signal_values[i]);
	}

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceAcquire(Opal_Device this, Opal_Swapchain swapchain, Opal_TextureView *texture_view)
{
	assert(this);
	assert(swapchain);
	assert(texture_view);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_Swapchain *swapchain_ptr = (DirectX12_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)swapchain);
	assert(swapchain_ptr);
	assert(swapchain_ptr->current_index < swapchain_ptr->num_textures);

	*texture_view = swapchain_ptr->texture_views[swapchain_ptr->current_index];
	return OPAL_SUCCESS;
}

static Opal_Result directx12_devicePresent(Opal_Device this, Opal_Swapchain swapchain)
{
	assert(this);
	assert(swapchain);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_Swapchain *swapchain_ptr = (DirectX12_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)swapchain);
	assert(swapchain_ptr);

	HRESULT hr = IDXGISwapChain3_Present(swapchain_ptr->swapchain, 0, swapchain_ptr->present_flags);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	swapchain_ptr->current_index = IDXGISwapChain3_GetCurrentBackBufferIndex(swapchain_ptr->swapchain);

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depth_stencil_attachment)
{
	assert(this);
	assert(command_buffer);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	ID3D12Device *d3d12_device = device_ptr->device;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	ID3D12GraphicsCommandList4 *d3d12_command_buffer = command_buffer_ptr->list;

	D3D12_RENDER_PASS_RENDER_TARGET_DESC color_render_targets[8] = {0};
	D3D12_RENDER_PASS_RENDER_TARGET_DESC *color_ptrs = NULL;

	if (num_color_attachments > 0)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtv_start = {0};
		ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(device_ptr->framebuffer_descriptor_heap.rtv_heap, &rtv_start);
		UINT stride = ID3D12Device_GetDescriptorHandleIncrementSize(d3d12_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		for (uint32_t i = 0; i < num_color_attachments; ++i)
		{
			const Opal_FramebufferAttachment *opal_attachment = &color_attachments[i];
			const DirectX12_TextureView *texture_view_ptr = (DirectX12_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)opal_attachment->texture_view);
			assert(texture_view_ptr);

			const DirectX12_TextureView *resolve_texture_view_ptr = (DirectX12_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)opal_attachment->resolve_texture_view);

			D3D12_RENDER_PASS_RENDER_TARGET_DESC *attachment = &color_render_targets[i];

			// descriptor
			attachment->cpuDescriptor.ptr = rtv_start.ptr + stride * i;
			ID3D12Device_CreateRenderTargetView(d3d12_device, texture_view_ptr->texture, &texture_view_ptr->rtv_desc, attachment->cpuDescriptor);

			// beginning access
			attachment->BeginningAccess.Type = directx12_helperToBeginningAccessType(opal_attachment->load_op);
			attachment->BeginningAccess.Clear.ClearValue.Format = texture_view_ptr->rtv_desc.Format;

			assert(sizeof(FLOAT) * 4 == sizeof(Opal_ClearValue));
			memcpy(&attachment->BeginningAccess.Clear.ClearValue.Color, &opal_attachment->clear_value, sizeof(FLOAT) * 4);

			// ending access
			attachment->EndingAccess.Type = directx12_helperToEndingAccessType(opal_attachment->store_op);

			if (resolve_texture_view_ptr)
			{
				attachment->EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
				attachment->EndingAccess.Resolve.pSrcResource = texture_view_ptr->texture;
				attachment->EndingAccess.Resolve.pDstResource = resolve_texture_view_ptr->texture;
				attachment->EndingAccess.Resolve.SubresourceCount = 0;
				attachment->EndingAccess.Resolve.pSubresourceParameters = NULL;
				attachment->EndingAccess.Resolve.Format = resolve_texture_view_ptr->rtv_desc.Format;
				attachment->EndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;
				attachment->EndingAccess.Resolve.PreserveResolveSource = (opal_attachment->store_op == OPAL_STORE_OP_STORE);
			}
		}

		color_ptrs = &color_render_targets[0];
	}

	D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depth_stencil_target = {0};
	D3D12_RENDER_PASS_DEPTH_STENCIL_DESC *depth_stencil_ptr = NULL;

	if (depth_stencil_attachment)
	{
		const DirectX12_TextureView *texture_view_ptr = (DirectX12_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)depth_stencil_attachment->texture_view);
		assert(texture_view_ptr);

		const DirectX12_TextureView *resolve_texture_view_ptr = (DirectX12_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)depth_stencil_attachment->resolve_texture_view);

		// descriptor
		ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(device_ptr->framebuffer_descriptor_heap.dsv_heap, &depth_stencil_target.cpuDescriptor);
		ID3D12Device_CreateDepthStencilView(d3d12_device, texture_view_ptr->texture, &texture_view_ptr->dsv_desc, depth_stencil_target.cpuDescriptor);

		// beginning access
		depth_stencil_target.DepthBeginningAccess.Type = directx12_helperToBeginningAccessType(depth_stencil_attachment->load_op);
		depth_stencil_target.DepthBeginningAccess.Clear.ClearValue.Format = texture_view_ptr->rtv_desc.Format;
		depth_stencil_target.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = depth_stencil_attachment->clear_value.depth_stencil.depth;

		depth_stencil_target.StencilBeginningAccess.Type = directx12_helperToBeginningAccessType(depth_stencil_attachment->load_op);
		depth_stencil_target.StencilBeginningAccess.Clear.ClearValue.Format = texture_view_ptr->rtv_desc.Format;
		depth_stencil_target.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = (UINT8)depth_stencil_attachment->clear_value.depth_stencil.stencil;

		// ending access
		depth_stencil_target.DepthEndingAccess.Type = directx12_helperToEndingAccessType(depth_stencil_attachment->store_op);
		depth_stencil_target.StencilEndingAccess.Type = directx12_helperToEndingAccessType(depth_stencil_attachment->store_op);

		if (resolve_texture_view_ptr)
		{
			depth_stencil_target.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
			depth_stencil_target.DepthEndingAccess.Resolve.pSrcResource = texture_view_ptr->texture;
			depth_stencil_target.DepthEndingAccess.Resolve.pDstResource = resolve_texture_view_ptr->texture;
			depth_stencil_target.DepthEndingAccess.Resolve.SubresourceCount = 0;
			depth_stencil_target.DepthEndingAccess.Resolve.pSubresourceParameters = NULL;
			depth_stencil_target.DepthEndingAccess.Resolve.Format = resolve_texture_view_ptr->rtv_desc.Format;
			depth_stencil_target.DepthEndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;
			depth_stencil_target.DepthEndingAccess.Resolve.PreserveResolveSource = (depth_stencil_attachment->store_op == OPAL_STORE_OP_STORE);

			depth_stencil_target.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
			depth_stencil_target.StencilEndingAccess.Resolve.pSrcResource = texture_view_ptr->texture;
			depth_stencil_target.StencilEndingAccess.Resolve.pDstResource = resolve_texture_view_ptr->texture;
			depth_stencil_target.StencilEndingAccess.Resolve.SubresourceCount = 0;
			depth_stencil_target.StencilEndingAccess.Resolve.pSubresourceParameters = NULL;
			depth_stencil_target.StencilEndingAccess.Resolve.Format = resolve_texture_view_ptr->rtv_desc.Format;
			depth_stencil_target.StencilEndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;
			depth_stencil_target.StencilEndingAccess.Resolve.PreserveResolveSource = (depth_stencil_attachment->store_op == OPAL_STORE_OP_STORE);
		}

		depth_stencil_ptr = &depth_stencil_target;
	}

	command_buffer_ptr->pass = DIRECTX12_PASS_TYPE_GRAPHICS;
	ID3D12GraphicsCommandList4_BeginRenderPass(d3d12_command_buffer, num_color_attachments, color_ptrs, depth_stencil_ptr, D3D12_RENDER_PASS_FLAG_ALLOW_UAV_WRITES);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	command_buffer_ptr->pass = DIRECTX12_PASS_TYPE_NONE;
	ID3D12GraphicsCommandList4_EndRenderPass(command_buffer_ptr->list);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdBeginComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	command_buffer_ptr->pass = DIRECTX12_PASS_TYPE_COMPUTE;
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdEndComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	command_buffer_ptr->pass = DIRECTX12_PASS_TYPE_NONE;
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdBeginRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdEndRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdSetPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Pipeline pipeline)
{
	assert(this);
	assert(command_buffer);
	assert(pipeline);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	DirectX12_Pipeline *pipeline_ptr = (DirectX12_Pipeline *)opal_poolGetElement(&device_ptr->pipelines, (Opal_PoolHandle)pipeline);
	assert(pipeline_ptr);

	ID3D12GraphicsCommandList4 *d3d12_list = command_buffer_ptr->list;
	ID3D12RootSignature *d3d12_root_signature = pipeline_ptr->root_signature;

	// FIXME: keep track of previously bound descriptor sets and rebind them in case root signature changes
	if (command_buffer_ptr->root_signature != d3d12_root_signature)
	{
		command_buffer_ptr->root_signature = d3d12_root_signature;
		switch (command_buffer_ptr->pass)
		{
			case DIRECTX12_PASS_TYPE_GRAPHICS: ID3D12GraphicsCommandList4_SetGraphicsRootSignature(d3d12_list, d3d12_root_signature); break;
			case DIRECTX12_PASS_TYPE_COMPUTE: ID3D12GraphicsCommandList4_SetComputeRootSignature(d3d12_list, d3d12_root_signature); break;
		}
	}

	ID3D12GraphicsCommandList4_SetPipelineState(command_buffer_ptr->list, pipeline_ptr->pipeline_state);
	ID3D12GraphicsCommandList4_IASetPrimitiveTopology(command_buffer_ptr->list, pipeline_ptr->primitive_topology);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdSetDescriptorHeap(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_DescriptorHeap descriptor_heap)
{
	assert(this);
	assert(command_buffer);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	ID3D12DescriptorHeap *heaps[2] = {0};
	uint32_t num_heaps = 0;

	DirectX12_DescriptorHeap *heap_ptr = (DirectX12_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)descriptor_heap);
	assert(heap_ptr);

	if (heap_ptr->resource_memory)
		heaps[num_heaps++] = heap_ptr->resource_memory;

	if (heap_ptr->sampler_memory)
		heaps[num_heaps++] = heap_ptr->sampler_memory;
	
	ID3D12GraphicsCommandList4_SetDescriptorHeaps(command_buffer_ptr->list, num_heaps, heaps);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdSetDescriptorSet(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets)
{
	OPAL_UNUSED(num_dynamic_offsets);
	OPAL_UNUSED(dynamic_offsets);

	assert(this);
	assert(command_buffer);
	assert(pipeline_layout);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	DirectX12_PipelineLayout *pipeline_layout_ptr = (DirectX12_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)pipeline_layout);
	assert(pipeline_layout_ptr);

	DirectX12_DescriptorSet *descriptor_set_ptr = (DirectX12_DescriptorSet *)opal_poolGetElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	assert(descriptor_set_ptr);

	DirectX12_DescriptorSetLayout *descriptor_set_layout_ptr = (DirectX12_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_ptr->layout);
	assert(descriptor_set_layout_ptr);

	ID3D12GraphicsCommandList4 *d3d12_list = command_buffer_ptr->list;
	ID3D12RootSignature *d3d12_root_signature = pipeline_layout_ptr->root_signature;

	// FIXME: keep track of previously bound descriptor sets and rebind them in case root signature changes
	if (command_buffer_ptr->root_signature != d3d12_root_signature)
	{
		command_buffer_ptr->root_signature = d3d12_root_signature;
		switch (command_buffer_ptr->pass)
		{
			case DIRECTX12_PASS_TYPE_GRAPHICS: ID3D12GraphicsCommandList4_SetGraphicsRootSignature(d3d12_list, d3d12_root_signature); break;
			case DIRECTX12_PASS_TYPE_COMPUTE: ID3D12GraphicsCommandList4_SetComputeRootSignature(d3d12_list, d3d12_root_signature); break;
		}
	}

	// CBV SRV UAV descriptor table
	if (descriptor_set_ptr->resource_handle.ptr > 0)
	{
		int table_index = pipeline_layout_ptr->layout_table_offsets[index * 2 + 0];
		assert(table_index != UINT32_MAX);

		switch (command_buffer_ptr->pass)
		{
			case DIRECTX12_PASS_TYPE_GRAPHICS: ID3D12GraphicsCommandList4_SetGraphicsRootDescriptorTable(d3d12_list, table_index, descriptor_set_ptr->resource_handle); break;
			case DIRECTX12_PASS_TYPE_COMPUTE: ID3D12GraphicsCommandList4_SetComputeRootDescriptorTable(d3d12_list, table_index, descriptor_set_ptr->resource_handle); break;
		}
	}

	// sampler descriptor table
	if (descriptor_set_ptr->sampler_handle.ptr > 0)
	{
		int table_index = pipeline_layout_ptr->layout_table_offsets[index * 2 + 1];
		assert(table_index != UINT32_MAX);

		switch (command_buffer_ptr->pass)
		{
			case DIRECTX12_PASS_TYPE_GRAPHICS: ID3D12GraphicsCommandList4_SetGraphicsRootDescriptorTable(d3d12_list, table_index, descriptor_set_ptr->sampler_handle); break;
			case DIRECTX12_PASS_TYPE_COMPUTE: ID3D12GraphicsCommandList4_SetComputeRootDescriptorTable(d3d12_list, table_index, descriptor_set_ptr->sampler_handle); break;
		}
	}

	// inline descriptors
	if (num_dynamic_offsets > 0)
	{
		assert(dynamic_offsets);
		assert(pipeline_layout_ptr->num_inline_descriptors == num_dynamic_offsets);
		assert(descriptor_set_layout_ptr->num_inline_descriptors == descriptor_set_ptr->num_inline_descriptors);

		uint32_t offset = 0;
		offset += descriptor_set_layout_ptr->num_table_cbv_descriptors;
		offset += descriptor_set_layout_ptr->num_table_uav_descriptors;
		offset += descriptor_set_layout_ptr->num_table_srv_descriptors;
		offset += descriptor_set_layout_ptr->num_table_sampler_descriptors;

		for (uint32_t i = 0; i < num_dynamic_offsets; ++i)
		{
			const DirectX12_DescriptorInfo *info = &descriptor_set_layout_ptr->descriptors[offset + i];
			const Opal_DescriptorSetEntry *data = &descriptor_set_ptr->inline_descriptors[i];

			int parameter_index = pipeline_layout_ptr->inline_offset + i;

			switch (info->api_type)
			{
				case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
				{
					Opal_BufferView buffer_view = data->data.buffer_view;
					DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer_view.buffer);
					assert(buffer_ptr);

					D3D12_GPU_VIRTUAL_ADDRESS address = buffer_ptr->address + buffer_view.offset + dynamic_offsets[i];
					switch (command_buffer_ptr->pass)
					{
						case DIRECTX12_PASS_TYPE_GRAPHICS: ID3D12GraphicsCommandList4_SetGraphicsRootConstantBufferView(d3d12_list, parameter_index, address); break;
						case DIRECTX12_PASS_TYPE_COMPUTE: ID3D12GraphicsCommandList4_SetComputeRootConstantBufferView(d3d12_list, parameter_index, address); break;
					}
				}
				break;

				case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
				{
					Opal_StorageBufferView buffer_view = data->data.storage_buffer_view;
					DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer_view.buffer);
					assert(buffer_ptr);

					D3D12_GPU_VIRTUAL_ADDRESS address = buffer_ptr->address + buffer_view.offset + dynamic_offsets[i];
					switch (command_buffer_ptr->pass)
					{
						case DIRECTX12_PASS_TYPE_GRAPHICS: ID3D12GraphicsCommandList4_SetGraphicsRootShaderResourceView(d3d12_list, parameter_index, address); break;
						case DIRECTX12_PASS_TYPE_COMPUTE: ID3D12GraphicsCommandList4_SetComputeRootShaderResourceView(d3d12_list, parameter_index, address); break;
					}
				}
				break;

				case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
				{
					Opal_StorageBufferView buffer_view = data->data.storage_buffer_view;
					DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer_view.buffer);
					assert(buffer_ptr);

					D3D12_GPU_VIRTUAL_ADDRESS address = buffer_ptr->address + buffer_view.offset + dynamic_offsets[i];
					switch (command_buffer_ptr->pass)
					{
						case DIRECTX12_PASS_TYPE_GRAPHICS: ID3D12GraphicsCommandList4_SetGraphicsRootUnorderedAccessView(d3d12_list, parameter_index, address); break;
						case DIRECTX12_PASS_TYPE_COMPUTE: ID3D12GraphicsCommandList4_SetComputeRootUnorderedAccessView(d3d12_list, parameter_index, address); break;
					}
				}
				break;

				default: assert(0); break;
			}
		}
	}

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_VertexBufferView *vertex_buffers)
{
	assert(this);
	assert(command_buffer);
	assert(num_vertex_buffers > 0);
	assert(vertex_buffers);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	opal_bumpReset(&device_ptr->bump);
	uint32_t buffers_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(D3D12_VERTEX_BUFFER_VIEW) * num_vertex_buffers);

	D3D12_VERTEX_BUFFER_VIEW *buffer_views = (D3D12_VERTEX_BUFFER_VIEW *)(device_ptr->bump.data + buffers_offset);

	for (uint32_t i = 0; i < num_vertex_buffers; ++i)
	{
		DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)vertex_buffers[i].buffer);
		assert(buffer_ptr);

		buffer_views[i].BufferLocation = buffer_ptr->address + vertex_buffers[i].offset;
		buffer_views[i].SizeInBytes = vertex_buffers[i].size;
		buffer_views[i].StrideInBytes = vertex_buffers[i].stride;
	}

	ID3D12GraphicsCommandList4_IASetVertexBuffers(command_buffer_ptr->list, 0, num_vertex_buffers, buffer_views);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_IndexBufferView index_buffer)
{
	assert(this);
	assert(command_buffer);
	assert(index_buffer.buffer);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)index_buffer.buffer);
	assert(buffer_ptr);

	D3D12_INDEX_BUFFER_VIEW buffer_view = {0};
	buffer_view.BufferLocation = buffer_ptr->address + index_buffer.offset;
	buffer_view.SizeInBytes = index_buffer.size;
	buffer_view.Format = directx12_helperToDXGIIndexFormat(index_buffer.format);

	ID3D12GraphicsCommandList4_IASetIndexBuffer(command_buffer_ptr->list, &buffer_view);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdSetViewport(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Viewport viewport)
{
	assert(this);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	D3D12_VIEWPORT d3d12_viewport = {0};
	d3d12_viewport.TopLeftX = viewport.x;
	d3d12_viewport.TopLeftY = viewport.y;
	d3d12_viewport.Width = viewport.width;
	d3d12_viewport.Height = viewport.height;
	d3d12_viewport.MinDepth = viewport.min_depth;
	d3d12_viewport.MaxDepth = viewport.max_depth;

	ID3D12GraphicsCommandList4_RSSetViewports(command_buffer_ptr->list, 1, &d3d12_viewport);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdSetScissor(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	assert(this);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	D3D12_RECT rect = {0};
	rect.left = x;
	rect.right = rect.left + width;
	rect.top = y;
	rect.bottom = rect.top + height;

	ID3D12GraphicsCommandList4_RSSetScissorRects(command_buffer_ptr->list, 1, &rect);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdDraw(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertices, uint32_t num_instances, uint32_t base_vertex, uint32_t base_instance)
{
	assert(this);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	ID3D12GraphicsCommandList4_DrawInstanced(command_buffer_ptr->list, num_vertices, num_instances, base_vertex, base_instance);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdDrawIndexed(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t num_instances, uint32_t base_index, int32_t vertex_offset, uint32_t base_instance)
{
	assert(this);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	ID3D12GraphicsCommandList4_DrawIndexedInstanced(command_buffer_ptr->list, num_indices, num_instances, base_index, vertex_offset, base_instance);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdMeshletDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_groups_x);
	OPAL_UNUSED(num_groups_y);
	OPAL_UNUSED(num_groups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdComputeDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	assert(this);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	ID3D12GraphicsCommandList4_Dispatch(command_buffer_ptr->list, num_groups_x, num_groups_y, num_groups_z);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdRaytraceDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView raygen_entry, Opal_BufferView hitgroup_entry, Opal_BufferView miss_entry, uint32_t width, uint32_t height, uint32_t depth)
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

static Opal_Result directx12_deviceCmdBuildAccelerationStructures(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_build_descs, const Opal_AccelerationStructureBuildDesc *descs)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_build_descs);
	OPAL_UNUSED(descs);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdCopyAccelerationStructure(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_AccelerationStructure src, Opal_AccelerationStructure dst, Opal_AccelerationStructureCopyMode mode)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(mode);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdCopyAccelerationStructuresPostbuildInfo(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_src_acceleration_structures, const Opal_AccelerationStructure *src_acceleration_structures, Opal_BufferView dst_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_src_acceleration_structures);
	OPAL_UNUSED(src_acceleration_structures);
	OPAL_UNUSED(dst_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdCopyBufferToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size)
{
	assert(this);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	DirectX12_Buffer *src_buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)src.buffer);
	assert(src_buffer_ptr);

	DirectX12_Buffer *dst_buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)dst.buffer);
	assert(dst_buffer_ptr);

	ID3D12GraphicsCommandList4_CopyBufferRegion(command_buffer_ptr->list, dst_buffer_ptr->buffer, dst.offset, src_buffer_ptr->buffer, src.offset, size);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferTextureRegion src, Opal_TextureRegion dst)
{
	assert(this);
	assert(command_buffer);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	DirectX12_Buffer *src_buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)src.buffer);
	assert(src_buffer_ptr);

	DirectX12_Texture *dst_texture_ptr = (DirectX12_Texture *)opal_poolGetElement(&device_ptr->textures, (Opal_PoolHandle)dst.texture);
	assert(dst_texture_ptr);

	D3D12_TEXTURE_COPY_LOCATION src_location = {0};
	src_location.pResource = src_buffer_ptr->buffer;
	src_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src_location.PlacedFootprint.Offset = src.offset;
	src_location.PlacedFootprint.Footprint.Format = dst_texture_ptr->format;
	src_location.PlacedFootprint.Footprint.Depth = (UINT)dst_texture_ptr->depth;
	src_location.PlacedFootprint.Footprint.Width = (UINT)dst_texture_ptr->width;
	src_location.PlacedFootprint.Footprint.Height = (UINT)src.num_rows;
	src_location.PlacedFootprint.Footprint.RowPitch = (UINT)src.row_size;

	D3D12_TEXTURE_COPY_LOCATION dst_location = {0};
	dst_location.pResource = dst_texture_ptr->texture;
	dst_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst_location.SubresourceIndex = dst.base_mip;

	ID3D12GraphicsCommandList4_CopyTextureRegion(
		command_buffer_ptr->list,
		&dst_location,
		dst.offset_x,
		dst.offset_y,
		dst.offset_z,
		&src_location,
		NULL
	);

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdCopyTextureToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferTextureRegion dst)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdBufferTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	assert(this);
	assert(command_buffer);
	assert(buffer.buffer);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	DirectX12_Buffer *buffer_ptr = (DirectX12_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer.buffer);
	assert(buffer_ptr);

	D3D12_RESOURCE_BARRIER barrier = {0};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = buffer_ptr->buffer;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = directx12_helperToResourceState(state_before);
	barrier.Transition.StateAfter = directx12_helperToResourceState(state_after);

	ID3D12GraphicsCommandList4_ResourceBarrier(command_buffer_ptr->list, 1, &barrier);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdBufferQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdBufferQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdTextureTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	assert(this);
	assert(command_buffer);
 
	DirectX12_Device *device_ptr = (DirectX12_Device *)this;

	DirectX12_CommandBuffer *command_buffer_ptr = (DirectX12_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	DirectX12_TextureView *texture_view_ptr = (DirectX12_TextureView *)opal_poolGetElement(&device_ptr->texture_views, (Opal_PoolHandle)texture_view);
	assert(texture_view_ptr);

	D3D12_RESOURCE_BARRIER barrier = {0};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = texture_view_ptr->texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = directx12_helperToResourceState(state_before);
	barrier.Transition.StateAfter = directx12_helperToResourceState(state_after);

	ID3D12GraphicsCommandList4_ResourceBarrier(command_buffer_ptr->list, 1, &barrier);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdTextureQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdTextureQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
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
	directx12_deviceGetInfo,
	directx12_deviceGetQueue,
	directx12_deviceGetAccelerationStructurePrebuildInfo,
	directx12_deviceGetShaderBindingTablePrebuildInfo,

	directx12_deviceGetSupportedSurfaceFormats,
	directx12_deviceGetSupportedPresentModes,
	directx12_deviceGetPreferredSurfaceFormat,
	directx12_deviceGetPreferredSurfacePresentMode,

	directx12_deviceCreateSemaphore,
	directx12_deviceCreateBuffer,
	directx12_deviceCreateTexture,
	directx12_deviceCreateTextureView,
	directx12_deviceCreateSampler,
	directx12_deviceCreateAccelerationStructure,
	directx12_deviceCreateCommandPool,
	directx12_deviceCreateShader,
	directx12_deviceCreateDescriptorHeap,
	directx12_deviceCreateDescriptorSetLayout,
	directx12_deviceCreatePipelineLayout,
	directx12_deviceCreateGraphicsPipeline,
	directx12_deviceCreateMeshletPipeline,
	directx12_deviceCreateComputePipeline,
	directx12_deviceCreateRaytracePipeline,
	directx12_deviceCreateSwapchain,

	directx12_deviceDestroySemaphore,
	directx12_deviceDestroyBuffer,
	directx12_deviceDestroyTexture,
	directx12_deviceDestroyTextureView,
	directx12_deviceDestroySampler,
	directx12_deviceDestroyAccelerationStructure,
	directx12_deviceDestroyCommandPool,
	directx12_deviceDestroyShader,
	directx12_deviceDestroyDescriptorHeap,
	directx12_deviceDestroyDescriptorSetLayout,
	directx12_deviceDestroyPipelineLayout,
	directx12_deviceDestroyPipeline,
	directx12_deviceDestroySwapchain,
	directx12_deviceDestroy,

	directx12_deviceBuildShaderBindingTable,
	directx12_deviceBuildAccelerationStructureInstanceBuffer,
	directx12_deviceAllocateCommandBuffer,
	directx12_deviceFreeCommandBuffer,
	directx12_deviceResetCommandPool,
	directx12_deviceResetCommandBuffer,
	directx12_deviceAllocateDescriptorSet,
	directx12_deviceFreeDescriptorSet,
	directx12_deviceMapBuffer,
	directx12_deviceUnmapBuffer,
	directx12_deviceWriteBuffer,
	directx12_deviceUpdateDescriptorSet,
	directx12_deviceBeginCommandBuffer,
	directx12_deviceEndCommandBuffer,
	directx12_deviceQuerySemaphore,
	directx12_deviceSignalSemaphore,
	directx12_deviceWaitSemaphore,
	directx12_deviceWaitQueue,
	directx12_deviceWaitIdle,
	directx12_deviceSubmit,
	directx12_deviceAcquire,
	directx12_devicePresent,

	directx12_deviceCmdBeginGraphicsPass,
	directx12_deviceCmdEndGraphicsPass,
	directx12_deviceCmdBeginComputePass,
	directx12_deviceCmdEndComputePass,
	directx12_deviceCmdBeginRaytracePass,
	directx12_deviceCmdEndRaytracePass,
	directx12_deviceCmdSetPipeline,
	directx12_deviceCmdSetDescriptorHeap,
	directx12_deviceCmdSetDescriptorSet,
	directx12_deviceCmdSetVertexBuffers,
	directx12_deviceCmdSetIndexBuffer,
	directx12_deviceCmdSetViewport,
	directx12_deviceCmdSetScissor,
	directx12_deviceCmdDraw,
	directx12_deviceCmdDrawIndexed,
	directx12_deviceCmdMeshletDispatch,
	directx12_deviceCmdComputeDispatch,
	directx12_deviceCmdRaytraceDispatch,
	directx12_deviceCmdBuildAccelerationStructures,
	directx12_deviceCmdCopyAccelerationStructure,
	directx12_deviceCmdCopyAccelerationStructuresPostbuildInfo,
	directx12_deviceCmdCopyBufferToBuffer,
	directx12_deviceCmdCopyBufferToTexture,
	directx12_deviceCmdCopyTextureToBuffer,
	directx12_deviceCmdBufferTransitionBarrier,
	directx12_deviceCmdBufferQueueGrabBarrier,
	directx12_deviceCmdBufferQueueReleaseBarrier,
	directx12_deviceCmdTextureTransitionBarrier,
	directx12_deviceCmdTextureQueueGrabBarrier,
	directx12_deviceCmdTextureQueueReleaseBarrier,
};

/*
 */
Opal_Result directx12_deviceInitialize(DirectX12_Device *device_ptr, DirectX12_Instance *instance_ptr, IDXGIAdapter1 *adapter, ID3D12Device *device)
{
	assert(instance_ptr);
	assert(device_ptr);
	assert(adapter);
	assert(device);

	OPAL_UNUSED(instance_ptr);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	device_ptr->instance = instance_ptr;
	device_ptr->adapter = adapter;
	device_ptr->device = device;

	// allocator
	Opal_Result result = directx12_allocatorInitialize(device_ptr, instance_ptr->heap_size, instance_ptr->max_heap_allocations, instance_ptr->max_heaps);
	assert(result == OPAL_SUCCESS);

	// device engine info
	result = directx12_helperFillDeviceEnginesInfo(&device_ptr->device_engines_info);
	assert(result == OPAL_SUCCESS);

	// framebuffer descriptor heap
	result = directx12_createFramebufferDescriptorHeap(device_ptr, &device_ptr->framebuffer_descriptor_heap);
	assert(result == OPAL_SUCCESS);

	// bump
	opal_bumpInitialize(&device_ptr->bump, 256);

	// pools
	opal_poolInitialize(&device_ptr->queues, sizeof(DirectX12_Queue), 32);
	opal_poolInitialize(&device_ptr->semaphores, sizeof(DirectX12_Semaphore), 32);
	opal_poolInitialize(&device_ptr->buffers, sizeof(DirectX12_Buffer), 32);
	opal_poolInitialize(&device_ptr->textures, sizeof(DirectX12_Texture), 32);
	opal_poolInitialize(&device_ptr->texture_views, sizeof(DirectX12_TextureView), 32);
	opal_poolInitialize(&device_ptr->samplers, sizeof(DirectX12_Sampler), 32);
	opal_poolInitialize(&device_ptr->command_pools, sizeof(DirectX12_CommandPool), 32);
	opal_poolInitialize(&device_ptr->command_buffers, sizeof(DirectX12_CommandBuffer), 32);
	opal_poolInitialize(&device_ptr->shaders, sizeof(DirectX12_Shader), 32);
	opal_poolInitialize(&device_ptr->descriptor_heaps, sizeof(DirectX12_DescriptorHeap), 32);
	opal_poolInitialize(&device_ptr->descriptor_set_layouts, sizeof(DirectX12_DescriptorSetLayout), 32);
	opal_poolInitialize(&device_ptr->descriptor_sets, sizeof(DirectX12_DescriptorSet), 32);
	opal_poolInitialize(&device_ptr->pipeline_layouts, sizeof(DirectX12_PipelineLayout), 32);
	opal_poolInitialize(&device_ptr->pipelines, sizeof(DirectX12_Pipeline), 32);
	opal_poolInitialize(&device_ptr->swapchains, sizeof(DirectX12_Swapchain), 32);

	// queues
	const DirectX12_DeviceEnginesInfo *engines_info = &device_ptr->device_engines_info;

	static D3D12_COMMAND_LIST_TYPE queue_types[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX] = 
	{
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_LIST_TYPE_COMPUTE,
		D3D12_COMMAND_LIST_TYPE_COPY,
	};

	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
	{
		uint32_t queue_count = engines_info->queue_counts[i];

		DirectX12_Queue queue = {0};
		D3D12_COMMAND_QUEUE_DESC desc = {0};
		desc.Type = queue_types[i];

		Opal_Queue *queue_handles = (Opal_Queue *)malloc(sizeof(Opal_Queue) * queue_count);

		for (uint32_t j = 0; j < queue_count; j++)
		{
			HRESULT hr = ID3D12Device_CreateCommandQueue(device_ptr->device, &desc, &IID_ID3D12CommandQueue, &queue.queue);
			assert(SUCCEEDED(hr));

			hr = ID3D12Device_CreateFence(device_ptr->device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &queue.fence);
			assert(SUCCEEDED(hr));

			queue.event = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
			queue.wanted_value = 0;

			OPAL_UNUSED(hr);
			queue_handles[j] = (Opal_Queue)opal_poolAddElement(&device_ptr->queues, &queue);
		}

		device_ptr->queue_handles[i] = queue_handles;
	}

	return OPAL_SUCCESS;
}

Opal_Result directx12_deviceAllocateMemory(DirectX12_Device *device_ptr, const DirectX12_AllocationDesc *desc, DirectX12_Allocation *allocation)
{
	assert(device_ptr);

	DirectX12_Allocator *allocator = &device_ptr->allocator;

	// resolve allocation type (suballocated or dedicated)
	uint32_t dedicated = (desc->hint == OPAL_ALLOCATION_HINT_PREFER_DEDICATED);
	const float heap_threshold = 0.7f;

	switch (desc->hint)
	{
		case OPAL_ALLOCATION_HINT_AUTO:
		{
			uint32_t too_big_for_heap = desc->size > allocator->heap_size * heap_threshold;
			dedicated = too_big_for_heap;
		}
		break;

		case OPAL_ALLOCATION_HINT_PREFER_HEAP:
		{
			uint32_t wont_fit_in_heap = desc->size > allocator->heap_size;
			dedicated = wont_fit_in_heap;
			break;
		}
	}

	Opal_Result opal_result = OPAL_NO_MEMORY;

	if (dedicated == 0)
		opal_result = directx12_allocatorAllocateMemory(device_ptr, desc, 0, allocation);

	if (opal_result == OPAL_NO_MEMORY)
		opal_result = directx12_allocatorAllocateMemory(device_ptr, desc, 1, allocation);

	return opal_result;
}
