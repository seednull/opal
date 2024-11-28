#include "directx12_internal.h"
#include <assert.h>

/*
 */
static Opal_Result directx12_deviceFreeBindset(Opal_Device this, Opal_BindsetPool bindset_pool, Opal_Bindset bindset);
static Opal_Result directx12_deviceUpdateBindset(Opal_Device this, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings);

/*
 */
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

	ID3D12GraphicsCommandList_Release(command_buffer_ptr->list);
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

	ID3D12DescriptorHeap_Release(descriptor_heap_ptr->memory);
	opal_heapShutdown(&descriptor_heap_ptr->heap);
}

static void directx12_destroyBindsetLayout(DirectX12_Device *device_ptr, DirectX12_BindsetLayout *bindset_layout_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(bindset_layout_ptr);

	free(bindset_layout_ptr->descriptors);
}

static void directx12_destroyPipelineLayout(DirectX12_Device *device_ptr, DirectX12_PipelineLayout *pipeline_layout_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(pipeline_layout_ptr);

	ID3D12RootSignature_Release(pipeline_layout_ptr->root_signature);
}

static void directx12_destroyPipeline(DirectX12_Device *device_ptr, DirectX12_Pipeline *pipeline_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(pipeline_ptr);

	ID3D12PipelineState_Release(pipeline_ptr->pipeline_state);
}

static void directx12_destroySwapchain(DirectX12_Device *device_ptr, DirectX12_Swapchain *swapchain_ptr)
{
	OPAL_UNUSED(device_ptr);
	assert(swapchain_ptr);

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

	// fill allocation info
	DirectX12_AllocationDesc allocation_desc = {0};
	allocation_desc.size = desc->size;
	allocation_desc.resource_type = DIRECTX12_RESOURCE_TYPE_BUFFER;
	allocation_desc.allocation_type = desc->memory_type;
	allocation_desc.hint = desc->hint;

	Opal_Result opal_result = directx12_deviceAllocateMemory(device_ptr, &allocation_desc, &allocation);
	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	// fill buffer info
	D3D12_RESOURCE_DESC buffer_info = {0};
	buffer_info.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buffer_info.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	buffer_info.Width = desc->size;
	buffer_info.Height = 1;
	buffer_info.DepthOrArraySize = 1;
	buffer_info.MipLevels = 1;
	buffer_info.Format = DXGI_FORMAT_UNKNOWN;
	buffer_info.SampleDesc.Count = 1;
	buffer_info.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	if (desc->usage & OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE)
		buffer_info.Flags = D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;

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
	result.allocation = allocation;

	*buffer = (Opal_Buffer)opal_poolAddElement(&device_ptr->buffers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(texture);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(texture_view);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCreateSampler(Opal_Device this, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(sampler);

	return OPAL_NOT_SUPPORTED;
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

	D3D12_DESCRIPTOR_HEAP_DESC heap_info = {0};
	heap_info.Type = directx12_helperToDescriptorHeapType(desc->type);
	heap_info.NumDescriptors = desc->num_descriptors;
	heap_info.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	ID3D12DescriptorHeap *d3d12_heap = NULL;
	HRESULT hr = ID3D12Device_CreateDescriptorHeap(d3d12_device, &heap_info, &IID_ID3D12DescriptorHeap, &d3d12_heap);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	DirectX12_DescriptorHeap result = {0};
	result.memory = d3d12_heap;
	result.type = heap_info.Type;

	Opal_Result opal_result = opal_heapInitialize(&result.heap, desc->num_descriptors, desc->num_descriptors);
	OPAL_UNUSED(opal_result);

	assert(opal_result == OPAL_SUCCESS);

	*descriptor_heap = (Opal_DescriptorHeap)opal_poolAddElement(&device_ptr->descriptor_heaps, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateBindsetLayout(Opal_Device this, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout)
{
	assert(this);
	assert(num_bindings > 0);
	assert(bindings);
	assert(bindset_layout);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_BindsetLayout result = {0};

	for (uint32_t i = 0; i < num_bindings; ++i)
	{
		switch (bindings[i].type)
		{
			case OPAL_BINDING_TYPE_UNIFORM_BUFFER:
			{
				result.num_table_cbv_descriptors++;
			}
			break;

			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_1D:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_2D:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_2D_ARRAY:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_CUBE:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_CUBE_ARRAY:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_3D:
			case OPAL_BINDING_TYPE_MULTISAMPLED_TEXTURE_2D:
			case OPAL_BINDING_TYPE_ACCELERATION_STRUCTURE: // ?
			{
				result.num_table_srv_descriptors++;
			}
			break;

			case OPAL_BINDING_TYPE_STORAGE_BUFFER:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER_READONLY:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_1D:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_2D:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_2D_ARRAY:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_3D:
			{
				result.num_table_uav_descriptors++;
			}
			break;

			case OPAL_BINDING_TYPE_SAMPLER:
			case OPAL_BINDING_TYPE_COMPARE_SAMPLER:
			{
				result.num_table_sampler_descriptors++;
			}
			break;

			case OPAL_BINDING_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER_DYNAMIC:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC:
			{
				result.num_inline_descriptors++;
			}
			break;

			default: assert(0); break;
		}
	}

	uint32_t table_cbv_descriptor_offset = 0;
	uint32_t table_uav_descriptor_offset = table_cbv_descriptor_offset + result.num_table_cbv_descriptors;
	uint32_t table_srv_descriptor_offset = table_uav_descriptor_offset + result.num_table_uav_descriptors;
	uint32_t table_sampler_descriptor_offset = table_srv_descriptor_offset + result.num_table_srv_descriptors;
	uint32_t inline_descriptor_offset = table_sampler_descriptor_offset + result.num_table_sampler_descriptors;

	for (uint32_t i = 0; i < num_bindings; ++i)
	{
		uint32_t index = 0;
		D3D12_DESCRIPTOR_RANGE_TYPE type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;

		switch (bindings[i].type)
		{
			case OPAL_BINDING_TYPE_UNIFORM_BUFFER:
			{
				index = table_cbv_descriptor_offset++;
			}
			break;

			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_1D:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_2D:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_2D_ARRAY:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_CUBE:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_CUBE_ARRAY:
			case OPAL_BINDING_TYPE_SAMPLED_TEXTURE_3D:
			case OPAL_BINDING_TYPE_MULTISAMPLED_TEXTURE_2D:
			case OPAL_BINDING_TYPE_ACCELERATION_STRUCTURE: // ?
			{
				index = table_srv_descriptor_offset++;
				type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			}
			break;

			case OPAL_BINDING_TYPE_STORAGE_BUFFER:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER_READONLY:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_1D:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_2D:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_2D_ARRAY:
			case OPAL_BINDING_TYPE_STORAGE_TEXTURE_3D:
			{
				index = table_uav_descriptor_offset++;
				type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			}
			break;

			case OPAL_BINDING_TYPE_SAMPLER:
			case OPAL_BINDING_TYPE_COMPARE_SAMPLER:
			{
				index = table_sampler_descriptor_offset++;
				type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			}
			break;

			case OPAL_BINDING_TYPE_STORAGE_BUFFER_DYNAMIC:
			case OPAL_BINDING_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC:
			{
				type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				index = inline_descriptor_offset++;
			}
			break;

			case OPAL_BINDING_TYPE_UNIFORM_BUFFER_DYNAMIC:
			{
				index = inline_descriptor_offset++;
			}
			break;

			default: assert(0); break;
		}

		result.descriptors[index].type = type;
		result.descriptors[index].binding = bindings[i].binding;
		result.descriptors[index].visibility = directx12_helperToShaderVisibility(bindings[i].visibility);
	}

	result.descriptors = (DirectX12_DescriptorInfo *)malloc(num_bindings * sizeof(DirectX12_DescriptorInfo));

	*bindset_layout = (Opal_BindsetLayout)opal_poolAddElement(&device_ptr->bindset_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCreateBindsetPool(Opal_Device this, const Opal_BindsetPoolDesc *desc, Opal_BindsetPool *bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout)
{
	assert(this);
	assert(pipeline_layout);
	assert(num_bindset_layouts == 0 || bindset_layouts);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	ID3D12Device *d3d12_device = device_ptr->device;

	D3D12_ROOT_PARAMETER *parameters = NULL;
	D3D12_DESCRIPTOR_RANGE *ranges = NULL;
	uint32_t num_inline_descriptors = 0;

	if (num_bindset_layouts > 0)
	{
		uint32_t num_table_descriptors = 0;
		for (uint32_t i = 0; i < num_bindset_layouts; ++i)
		{
			DirectX12_BindsetLayout *bindset_layout_ptr = (DirectX12_BindsetLayout *)opal_poolGetElement(&device_ptr->bindset_layouts, (Opal_PoolHandle)bindset_layouts[i]);
			assert(bindset_layout_ptr);
			assert(bindset_layout_ptr->descriptors);

			num_inline_descriptors += bindset_layout_ptr->num_inline_descriptors;
			num_table_descriptors += bindset_layout_ptr->num_table_cbv_descriptors;
			num_table_descriptors += bindset_layout_ptr->num_table_uav_descriptors;
			num_table_descriptors += bindset_layout_ptr->num_table_srv_descriptors;
			num_table_descriptors += bindset_layout_ptr->num_table_sampler_descriptors;
		}

		opal_bumpReset(&device_ptr->bump);
		uint32_t parameters_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(D3D12_ROOT_PARAMETER) * (num_bindset_layouts + num_inline_descriptors));
		uint32_t ranges_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(D3D12_DESCRIPTOR_RANGE) * num_table_descriptors);

		parameters = (D3D12_ROOT_PARAMETER *)(device_ptr->bump.data + parameters_offset);
		ranges = (D3D12_DESCRIPTOR_RANGE *)(device_ptr->bump.data + ranges_offset);

		D3D12_ROOT_PARAMETER *table_parameters = parameters;
		for (uint32_t i = 0; i < num_bindset_layouts; ++i)
		{
			DirectX12_BindsetLayout *bindset_layout_ptr = (DirectX12_BindsetLayout *)opal_poolGetElement(&device_ptr->bindset_layouts, (Opal_PoolHandle)bindset_layouts[i]);
			assert(bindset_layout_ptr);
			assert(bindset_layout_ptr->descriptors);

			DirectX12_DescriptorInfo *current_descriptor = bindset_layout_ptr->descriptors;

			uint32_t num_ranges = 0;

			for (uint32_t j = 0; j < bindset_layout_ptr->num_table_cbv_descriptors; ++j)
			{
				assert(current_descriptor->type == D3D12_DESCRIPTOR_RANGE_TYPE_CBV);

				D3D12_DESCRIPTOR_RANGE *current_range = &ranges[num_ranges];
				current_range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				current_range->NumDescriptors = 1;
				current_range->BaseShaderRegister = current_descriptor->binding;
				current_range->RegisterSpace = i;
				current_range->OffsetInDescriptorsFromTableStart = num_ranges;

				num_ranges++;
				current_descriptor++;
			}

			for (uint32_t j = 0; j < bindset_layout_ptr->num_table_uav_descriptors; ++j)
			{
				assert(current_descriptor->type == D3D12_DESCRIPTOR_RANGE_TYPE_UAV);

				D3D12_DESCRIPTOR_RANGE *current_range = &ranges[num_ranges];
				current_range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				current_range->NumDescriptors = 1;
				current_range->BaseShaderRegister = current_descriptor->binding;
				current_range->RegisterSpace = i;
				current_range->OffsetInDescriptorsFromTableStart = num_ranges;

				num_ranges++;
				current_descriptor++;
			}

			for (uint32_t j = 0; j < bindset_layout_ptr->num_table_srv_descriptors; ++j)
			{
				assert(current_descriptor->type == D3D12_DESCRIPTOR_RANGE_TYPE_SRV);

				D3D12_DESCRIPTOR_RANGE *current_range = &ranges[num_ranges];
				current_range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				current_range->NumDescriptors = 1;
				current_range->BaseShaderRegister = current_descriptor->binding;
				current_range->RegisterSpace = i;
				current_range->OffsetInDescriptorsFromTableStart = num_ranges;

				num_ranges++;
				current_descriptor++;
			}

			for (uint32_t j = 0; j < bindset_layout_ptr->num_table_sampler_descriptors; ++j)
			{
				assert(current_descriptor->type == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);

				D3D12_DESCRIPTOR_RANGE *current_range = &ranges[num_ranges];
				current_range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				current_range->NumDescriptors = 1;
				current_range->BaseShaderRegister = current_descriptor->binding;
				current_range->RegisterSpace = i;
				current_range->OffsetInDescriptorsFromTableStart = num_ranges;

				num_ranges++;
				current_descriptor++;	
			}

			D3D12_ROOT_PARAMETER *parameter = &table_parameters[i];
			parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			parameter->ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			parameter->DescriptorTable.NumDescriptorRanges = num_ranges;
			parameter->DescriptorTable.pDescriptorRanges = ranges;

			ranges += num_ranges;
		}

		D3D12_ROOT_PARAMETER *inline_parameters = parameters + num_bindset_layouts;
		for (uint32_t i = 0; i < num_bindset_layouts; ++i)
		{
			DirectX12_BindsetLayout *bindset_layout_ptr = (DirectX12_BindsetLayout *)opal_poolGetElement(&device_ptr->bindset_layouts, (Opal_PoolHandle)bindset_layouts[i]);
			assert(bindset_layout_ptr);
			assert(bindset_layout_ptr->descriptors);

			DirectX12_DescriptorInfo *current_descriptor = bindset_layout_ptr->descriptors;

			for (uint32_t j = 0; j < bindset_layout_ptr->num_inline_descriptors; ++j)
			{
				assert(current_descriptor->type == D3D12_DESCRIPTOR_RANGE_TYPE_CBV || current_descriptor->type == D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
				D3D12_ROOT_PARAMETER *parameter = &inline_parameters[i];

				parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

				if (current_descriptor->type == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
					parameter->ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;

				parameter->ShaderVisibility = current_descriptor->visibility;
				parameter->Descriptor.RegisterSpace = i;
				parameter->Descriptor.ShaderRegister = current_descriptor->binding;

				current_descriptor++;
			}
		}
	}

	D3D12_ROOT_SIGNATURE_DESC layout_info = {0};
	layout_info.NumParameters = num_bindset_layouts + num_inline_descriptors;
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

	*pipeline_layout = (Opal_BindsetLayout)opal_poolAddElement(&device_ptr->pipeline_layouts, &result);
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
	pipeline_info.PrimitiveTopologyType = directx12_helperToPrimitiveTopology(desc->primitive_type);

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

	DirectX12_Pipeline result = {0};
	result.pipeline_state = d3d12_pipeline_state;

	*pipeline = (Opal_BindsetLayout)opal_poolAddElement(&device_ptr->pipelines, &result);
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
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

	// TODO: create texture views from swapchain buffers
	// ID3D12Resource *d3d12_resource = NULL;
	// IDXGISwapChain3_GetBuffer(d3d12_swapchain3, 0, &IID_ID3D12Resource, &d3d12_resource);

	// create opal struct
	DirectX12_Swapchain result = {0};
	result.swapchain = d3d12_swapchain3;
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(texture);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(texture_view);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceDestroySampler(Opal_Device this, Opal_Sampler sampler)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(sampler);

	return OPAL_NOT_SUPPORTED;
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

static Opal_Result directx12_deviceDestroyBindsetLayout(Opal_Device this, Opal_BindsetLayout bindset_layout)
{
	assert(this);
	assert(bindset_layout);

	Opal_PoolHandle handle = (Opal_PoolHandle)bindset_layout;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Device *device_ptr = (DirectX12_Device *)this;
	DirectX12_BindsetLayout *bindset_layout_ptr = (DirectX12_BindsetLayout *)opal_poolGetElement(&device_ptr->bindset_layouts, handle);
	assert(bindset_layout_ptr);

	opal_poolRemoveElement(&device_ptr->bindset_layouts, handle);

	directx12_destroyBindsetLayout(device_ptr, bindset_layout_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceDestroyBindsetPool(Opal_Device this, Opal_BindsetPool bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
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
		uint32_t head = opal_poolGetHeadIndex(&ptr->bindset_layouts);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			DirectX12_BindsetLayout *bindset_layout_ptr = (DirectX12_BindsetLayout *)opal_poolGetElementByIndex(&ptr->bindset_layouts, head);
			directx12_destroyBindsetLayout(ptr, bindset_layout_ptr);

			head = opal_poolGetNextIndex(&ptr->bindset_layouts, head);
		}

		opal_poolShutdown(&ptr->bindset_layouts);
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

	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
		free(ptr->queue_handles[i]);

	opal_bumpShutdown(&ptr->bump);

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

	ID3D12GraphicsCommandList *d3d12_command_list = NULL;
	HRESULT hr = ID3D12Device_CreateCommandList(d3d12_device, 0, command_pool_ptr->type, d3d12_command_allocator, NULL, &IID_ID3D12GraphicsCommandList, &d3d12_command_list);
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
		HRESULT hr = ID3D12GraphicsCommandList_Close(command_buffer_ptr->list);
		if (!SUCCEEDED(hr))
			return OPAL_DIRECTX12_ERROR;
	}

	HRESULT hr = ID3D12GraphicsCommandList_Reset(command_buffer_ptr->list, command_pool_ptr->allocators[index], NULL);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	command_buffer_ptr->recording = 1;
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceAllocateEmptyBindset(Opal_Device this, Opal_BindsetLayout bindset_layout, Opal_BindsetPool bindset_pool, Opal_Bindset *bindset)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_layout);
	OPAL_UNUSED(bindset_pool);
	OPAL_UNUSED(bindset);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceAllocatePrefilledBindset(Opal_Device this, Opal_BindsetLayout bindset_layout, Opal_BindsetPool bindset_pool, uint32_t num_bindings, const Opal_BindsetBinding *bindings, Opal_Bindset *bindset)
{
	Opal_Result result = directx12_deviceAllocateEmptyBindset(this, bindset_layout, bindset_pool, bindset);
	if (result != OPAL_SUCCESS)
		return result;

	assert(bindset);
	result = directx12_deviceUpdateBindset(this, *bindset, num_bindings, bindings);
	if (result != OPAL_SUCCESS)
	{
		directx12_deviceFreeBindset(this, bindset_pool, *bindset);
		*bindset = OPAL_NULL_HANDLE;
		return result;
	}

	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceFreeBindset(Opal_Device this, Opal_BindsetPool bindset_pool, Opal_Bindset bindset)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);
	OPAL_UNUSED(bindset);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceResetBindsetPool(Opal_Device this, Opal_BindsetPool bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
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

static Opal_Result directx12_deviceUpdateBindset(Opal_Device this, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset);
	OPAL_UNUSED(num_bindings);
	OPAL_UNUSED(bindings);

	return OPAL_NOT_SUPPORTED;
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

	HRESULT hr = ID3D12GraphicsCommandList_Reset(command_buffer_ptr->list, command_pool_ptr->allocators[index], NULL);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	command_buffer_ptr->recording = 1;
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

	HRESULT hr = ID3D12GraphicsCommandList_Close(command_buffer_ptr->list);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	command_buffer_ptr->recording = 0;
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

	UINT64 current_value = ID3D12Fence_GetCompletedValue(queue_ptr->fence);
	UINT64 wanted_value = queue_ptr->wanted_value;

	if (wanted_value <= current_value)
		return OPAL_SUCCESS;

	ID3D12Fence_SetEventOnCompletion(queue_ptr->fence, wanted_value, queue_ptr->event);
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

		queue_ptr->wanted_value++;
		ID3D12CommandQueue_Signal(queue_ptr->queue, queue_ptr->fence, queue_ptr->wanted_value);
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

	// *texture_view = swapchain_ptr->texture_views[swapchain_ptr->current_index];
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_color_attachments);
	OPAL_UNUSED(color_attachments);
	OPAL_UNUSED(depth_stencil_attachment);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdBeginComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdEndComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdSetBindset(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout, uint32_t index, Opal_Bindset bindset, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline_layout);
	OPAL_UNUSED(index);
	OPAL_UNUSED(bindset);
	OPAL_UNUSED(num_dynamic_offsets);
	OPAL_UNUSED(dynamic_offsets);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_vertex_buffers);
	OPAL_UNUSED(vertex_buffers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer, Opal_IndexFormat index_format)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(index_buffer);
	OPAL_UNUSED(index_format);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdSetViewport(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Viewport viewport)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(viewport);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdSetScissor(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(x);
	OPAL_UNUSED(y);
	OPAL_UNUSED(width);
	OPAL_UNUSED(height);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdDraw(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertices, uint32_t num_instances, uint32_t base_vertex, uint32_t base_instance)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_vertices);
	OPAL_UNUSED(num_instances);
	OPAL_UNUSED(base_vertex);
	OPAL_UNUSED(base_instance);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCmdDrawIndexed(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t num_instances, uint32_t base_index, int32_t vertex_offset, uint32_t base_instance)
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_groups_x);
	OPAL_UNUSED(num_groups_y);
	OPAL_UNUSED(num_groups_z);

	return OPAL_NOT_SUPPORTED;
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

	ID3D12GraphicsCommandList_CopyBufferRegion(command_buffer_ptr->list, dst_buffer_ptr->buffer, dst.offset, src_buffer_ptr->buffer, src.offset, size);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferTextureRegion src, Opal_TextureRegion dst)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);

	return OPAL_NOT_SUPPORTED;
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_NOT_SUPPORTED;
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_NOT_SUPPORTED;
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
	directx12_deviceCreateBindsetLayout,
	directx12_deviceCreateBindsetPool,
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
	directx12_deviceDestroyDescriptorHeap,
	directx12_deviceDestroyShader,
	directx12_deviceDestroyBindsetLayout,
	directx12_deviceDestroyBindsetPool,
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
	directx12_deviceAllocateEmptyBindset,
	directx12_deviceAllocatePrefilledBindset,
	directx12_deviceFreeBindset,
	directx12_deviceResetBindsetPool,
	directx12_deviceMapBuffer,
	directx12_deviceUnmapBuffer,
	directx12_deviceWriteBuffer,
	directx12_deviceUpdateBindset,
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
	directx12_deviceCmdSetBindset,
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

	OPAL_UNUSED(result);

	// device engine info
	result = directx12_helperFillDeviceEnginesInfo(&device_ptr->device_engines_info);
	assert(result == OPAL_SUCCESS);

	// bump
	opal_bumpInitialize(&device_ptr->bump, 256);

	// pools
	opal_poolInitialize(&device_ptr->queues, sizeof(DirectX12_Queue), 32);
	opal_poolInitialize(&device_ptr->semaphores, sizeof(DirectX12_Semaphore), 32);
	opal_poolInitialize(&device_ptr->buffers, sizeof(DirectX12_Buffer), 32);
	opal_poolInitialize(&device_ptr->command_pools, sizeof(DirectX12_CommandPool), 32);
	opal_poolInitialize(&device_ptr->command_buffers, sizeof(DirectX12_CommandBuffer), 32);
	opal_poolInitialize(&device_ptr->shaders, sizeof(DirectX12_Shader), 32);
	opal_poolInitialize(&device_ptr->descriptor_heaps, sizeof(DirectX12_DescriptorHeap), 32);
	opal_poolInitialize(&device_ptr->bindset_layouts, sizeof(DirectX12_BindsetLayout), 32);
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
