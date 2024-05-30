#include "opal_internal.h"

/*
 */
Opal_Result opalCreateInstance(Opal_Api api, const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	switch (api)
	{
		case OPAL_API_VULKAN: return vulkan_createInstance(desc, instance);
		case OPAL_API_DIRECTX12: return directx12_createInstance(desc, instance);
		case OPAL_API_METAL: return metal_createInstance(desc, instance);
		case OPAL_API_WEBGPU: return webgpu_createInstance(desc, instance);
		case OPAL_API_NULL: return null_createInstance(desc, instance);

		default: return OPAL_NOT_SUPPORTED;
	}
}

Opal_Result opalDestroyInstance(Opal_Instance instance)
{
	// FIXME: change to handle + generation and do proper check
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Instance *ptr = (Instance *)(instance);
	Opal_Result result = ptr->destroy(ptr);

	free(ptr);
	return result;
}

/*
 */
Opal_Result opalEnumerateDevices(Opal_Instance instance, uint32_t *device_count, Opal_DeviceInfo *infos)
{
	// FIXME: change to handle + generation and do proper check
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Instance *ptr = (Instance *)(instance);
	return ptr->enumerateDevices(ptr, device_count, infos);
}

/*
 */
Opal_Result opalCreateDevice(Opal_Instance instance, uint32_t index, Opal_Device *device)
{
	// FIXME: change to handle + generation and do proper check
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Instance *ptr = (Instance *)(instance);
	return ptr->createDevice(ptr, index, device);
}

Opal_Result opalCreateDefaultDevice(Opal_Instance instance, Opal_DeviceHint hint, Opal_Device *device)
{
	// FIXME: change to handle + generation and do proper check
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Instance *ptr = (Instance *)(instance);
	return ptr->createDefaultDevice(ptr, hint, device);
}

Opal_Result opalDestroyDevice(Opal_Device device)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	Opal_Result result = ptr->destroy(ptr);

	free(ptr);
	return result;
}

/*
 */
Opal_Result opalGetDeviceInfo(Opal_Device device, Opal_DeviceInfo *info)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->getInfo(ptr, info);
}

/*
 */
Opal_Result opalCreateBuffer(Opal_Device device, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->createBuffer(ptr, desc, buffer);
}

Opal_Result opalCreateTexture(Opal_Device device, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->createTexture(ptr, desc, texture);
}

Opal_Result opalCreateTextureView(Opal_Device device, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->createTextureView(ptr, desc, texture_view);
}

Opal_Result opalCreateSampler(Opal_Device device, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCreateCommandBuffer(Opal_Device device, Opal_CommandBuffer *command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCreateShader(Opal_Device device, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCreateBindSetLayout(Opal_Device device, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCreateBindSet(Opal_Device device, Opal_BindsetLayout bindset_layout, uint32_t num_bindings, const Opal_BindsetBinding *bindings, Opal_Bindset *bindset)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCreatePipelineLayout(Opal_Device device, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCreateGraphicsPipeline(Opal_Device device, const Opal_GraphicsPipelineDesc *desc, Opal_GraphicsPipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCreateMeshletPipeline(Opal_Device device, const Opal_MeshletPipelineDesc *desc, Opal_MeshletPipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCreateComputePipeline(Opal_Device device, const Opal_ComputePipelineDesc *desc, Opal_ComputePipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCreateRaytracePipeline(Opal_Device device, const Opal_RaytracePipelineDesc *desc, Opal_RaytracePipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCreateSwapChain(Opal_Device device, const void *handle, Opal_SwapChain *swap_chain)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result opalDestroyBuffer(Opal_Device device, Opal_Buffer buffer)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->destroyBuffer(ptr, buffer);
}

Opal_Result opalDestroyTexture(Opal_Device device, Opal_Texture texture)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->destroyTexture(ptr, texture);
}

Opal_Result opalDestroyTextureView(Opal_Device device, Opal_TextureView texture_view)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->destroyTextureView(ptr, texture_view);
}

Opal_Result opalDestroySampler(Opal_Device device, Opal_Sampler sampler)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalDestroyCommandBuffer(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalDestroyShader(Opal_Device device, Opal_Shader shader)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalDestroyBindSetLayout(Opal_Device device, Opal_BindsetLayout bindset_layout)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalDestroyBindSet(Opal_Device device, Opal_Bindset bindset)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalDestroyPipelineLayout(Opal_Device device, Opal_PipelineLayout pipeline_layout)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalDestroyGraphicsPipeline(Opal_Device device, Opal_GraphicsPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalDestroyMeshletPipeline(Opal_Device device, Opal_MeshletPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalDestroyComputePipeline(Opal_Device device, Opal_ComputePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalDestroyRaytracePipeline(Opal_Device device, Opal_RaytracePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalDestroySwapChain(Opal_Device device, Opal_SwapChain swap_chain)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result opalMapBuffer(Opal_Device device, Opal_Buffer buffer, void **ptr)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;
	
	if (buffer == OPAL_NULL_HANDLE)
		return OPAL_INVALID_BUFFER;

	Device *device_ptr = (Device *)(device);
	return device_ptr->mapBuffer(device_ptr, buffer, ptr);
}

Opal_Result opalUnmapBuffer(Opal_Device device, Opal_Buffer buffer)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;
	
	if (buffer == OPAL_NULL_HANDLE)
		return OPAL_INVALID_BUFFER;

	Device *ptr = (Device *)(device);
	return ptr->unmapBuffer(ptr, buffer);
}

Opal_Result opalDeviceWaitIdle(Opal_Device device)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result opalUpdateBindSet(Opal_Device device, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result opalBeginCommands(Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalEndCommands(Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalSubmitCommands(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalAcquire(Opal_SwapChain swap_chain, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalPresent(Opal_SwapChain swap_chain)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result opalCmdBeginAsyncTransferPass(Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdEndAsyncTransferPass(Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdBeginAsyncComputePass(Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdEndAsyncComputePass(Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdBeginGraphicsPass(Opal_CommandBuffer command_buffer, uint32_t num_attachments, const Opal_FramebufferAttachment *attachments)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdEndGraphicsPass(Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdSetGraphicsPipeline(Opal_CommandBuffer command_buffer, Opal_GraphicsPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdSetMeshletPipeline(Opal_CommandBuffer command_buffer, Opal_MeshletPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdSetComputePipeline(Opal_CommandBuffer command_buffer, Opal_ComputePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdSetRaytracePipeline(Opal_CommandBuffer command_buffer, Opal_RaytracePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdSetBindsets(Opal_CommandBuffer command_buffer, uint32_t num_bindsets, const Opal_Bindset *bindsets)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdSetVertexBuffers(Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdSetIndexBuffer(Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdDrawIndexedInstanced(Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t base_index, uint32_t num_instances, uint32_t base_instance)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdMeshletDispatch(Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdComputeDispatch(Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdRaytraceDispatch(Opal_CommandBuffer command_buffer, Opal_ShaderBindingTableView shader_table, uint32_t width, uint32_t height, uint32_t depth)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdCopyBufferToBuffer(Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdCopyBufferToTexture(Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_TextureRegion dst)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result opalCmdCopyTextureToBuffer(Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferView dst)
{
	return OPAL_NOT_SUPPORTED;
}
