#include "directx12_internal.h"
#include <assert.h>

/*
 */
static Opal_Result directx12_deviceFreeBindset(Opal_Device this, Opal_BindsetPool bindset_pool, Opal_Bindset bindset);
static Opal_Result directx12_deviceUpdateBindset(Opal_Device this, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings);

/*
 */
static void directx12_destroyBuffer(DirectX12_Device *device_ptr, DirectX12_Buffer *buffer_ptr)
{
	assert(device_ptr);
	assert(buffer_ptr);

	ID3D12Resource_Release(buffer_ptr->buffer);
	directx12_allocatorFreeMemory(device_ptr, buffer_ptr->allocation);
}

/*
 */
static Opal_Result directx12_deviceGetInfo(Opal_Device this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	DirectX12_Device *ptr = (DirectX12_Device *)this;
	return directx12_fillDeviceInfo(ptr->adapter, ptr->device, info);
}

static Opal_Result directx12_deviceGetQueue(Opal_Device this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(engine_type);
	OPAL_UNUSED(index);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(num_formats);
	OPAL_UNUSED(formats);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceGetSupportedPresentModes(Opal_Device this, Opal_Surface surface, uint32_t *num_present_modes, Opal_PresentMode *present_modes)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(num_present_modes);
	OPAL_UNUSED(present_modes);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceGetPreferredSurfaceFormat(Opal_Device this, Opal_Surface surface, Opal_SurfaceFormat *format)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(format);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceGetPreferredSurfacePresentMode(Opal_Device this, Opal_Surface surface, Opal_PresentMode *present_mode)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(present_mode);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCreateSemaphore(Opal_Device this, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(semaphore);

	return OPAL_NOT_SUPPORTED;
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
		return OPAL_DIRECX12_ERROR;
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCreateShader(Opal_Device this, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(shader);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCreateBindsetLayout(Opal_Device this, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(num_bindings);
	OPAL_UNUSED(bindings);
	OPAL_UNUSED(bindset_layout);

	return OPAL_NOT_SUPPORTED;
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(num_bindset_layouts);
	OPAL_UNUSED(bindset_layouts);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceDestroySemaphore(Opal_Device this, Opal_Semaphore semaphore)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);

	return OPAL_NOT_SUPPORTED;
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(shader);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceDestroyBindsetLayout(Opal_Device this, Opal_BindsetLayout bindset_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceDestroyBindsetPool(Opal_Device this, Opal_BindsetPool bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceDestroyPipeline(Opal_Device this, Opal_Pipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceDestroySwapchain(Opal_Device this, Opal_Swapchain swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceDestroy(Opal_Device this)
{
	assert(this);

	DirectX12_Device *ptr = (DirectX12_Device *)this;

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
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceFreeCommandBuffer(Opal_Device this, Opal_CommandPool command_pool, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceResetCommandPool(Opal_Device this, Opal_CommandPool command_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceResetCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
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
		return OPAL_DIRECX12_ERROR;

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
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceQuerySemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t *value)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceSignalSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceWaitSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);
	OPAL_UNUSED(timeout_milliseconds);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceWaitQueue(Opal_Device this, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceWaitIdle(Opal_Device this)
{
	OPAL_UNUSED(this);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceSubmit(Opal_Device this, Opal_Queue queue, const Opal_SubmitDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_deviceAcquire(Opal_Device this, Opal_Swapchain swapchain, Opal_TextureView *texture_view)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);
	OPAL_UNUSED(texture_view);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_devicePresent(Opal_Device this, Opal_Swapchain swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
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
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
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
	device_ptr->adapter = adapter;
	device_ptr->device = device;

	// allocator
	Opal_Result result = directx12_allocatorInitialize(device_ptr, instance_ptr->heap_size, instance_ptr->max_heap_allocations, instance_ptr->max_heaps);
	assert(result == OPAL_SUCCESS);

	OPAL_UNUSED(result);

	// pools
	opal_poolInitialize(&device_ptr->buffers, sizeof(DirectX12_Buffer), 32);

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
