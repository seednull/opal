#include "webgpu_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Opal_DeviceTable device_vtbl =
{
	webgpu_deviceDestroy,
	webgpu_deviceGetInfo,
	webgpu_deviceGetQueue,

	webgpu_deviceCreateBuffer,
	webgpu_deviceCreateTexture,
	webgpu_deviceCreateTextureView,
	NULL, // PFN_opalCreateSampler createSampler
	NULL, // PFN_opalCreateCommandBuffer createCommandBuffer
	NULL, // PFN_opalCreateShader createShader
	NULL, // PFN_opalCreateBindsetLayout createBindsetLayout
	NULL, // PFN_opalCreateBindset createBindset
	NULL, // PFN_opalCreatePipelineLayout createPipelineLayout
	NULL, // PFN_opalCreateGraphicsPipeline createGraphicsPipeline
	NULL, // PFN_opalCreateMeshletPipeline createMeshletPipeline
	NULL, // PFN_opalCreateComputePipeline createComputePipeline
	NULL, // PFN_opalCreateRaytracePipeline createRaytracePipeline
	NULL, // PFN_opalCreateSwapChain createSwapChain

	webgpu_deviceDestroyBuffer,
	webgpu_deviceDestroyTexture,
	webgpu_deviceDestroyTextureView,
	NULL, // PFN_opalDestroySampler destroySampler
	NULL, // PFN_opalDestroyCommandBuffer destroyCommandBuffer
	NULL, // PFN_opalDestroyShader destroyShader
	NULL, // PFN_opalDestroyBindsetLayout destroyBindsetLayout
	NULL, // PFN_opalDestroyBindset destroyBindset
	NULL, // PFN_opalDestroyPipelineLayout destroyPipelineLayout
	NULL, // PFN_opalDestroyGraphicsPipeline destroyGraphicsPipeline
	NULL, // PFN_opalDestroyMeshletPipeline destroyMeshletPipeline
	NULL, // PFN_opalDestroyComputePipeline destroyComputePipeline
	NULL, // PFN_opalDestroyRaytracePipeline destroyRaytracePipeline
	NULL, // PFN_opalDestroySwapChain destroySwapChain

	webgpu_deviceMapBuffer,
	webgpu_deviceUnmapBuffer,
	NULL, // PFN_opalDeviceWaitIdle deviceWaitIdle

	NULL, // PFN_opalUpdateBindset updateBindset

	NULL, // PFN_opalBeginCommandBuffer beginCommandBuffer
	NULL, // PFN_opalEndCommandBuffer endCommandBuffer
	NULL, // PFN_opalWaitCommandBuffers waitCommandBuffers

	NULL, // PFN_opalSubmit submit
	NULL, // PFN_opalAcquire acquire
	NULL, // PFN_opalPresent present

	NULL, // PFN_opalCmdBeginGraphicsPass cmdBeginGraphicsPass
	NULL, // PFN_opalCmdEndGraphicsPass cmdEndGraphicsPass
	NULL, // PFN_opalCmdSetGraphicsPipeline cmdSetGraphicsPipeline
	NULL, // PFN_opalCmdSetMeshletPipeline cmdSetMeshletPipeline
	NULL, // PFN_opalCmdSetComputePipeline cmdSetComputePipeline
	NULL, // PFN_opalCmdSetRaytracePipeline cmdSetRaytracePipeline
	NULL, // PFN_opalCmdSetBindsets cmdSetBindsets
	NULL, // PFN_opalCmdSetVertexBuffers cmdSetVertexBuffers
	NULL, // PFN_opalCmdSetIndexBuffer cmdSetIndexBuffer
	NULL, // PFN_opalCmdDrawIndexedInstanced cmdDrawIndexedInstanced
	NULL, // PFN_opalCmdMeshletDispatch cmdMeshletDispatch
	NULL, // PFN_opalCmdComputeDispatch cmdComputeDispatch
	NULL, // PFN_opalCmdRaytraceDispatch cmdRaytraceDispatch
	NULL, // PFN_opalCmdCopyBufferToBuffer cmdCopyBufferToBuffer
	NULL, // PFN_opalCmdCopyBufferToTexture cmdCopyBufferToTexture
	NULL, // PFN_opalCmdCopyTextureToBuffer cmdCopyTextureToBuffer
	NULL, // PFN_opalCmdBufferTransitionBarrier cmdBufferTransitionBarrier
	NULL, // PFN_opalCmdBufferQueueGrabBarrier cmdBufferQueueGrabBarrier
	NULL, // PFN_opalCmdBufferQueueReleaseBarrier cmdBufferQueueReleaseBarrier
	NULL, // PFN_opalCmdTextureTransitionBarrier cmdTextureTransitionBarrier
	NULL, // PFN_opalCmdTextureQueueGrabBarrier cmdTextureQueueGrabBarrier
	NULL, // PFN_opalCmdTextureQueueReleaseBarrier cmdTextureQueueReleaseBarrier
};

/*
 */
Opal_Result webgpu_deviceInitialize(WebGPU_Device *device_ptr, WebGPU_Instance *instance_ptr, WGPUAdapter adapter, WGPUDevice device)
{
	assert(device_ptr);
	assert(instance_ptr);
	assert(adapter);
	assert(device);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	device_ptr->adapter = adapter;
	device_ptr->device = device;

	return OPAL_SUCCESS;
}

Opal_Result webgpu_deviceGetInfo(Opal_Device this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	WebGPU_Device *ptr = (WebGPU_Device *)this;
	return webgpu_fillDeviceInfo(ptr->adapter, info);
}

Opal_Result webgpu_deviceGetQueue(Opal_Device this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	assert(this);
	assert(queue);
	assert(engine_type < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result webgpu_deviceDestroy(Opal_Device this)
{
	assert(this);

	WebGPU_Device *ptr = (WebGPU_Device *)this;
	wgpuAdapterRelease(ptr->adapter);
	wgpuDeviceRelease(ptr->device);

	free(ptr);
	return OPAL_SUCCESS;
}

/*
 */
Opal_Result webgpu_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result webgpu_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result webgpu_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_deviceDestroyTexture(Opal_Device this, Opal_Texture texture)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	return OPAL_NOT_SUPPORTED;
}
