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
Opal_Result opalCreateSurface(Opal_Instance instance, void *handle, Opal_Surface *surface)
{
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Opal_InstanceInternal *ptr = (Opal_InstanceInternal *)instance;
	assert(ptr->vtbl);
	assert(ptr->vtbl->createSurface);

	return ptr->vtbl->createSurface(instance, handle, surface);
}

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

/*
 */
Opal_Result opalDestroySurface(Opal_Instance instance, Opal_Surface surface)
{
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Opal_InstanceInternal *ptr = (Opal_InstanceInternal *)instance;
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroySurface);

	return ptr->vtbl->destroySurface(instance, surface);
}

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

Opal_Result opalGetAccelerationStructurePrebuildInfo(Opal_Device device, const Opal_AccelerationStructureBuildDesc *desc, Opal_AccelerationStructurePrebuildInfo *info)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->getAccelerationStructurePrebuildInfo);

	return ptr->vtbl->getAccelerationStructurePrebuildInfo(device, desc, info);
}

Opal_Result opalGetShaderBindingTablePrebuildInfo(Opal_Device device, const Opal_ShaderBindingTableLayoutDesc *desc, Opal_ShaderBindingTablePrebuildInfo *info)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->getShaderBindingTablePrebuildInfo);

	return ptr->vtbl->getShaderBindingTablePrebuildInfo(device, desc, info);
}

/*
 */
Opal_Result opalCreateSemaphore(Opal_Device device, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createSemaphore);

	return ptr->vtbl->createSemaphore(device, desc, semaphore);
}

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

Opal_Result opalCreateAccelerationStructure(Opal_Device device, const Opal_AccelerationStructureDesc *desc, Opal_AccelerationStructure *acceleration_structure)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createAccelerationStructure);

	return ptr->vtbl->createAccelerationStructure(device, desc, acceleration_structure);
}

Opal_Result opalCreateCommandPool(Opal_Device device, Opal_Queue queue, Opal_CommandPool *command_pool)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createCommandPool);

	return ptr->vtbl->createCommandPool(device, queue, command_pool);
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

Opal_Result opalCreateBindsetPool(Opal_Device device, Opal_BindsetLayout bindset_layout, uint32_t max_bindsets, Opal_BindsetPool *bindset_pool)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createBindsetPool);

	return ptr->vtbl->createBindsetPool(device, bindset_layout, max_bindsets, bindset_pool);
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

Opal_Result opalCreateGraphicsPipeline(Opal_Device device, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createGraphicsPipeline);

	return ptr->vtbl->createGraphicsPipeline(device, desc, pipeline);
}

Opal_Result opalCreateMeshletPipeline(Opal_Device device, const Opal_MeshletPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createMeshletPipeline);

	return ptr->vtbl->createMeshletPipeline(device, desc, pipeline);
}

Opal_Result opalCreateComputePipeline(Opal_Device device, const Opal_ComputePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createComputePipeline);

	return ptr->vtbl->createComputePipeline(device, desc, pipeline);
}

Opal_Result opalCreateRaytracePipeline(Opal_Device device, const Opal_RaytracePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createRaytracePipeline);

	return ptr->vtbl->createRaytracePipeline(device, desc, pipeline);
}

Opal_Result opalCreateSwapchain(Opal_Device device, const Opal_SwapchainDesc *desc, Opal_Swapchain *swapchain)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->createSwapchain);

	return ptr->vtbl->createSwapchain(device, desc, swapchain);
}

/*
 */
Opal_Result opalDestroySemaphore(Opal_Device device, Opal_Semaphore semaphore)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroySemaphore);

	return ptr->vtbl->destroySemaphore(device, semaphore);
}

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

Opal_Result opalDestroyAccelerationStructure(Opal_Device device, Opal_AccelerationStructure acceleration_structure)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyAccelerationStructure);

	return ptr->vtbl->destroyAccelerationStructure(device, acceleration_structure);
}

Opal_Result opalDestroyCommandPool(Opal_Device device, Opal_CommandPool command_pool)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyCommandPool);

	return ptr->vtbl->destroyCommandPool(device, command_pool);
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

Opal_Result opalDestroyBindsetPool(Opal_Device device, Opal_BindsetPool bindset_pool)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyBindsetPool);

	return ptr->vtbl->destroyBindsetPool(device, bindset_pool);
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

Opal_Result opalDestroyPipeline(Opal_Device device, Opal_Pipeline pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroyPipeline);

	return ptr->vtbl->destroyPipeline(device, pipeline);
}

Opal_Result opalDestroySwapchain(Opal_Device device, Opal_Swapchain swapchain)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->destroySwapchain);

	return ptr->vtbl->destroySwapchain(device, swapchain);
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
Opal_Result opalBuildShaderBindingTable(Opal_Device device, const Opal_ShaderBindingTableBuildDesc *desc)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->buildShaderBindingTable);

	return ptr->vtbl->buildShaderBindingTable(device, desc);
}

Opal_Result opalAllocateCommandBuffer(Opal_Device device, Opal_CommandPool command_pool, Opal_CommandBuffer *command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->allocateCommandBuffer);

	return ptr->vtbl->allocateCommandBuffer(device, command_pool, command_buffer);
}

Opal_Result opalFreeCommandBuffer(Opal_Device device, Opal_CommandPool command_pool, Opal_CommandBuffer command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->freeCommandBuffer);

	return ptr->vtbl->freeCommandBuffer(device, command_pool, command_buffer);
}

Opal_Result opalResetCommandPool(Opal_Device device, Opal_CommandPool command_pool)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->resetCommandPool);

	return ptr->vtbl->resetCommandPool(device, command_pool);
}

Opal_Result opalResetCommandBuffer(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->resetCommandBuffer);

	return ptr->vtbl->resetCommandBuffer(device, command_buffer);
}

Opal_Result opalAllocateBindset(Opal_Device device, Opal_BindsetPool bindset_pool, Opal_Bindset *bindset)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->allocateBindset);

	return ptr->vtbl->allocateBindset(device, bindset_pool, bindset);
}

Opal_Result opalFreeBindset(Opal_Device device, Opal_BindsetPool bindset_pool, Opal_Bindset bindset)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->freeBindset);

	return ptr->vtbl->freeBindset(device, bindset_pool, bindset);
}

Opal_Result opalResetBindsetPool(Opal_Device device, Opal_BindsetPool bindset_pool)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->resetBindsetPool);

	return ptr->vtbl->resetBindsetPool(device, bindset_pool);
}

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

Opal_Result opalUpdateBindset(Opal_Device device, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->updateBindset);

	return ptr->vtbl->updateBindset(device, bindset, num_bindings, bindings);
}

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

Opal_Result opalQuerySemaphore(Opal_Device device, Opal_Semaphore semaphore, uint64_t *value)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->querySemaphore);

	return ptr->vtbl->querySemaphore(device, semaphore, value);
}

Opal_Result opalSignalSemaphore(Opal_Device device, Opal_Semaphore semaphore, uint64_t value)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->signalSemaphore);

	return ptr->vtbl->signalSemaphore(device, semaphore, value);
}

Opal_Result opalWaitSemaphore(Opal_Device device, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->waitSemaphore);

	return ptr->vtbl->waitSemaphore(device, semaphore, value, timeout_milliseconds);
}

Opal_Result opalWaitQueue(Opal_Device device, Opal_Queue queue)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->waitQueue);

	return ptr->vtbl->waitQueue(device, queue);
}

Opal_Result opalWaitIdle(Opal_Device device)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->waitIdle);

	return ptr->vtbl->waitIdle(device);
}

Opal_Result opalSubmit(Opal_Device device, Opal_Queue queue, const Opal_SubmitDesc *desc)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->submit);

	return ptr->vtbl->submit(device, queue, desc);
}

Opal_Result opalAcquire(Opal_Device device, Opal_Swapchain swapchain, Opal_TextureView *texture_view)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->acquire);

	return ptr->vtbl->acquire(device, swapchain, texture_view);
}

Opal_Result opalPresent(Opal_Device device, Opal_Swapchain swapchain)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->present);

	return ptr->vtbl->present(device, swapchain);
}

/*
 */
Opal_Result opalCmdBeginGraphicsPass(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depthstencil_attachment)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdBeginGraphicsPass);

	return ptr->vtbl->cmdBeginGraphicsPass(device, command_buffer, num_color_attachments, color_attachments, depthstencil_attachment);
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

Opal_Result opalCmdBeginComputePass(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdBeginComputePass);

	return ptr->vtbl->cmdBeginComputePass(device, command_buffer);
}

Opal_Result opalCmdEndComputePass(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdEndComputePass);

	return ptr->vtbl->cmdEndComputePass(device, command_buffer);
}

Opal_Result opalCmdBeginRaytracePass(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdBeginRaytracePass);

	return ptr->vtbl->cmdBeginRaytracePass(device, command_buffer);
}

Opal_Result opalCmdEndRaytracePass(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdEndRaytracePass);

	return ptr->vtbl->cmdEndRaytracePass(device, command_buffer);
}

Opal_Result opalCmdSetPipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_Pipeline pipeline)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetPipeline);

	return ptr->vtbl->cmdSetPipeline(device, command_buffer, pipeline);
}

Opal_Result opalCmdSetBindsets(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout, uint32_t num_bindsets, const Opal_Bindset *bindsets)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetBindsets);

	return ptr->vtbl->cmdSetBindsets(device, command_buffer, pipeline_layout, num_bindsets, bindsets);
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

Opal_Result opalCmdSetIndexBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer, Opal_IndexFormat index_format)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetIndexBuffer);

	return ptr->vtbl->cmdSetIndexBuffer(device, command_buffer, index_buffer, index_format);
}

Opal_Result opalCmdSetViewport(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_Viewport viewport)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetViewport);

	return ptr->vtbl->cmdSetViewport(device, command_buffer, viewport);
}

Opal_Result opalCmdSetScissor(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdSetScissor);

	return ptr->vtbl->cmdSetScissor(device, command_buffer, x, y, width, height);
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

Opal_Result opalCmdRaytraceDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTableEntry raygen_entry, Opal_ShaderBindingTableEntry hitgroup_entry, Opal_ShaderBindingTableEntry miss_entry, uint32_t width, uint32_t height, uint32_t depth)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdRaytraceDispatch);

	return ptr->vtbl->cmdRaytraceDispatch(device, command_buffer, raygen_entry, hitgroup_entry, miss_entry, width, height, depth);
}

Opal_Result opalCmdBuildAccelerationStructures(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_build_descs, const Opal_AccelerationStructureBuildDesc *descs)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdBuildAccelerationStructures);

	return ptr->vtbl->cmdBuildAccelerationStructures(device, command_buffer, num_build_descs, descs);
}

Opal_Result opalCmdCopyAccelerationStructure(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_AccelerationStructure src, Opal_AccelerationStructure dst, Opal_AccelerationStructureCopyMode mode)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdCopyAccelerationStructure);

	return ptr->vtbl->cmdCopyAccelerationStructure(device, command_buffer, src, dst, mode);
}

Opal_Result opalCmdCopyAccelerationStructuresPostbuildInfo(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_src_acceleration_structures, const Opal_AccelerationStructure *src_acceleration_structures, Opal_BufferView dst_buffer)
{
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Opal_DeviceInternal *ptr = (Opal_DeviceInternal *)(device);
	assert(ptr->vtbl);
	assert(ptr->vtbl->cmdCopyAccelerationStructuresPostbuildInfo);

	return ptr->vtbl->cmdCopyAccelerationStructuresPostbuildInfo(device, command_buffer, num_src_acceleration_structures, src_acceleration_structures, dst_buffer);
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
