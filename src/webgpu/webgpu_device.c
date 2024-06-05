#include "webgpu_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Opal_Result webgpu_deviceGetInfo(Opal_Device this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	WebGPU_Device *ptr = (WebGPU_Device *)this;
	return webgpu_fillDeviceInfo(ptr->adapter, info);
}

static Opal_Result webgpu_deviceGetQueue(Opal_Device this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	assert(this);
	assert(queue);
	assert(engine_type < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroy(Opal_Device this)
{
	assert(this);

	WebGPU_Device *ptr = (WebGPU_Device *)this;
	wgpuAdapterRelease(ptr->adapter);
	wgpuDeviceRelease(ptr->device);

	free(ptr);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateSampler(Opal_Device this, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateCommandBuffer(Opal_Device this, Opal_DeviceEngineType engine_type, Opal_CommandBuffer *command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateShader(Opal_Device this, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateBindsetLayout(Opal_Device this, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateBindset(Opal_Device this, Opal_BindsetLayout bindset_layout, uint32_t num_bindings, const Opal_BindsetBinding *bindings, Opal_Bindset *bindset)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_GraphicsPipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateMeshletPipeline(Opal_Device this, const Opal_MeshletPipelineDesc *desc, Opal_MeshletPipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateComputePipeline(Opal_Device this, const Opal_ComputePipelineDesc *desc, Opal_ComputePipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateRaytracePipeline(Opal_Device this, const Opal_RaytracePipelineDesc *desc, Opal_RaytracePipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateSwapChain(Opal_Device this, const void *handle, Opal_SwapChain *swap_chain)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyTexture(Opal_Device this, Opal_Texture texture)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroySampler(Opal_Device this, Opal_Sampler sampler)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyBindsetLayout(Opal_Device this, Opal_BindsetLayout bindset_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyBindset(Opal_Device this, Opal_Bindset bindset)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyGraphicsPipeline(Opal_Device this, Opal_GraphicsPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyMeshletPipeline(Opal_Device this, Opal_MeshletPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyComputePipeline(Opal_Device this, Opal_ComputePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyRaytracePipeline(Opal_Device this, Opal_RaytracePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroySwapChain(Opal_Device this, Opal_SwapChain swap_chain)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceUpdateBindset(Opal_Device this, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceBeginCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceWaitCommandBuffers(Opal_Device this, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceWaitIdle(Opal_Device this)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceSubmit(Opal_Device this, Opal_Queue queue, uint32_t num_command_buffer, const Opal_CommandBuffer *command_buffers, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceAcquire(Opal_Device this, Opal_SwapChain swap_chain, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_devicePresent(Opal_Device this, Opal_SwapChain swap_chain, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_attachments, const Opal_FramebufferAttachment *attachments)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetGraphicsPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_GraphicsPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetMeshletPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_MeshletPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetComputePipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_ComputePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetRaytracePipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_RaytracePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetBindsets(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_bindsets, const Opal_Bindset *bindsets)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdDrawIndexedInstanced(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t base_index, uint32_t num_instances, uint32_t base_instance)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdMeshletDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdComputeDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdRaytraceDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTableView shader_table, uint32_t width, uint32_t height, uint32_t depth)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyBufferToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_TextureRegion dst)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyTextureToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferView dst)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBufferTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBufferQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBufferQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdTextureTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdTextureQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdTextureQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

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
	webgpu_deviceCreateSampler,
	webgpu_deviceCreateCommandBuffer,
	webgpu_deviceCreateShader,
	webgpu_deviceCreateBindsetLayout,
	webgpu_deviceCreateBindset,
	webgpu_deviceCreatePipelineLayout,
	webgpu_deviceCreateGraphicsPipeline,
	webgpu_deviceCreateMeshletPipeline,
	webgpu_deviceCreateComputePipeline,
	webgpu_deviceCreateRaytracePipeline,
	webgpu_deviceCreateSwapChain,

	webgpu_deviceDestroyBuffer,
	webgpu_deviceDestroyTexture,
	webgpu_deviceDestroyTextureView,
	webgpu_deviceDestroySampler,
	webgpu_deviceDestroyCommandBuffer,
	webgpu_deviceDestroyShader,
	webgpu_deviceDestroyBindsetLayout,
	webgpu_deviceDestroyBindset,
	webgpu_deviceDestroyPipelineLayout,
	webgpu_deviceDestroyGraphicsPipeline,
	webgpu_deviceDestroyMeshletPipeline,
	webgpu_deviceDestroyComputePipeline,
	webgpu_deviceDestroyRaytracePipeline,
	webgpu_deviceDestroySwapChain,

	webgpu_deviceMapBuffer,
	webgpu_deviceUnmapBuffer,
	webgpu_deviceUpdateBindset,
	webgpu_deviceBeginCommandBuffer,
	webgpu_deviceEndCommandBuffer,
	webgpu_deviceWaitCommandBuffers,
	webgpu_deviceWaitIdle,
	webgpu_deviceSubmit,
	webgpu_deviceAcquire,
	webgpu_devicePresent,

	webgpu_deviceCmdBeginGraphicsPass,
	webgpu_deviceCmdEndGraphicsPass,
	webgpu_deviceCmdSetGraphicsPipeline,
	webgpu_deviceCmdSetMeshletPipeline,
	webgpu_deviceCmdSetComputePipeline,
	webgpu_deviceCmdSetRaytracePipeline,
	webgpu_deviceCmdSetBindsets,
	webgpu_deviceCmdSetVertexBuffers,
	webgpu_deviceCmdSetIndexBuffer,
	webgpu_deviceCmdDrawIndexedInstanced,
	webgpu_deviceCmdMeshletDispatch,
	webgpu_deviceCmdComputeDispatch,
	webgpu_deviceCmdRaytraceDispatch,
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
