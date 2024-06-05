#include "opal_internal.h"

#include <assert.h>
#include <string.h>

/*
 */
typedef struct Opal_InstanceInternal_t
{
	Opal_InstanceTable *vtbl;
} Opal_InstanceInternal;

typedef struct Opal_DeviceInternal_t
{
	Opal_DeviceTable *vtbl;
} Opal_DeviceInternal;

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

Opal_Result opalGetInstanceTable(Opal_Instance instance, Opal_InstanceTable *instance_table)
{
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	if (instance_table == NULL)
		return OPAL_INVALID_OUTPUT_ARGUMENT;

	Opal_InstanceInternal *ptr = (Opal_InstanceInternal *)instance;
	assert(ptr->vtbl);

	memcpy(instance_table, ptr->vtbl, sizeof(Opal_InstanceTable));
	return OPAL_SUCCESS;
}

Opal_Result opalGetDeviceTable(Opal_Device device, Opal_DeviceTable *device_table)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	if (device_table == NULL)
		return OPAL_INVALID_OUTPUT_ARGUMENT;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)device;
	assert(ptr->vtbl);

	memcpy(device_table, ptr->vtbl, sizeof(Opal_DeviceTable));
	return OPAL_SUCCESS;
}

/*
 */
Opal_Result opalDestroyInstance(Opal_Instance instance)
{
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Opal_InstanceInternal *ptr = (Opal_InstanceInternal *)instance;
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyInstance);

	return ptr->vtbl->destroyInstance(instance);
}

/*
 */
Opal_Result opalEnumerateDevices(Opal_Instance instance, uint32_t *device_count, Opal_DeviceInfo *infos)
{
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Opal_InstanceInternal *ptr = (Opal_InstanceInternal *)instance;
	assert(ptr->vtbl);
	assert(ptr->vtbl->enumerateDevices);

	return ptr->vtbl->enumerateDevices(instance, device_count, infos);
}

/*
 */
Opal_Result opalCreateDevice(Opal_Instance instance, uint32_t index, Opal_Device *device)
{
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Opal_InstanceInternal *ptr = (Opal_InstanceInternal *)instance;
	assert(ptr->vtbl);
	assert(ptr->vtbl->createDevice);

	return ptr->vtbl->createDevice(instance, index, device);
}

Opal_Result opalCreateDefaultDevice(Opal_Instance instance, Opal_DeviceHint hint, Opal_Device *device)
{
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Opal_InstanceInternal *ptr = (Opal_InstanceInternal *)instance;
	assert(ptr->vtbl);
	assert(ptr->vtbl->createDefaultDevice);

	return ptr->vtbl->createDefaultDevice(instance, hint, device);
}

Opal_Result opalDestroyDevice(Opal_Device device)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyDevice);

	return ptr->vtbl->destroyDevice(device);
}

/*
 */
Opal_Result opalGetDeviceInfo(Opal_Device device, Opal_DeviceInfo *info)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->getDeviceInfo);

	return ptr->vtbl->getDeviceInfo(device, info);
}

Opal_Result opalGetDeviceQueue(Opal_Device device, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->getDeviceQueue);

	return ptr->vtbl->getDeviceQueue(device, engine_type, index, queue);
}

/*
 */
Opal_Result opalCreateBuffer(Opal_Device device, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createBuffer);

	return ptr->vtbl->createBuffer(device, desc, buffer);
}

Opal_Result opalCreateTexture(Opal_Device device, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createTexture);

	return ptr->vtbl->createTexture(device, desc, texture);
}

Opal_Result opalCreateTextureView(Opal_Device device, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createTextureView);

	return ptr->vtbl->createTextureView(device, desc, texture_view);
}

Opal_Result opalCreateSampler(Opal_Device device, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createSampler);

	return ptr->vtbl->createSampler(device, desc, sampler);
}

Opal_Result opalCreateCommandBuffer(Opal_Device device, Opal_DeviceEngineType engine_type, Opal_CommandBuffer *command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createCommandBuffer);

	return ptr->vtbl->createCommandBuffer(device, engine_type, command_buffer);
}

Opal_Result opalCreateShader(Opal_Device device, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createShader);

	return ptr->vtbl->createShader(device, desc, shader);
}

Opal_Result opalCreateBindsetLayout(Opal_Device device, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createBindsetLayout);

	return ptr->vtbl->createBindsetLayout(device, num_bindings, bindings, bindset_layout);
}

Opal_Result opalCreateBindset(Opal_Device device, Opal_BindsetLayout bindset_layout, uint32_t num_bindings, const Opal_BindsetBinding *bindings, Opal_Bindset *bindset)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createBindset);

	return ptr->vtbl->createBindset(device, bindset_layout, num_bindings, bindings, bindset);
}

Opal_Result opalCreatePipelineLayout(Opal_Device device, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createPipelineLayout);

	return ptr->vtbl->createPipelineLayout(device, num_bindset_layouts, bindset_layouts, pipeline_layout);
}

Opal_Result opalCreateGraphicsPipeline(Opal_Device device, const Opal_GraphicsPipelineDesc *desc, Opal_GraphicsPipeline *pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createGraphicsPipeline);

	return ptr->vtbl->createGraphicsPipeline(device, desc, pipeline);
}

Opal_Result opalCreateMeshletPipeline(Opal_Device device, const Opal_MeshletPipelineDesc *desc, Opal_MeshletPipeline *pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createMeshletPipeline);

	return ptr->vtbl->createMeshletPipeline(device, desc, pipeline);
}

Opal_Result opalCreateComputePipeline(Opal_Device device, const Opal_ComputePipelineDesc *desc, Opal_ComputePipeline *pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createComputePipeline);

	return ptr->vtbl->createComputePipeline(device, desc, pipeline);
}

Opal_Result opalCreateRaytracePipeline(Opal_Device device, const Opal_RaytracePipelineDesc *desc, Opal_RaytracePipeline *pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createRaytracePipeline);

	return ptr->vtbl->createRaytracePipeline(device, desc, pipeline);
}

Opal_Result opalCreateSwapChain(Opal_Device device, const void *handle, Opal_SwapChain *swap_chain)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createSwapChain);

	return ptr->vtbl->createSwapChain(device, handle, swap_chain);
}

/*
 */
Opal_Result opalDestroyBuffer(Opal_Device device, Opal_Buffer buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyBuffer);

	return ptr->vtbl->destroyBuffer(device, buffer);
}

Opal_Result opalDestroyTexture(Opal_Device device, Opal_Texture texture)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyTexture);

	return ptr->vtbl->destroyTexture(device, texture);
}

Opal_Result opalDestroyTextureView(Opal_Device device, Opal_TextureView texture_view)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyTextureView);

	return ptr->vtbl->destroyTextureView(device, texture_view);
}

Opal_Result opalDestroySampler(Opal_Device device, Opal_Sampler sampler)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroySampler);

	return ptr->vtbl->destroySampler(device, sampler);
}

Opal_Result opalDestroyCommandBuffer(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyCommandBuffer);

	return ptr->vtbl->destroyCommandBuffer(device, command_buffer);
}

Opal_Result opalDestroyShader(Opal_Device device, Opal_Shader shader)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyShader);

	return ptr->vtbl->destroyShader(device, shader);
}

Opal_Result opalDestroyBindsetLayout(Opal_Device device, Opal_BindsetLayout bindset_layout)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyBindsetLayout);

	return ptr->vtbl->destroyBindsetLayout(device, bindset_layout);
}

Opal_Result opalDestroyBindset(Opal_Device device, Opal_Bindset bindset)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyBindset);

	return ptr->vtbl->destroyBindset(device, bindset);
}

Opal_Result opalDestroyPipelineLayout(Opal_Device device, Opal_PipelineLayout pipeline_layout)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyPipelineLayout);

	return ptr->vtbl->destroyPipelineLayout(device, pipeline_layout);
}

Opal_Result opalDestroyGraphicsPipeline(Opal_Device device, Opal_GraphicsPipeline pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyGraphicsPipeline);

	return ptr->vtbl->destroyGraphicsPipeline(device, pipeline);
}

Opal_Result opalDestroyMeshletPipeline(Opal_Device device, Opal_MeshletPipeline pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyMeshletPipeline);

	return ptr->vtbl->destroyMeshletPipeline(device, pipeline);
}

Opal_Result opalDestroyComputePipeline(Opal_Device device, Opal_ComputePipeline pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyComputePipeline);

	return ptr->vtbl->destroyComputePipeline(device, pipeline);
}

Opal_Result opalDestroyRaytracePipeline(Opal_Device device, Opal_RaytracePipeline pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyRaytracePipeline);

	return ptr->vtbl->destroyRaytracePipeline(device, pipeline);
}

Opal_Result opalDestroySwapChain(Opal_Device device, Opal_SwapChain swap_chain)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroySwapChain);

	return ptr->vtbl->destroySwapChain(device, swap_chain);
}

/*
 */
Opal_Result opalMapBuffer(Opal_Device device, Opal_Buffer buffer, void **mapped_ptr)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->mapBuffer);

	return ptr->vtbl->mapBuffer(device, buffer, mapped_ptr);
}

Opal_Result opalUnmapBuffer(Opal_Device device, Opal_Buffer buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->unmapBuffer);

	return ptr->vtbl->unmapBuffer(device, buffer);
}

Opal_Result opalDeviceWaitIdle(Opal_Device device)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->deviceWaitIdle);

	return ptr->vtbl->deviceWaitIdle(device);
}

/*
 */
Opal_Result opalUpdateBindset(Opal_Device device, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->updateBindset);

	return ptr->vtbl->updateBindset(device, bindset, num_bindings, bindings);
}

/*
 */
Opal_Result opalBeginCommandBuffer(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->beginCommandBuffer);

	return ptr->vtbl->beginCommandBuffer(device, command_buffer);
}

Opal_Result opalEndCommandBuffer(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->endCommandBuffer);

	return ptr->vtbl->endCommandBuffer(device, command_buffer);
}

Opal_Result opalWaitCommandBuffers(Opal_Device device, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->waitCommandBuffers);

	return ptr->vtbl->waitCommandBuffers(device, num_wait_command_buffers, wait_command_buffers);
}

Opal_Result opalSubmit(Opal_Device device, Opal_Queue queue, uint32_t num_command_buffers, const Opal_CommandBuffer *command_buffers, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->submit);

	return ptr->vtbl->submit(device, queue, num_command_buffers, command_buffers, num_wait_command_buffers, wait_command_buffers);
}

Opal_Result opalAcquire(Opal_Device device, Opal_SwapChain swap_chain, Opal_TextureView *texture_view)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->acquire);

	return ptr->vtbl->acquire(device, swap_chain, texture_view);
}

Opal_Result opalPresent(Opal_Device device, Opal_SwapChain swap_chain, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->present);

	return ptr->vtbl->present(device, swap_chain, num_wait_command_buffers, wait_command_buffers);
}

/*
 */
Opal_Result opalCmdBeginGraphicsPass(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_attachments, const Opal_FramebufferAttachment *attachments)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdBeginGraphicsPass);

	return ptr->vtbl->cmdBeginGraphicsPass(device, command_buffer, num_attachments, attachments);
}

Opal_Result opalCmdEndGraphicsPass(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdEndGraphicsPass);

	return ptr->vtbl->cmdEndGraphicsPass(device, command_buffer);
}

Opal_Result opalCmdSetGraphicsPipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_GraphicsPipeline pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetGraphicsPipeline);

	return ptr->vtbl->cmdSetGraphicsPipeline(device, command_buffer, pipeline);
}

Opal_Result opalCmdSetMeshletPipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_MeshletPipeline pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetMeshletPipeline);

	return ptr->vtbl->cmdSetMeshletPipeline(device, command_buffer, pipeline);
}

Opal_Result opalCmdSetComputePipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ComputePipeline pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetComputePipeline);

	return ptr->vtbl->cmdSetComputePipeline(device, command_buffer, pipeline);
}

Opal_Result opalCmdSetRaytracePipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_RaytracePipeline pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetRaytracePipeline);

	return ptr->vtbl->cmdSetRaytracePipeline(device, command_buffer, pipeline);
}

Opal_Result opalCmdSetBindsets(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_bindsets, const Opal_Bindset *bindsets)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetBindsets);

	return ptr->vtbl->cmdSetBindsets(device, command_buffer, num_bindsets, bindsets);
}

Opal_Result opalCmdSetVertexBuffers(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetVertexBuffers);

	return ptr->vtbl->cmdSetVertexBuffers(device, command_buffer, num_vertex_buffers, vertex_buffers);
}

Opal_Result opalCmdSetIndexBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetIndexBuffer);

	return ptr->vtbl->cmdSetIndexBuffer(device, command_buffer, index_buffer);
}

Opal_Result opalCmdDrawIndexedInstanced(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t base_index, uint32_t num_instances, uint32_t base_instance)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdDrawIndexedInstanced);

	return ptr->vtbl->cmdDrawIndexedInstanced(device, command_buffer, num_indices, base_index, num_indices, base_instance);
}

Opal_Result opalCmdMeshletDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdMeshletDispatch);

	return ptr->vtbl->cmdMeshletDispatch(device, command_buffer, num_groups_x, num_groups_y, num_groups_z);
}

Opal_Result opalCmdComputeDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdComputeDispatch);

	return ptr->vtbl->cmdComputeDispatch(device, command_buffer, num_groups_x, num_groups_y, num_groups_z);
}

Opal_Result opalCmdRaytraceDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTableView shader_table, uint32_t width, uint32_t height, uint32_t depth)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdRaytraceDispatch);

	return ptr->vtbl->cmdRaytraceDispatch(device, command_buffer, shader_table, width, height, depth);
}

Opal_Result opalCmdCopyBufferToBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdCopyBufferToBuffer);

	return ptr->vtbl->cmdCopyBufferToBuffer(device, command_buffer, src, dst, size);
}

Opal_Result opalCmdCopyBufferToTexture(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_TextureRegion dst)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdCopyBufferToTexture);

	return ptr->vtbl->cmdCopyBufferToTexture(device, command_buffer, src, dst);
}

Opal_Result opalCmdCopyTextureToBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferView dst)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdCopyTextureToBuffer);

	return ptr->vtbl->cmdCopyTextureToBuffer(device, command_buffer, src, dst);
}

Opal_Result opalCmdBufferTransitionBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdBufferTransitionBarrier);

	return ptr->vtbl->cmdBufferTransitionBarrier(device, command_buffer, buffer, state_before, state_after);
}

Opal_Result opalCmdBufferQueueGrabBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdBufferQueueGrabBarrier);

	return ptr->vtbl->cmdBufferQueueGrabBarrier(device, command_buffer, buffer, queue);
}

Opal_Result opalCmdBufferQueueReleaseBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdBufferQueueReleaseBarrier);

	return ptr->vtbl->cmdBufferQueueReleaseBarrier(device, command_buffer, buffer, queue);
}

Opal_Result opalCmdTextureTransitionBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdTextureTransitionBarrier);

	return ptr->vtbl->cmdTextureTransitionBarrier(device, command_buffer, texture_view, state_before, state_after);
}

Opal_Result opalCmdTextureQueueGrabBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdTextureQueueGrabBarrier);

	return ptr->vtbl->cmdTextureQueueGrabBarrier(device, command_buffer, texture_view, queue);
}

Opal_Result opalCmdTextureQueueReleaseBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdTextureQueueReleaseBarrier);

	return ptr->vtbl->cmdTextureQueueReleaseBarrier(device, command_buffer, texture_view, queue);
}
