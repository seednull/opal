#include "metal_internal.h"

/*
 */
static Opal_Result metal_deviceGetInfo(Opal_Device this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	Metal_Device *ptr = (Metal_Device *)this;
	return metal_fillDeviceInfo(ptr->device, info);
}

static Opal_Result metal_deviceGetQueue(Opal_Device this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	assert(this);
	assert(queue);
	assert(engine_type < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroy(Opal_Device this)
{
	assert(this);

	Metal_Device *ptr = (Metal_Device *)this;
	// [ptr->device release]; // FIXME: do we need to manually release it here?

	return OPAL_SUCCESS;
}

static Opal_Result metal_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateSampler(Opal_Device this, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateCommandBuffer(Opal_Device this, Opal_DeviceEngineType engine_type, Opal_CommandBuffer *command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateShader(Opal_Device this, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateBindsetLayout(Opal_Device this, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateBindset(Opal_Device this, Opal_BindsetLayout bindset_layout, uint32_t num_bindings, const Opal_BindsetBinding *bindings, Opal_Bindset *bindset)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_GraphicsPipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateMeshletPipeline(Opal_Device this, const Opal_MeshletPipelineDesc *desc, Opal_MeshletPipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateComputePipeline(Opal_Device this, const Opal_ComputePipelineDesc *desc, Opal_ComputePipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateRaytracePipeline(Opal_Device this, const Opal_RaytracePipelineDesc *desc, Opal_RaytracePipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCreateSwapChain(Opal_Device this, const void *handle, Opal_SwapChain *swap_chain)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyTexture(Opal_Device this, Opal_Texture texture)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroySampler(Opal_Device this, Opal_Sampler sampler)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyBindsetLayout(Opal_Device this, Opal_BindsetLayout bindset_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyBindset(Opal_Device this, Opal_Bindset bindset)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyGraphicsPipeline(Opal_Device this, Opal_GraphicsPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyMeshletPipeline(Opal_Device this, Opal_MeshletPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyComputePipeline(Opal_Device this, Opal_ComputePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroyRaytracePipeline(Opal_Device this, Opal_RaytracePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceDestroySwapChain(Opal_Device this, Opal_SwapChain swap_chain)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceUpdateBindset(Opal_Device this, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceBeginCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceWaitCommandBuffers(Opal_Device this, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceWaitIdle(Opal_Device this)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceSubmit(Opal_Device this, Opal_Queue queue, uint32_t num_command_buffer, const Opal_CommandBuffer *command_buffers, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceAcquire(Opal_Device this, Opal_SwapChain swap_chain, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_devicePresent(Opal_Device this, Opal_SwapChain swap_chain, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_attachments, const Opal_FramebufferAttachment *attachments)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetGraphicsPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_GraphicsPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetMeshletPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_MeshletPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetComputePipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_ComputePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetRaytracePipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_RaytracePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetBindsets(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_bindsets, const Opal_Bindset *bindsets)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdDrawIndexedInstanced(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t base_index, uint32_t num_instances, uint32_t base_instance)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdMeshletDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdComputeDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdRaytraceDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTableView shader_table, uint32_t width, uint32_t height, uint32_t depth)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdCopyBufferToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_TextureRegion dst)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdCopyTextureToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferView dst)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdBufferTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdBufferQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdBufferQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdTextureTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdTextureQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result metal_deviceCmdTextureQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
static Opal_DeviceTable device_vtbl =
{
	metal_deviceDestroy,
	metal_deviceGetInfo,
	metal_deviceGetQueue,

	metal_deviceCreateBuffer,
	metal_deviceCreateTexture,
	metal_deviceCreateTextureView,
	metal_deviceCreateSampler,
	metal_deviceCreateCommandBuffer,
	metal_deviceCreateShader,
	metal_deviceCreateBindsetLayout,
	metal_deviceCreateBindset,
	metal_deviceCreatePipelineLayout,
	metal_deviceCreateGraphicsPipeline,
	metal_deviceCreateMeshletPipeline,
	metal_deviceCreateComputePipeline,
	metal_deviceCreateRaytracePipeline,
	metal_deviceCreateSwapChain,

	metal_deviceDestroyBuffer,
	metal_deviceDestroyTexture,
	metal_deviceDestroyTextureView,
	metal_deviceDestroySampler,
	metal_deviceDestroyCommandBuffer,
	metal_deviceDestroyShader,
	metal_deviceDestroyBindsetLayout,
	metal_deviceDestroyBindset,
	metal_deviceDestroyPipelineLayout,
	metal_deviceDestroyGraphicsPipeline,
	metal_deviceDestroyMeshletPipeline,
	metal_deviceDestroyComputePipeline,
	metal_deviceDestroyRaytracePipeline,
	metal_deviceDestroySwapChain,

	metal_deviceMapBuffer,
	metal_deviceUnmapBuffer,
	metal_deviceUpdateBindset,
	metal_deviceBeginCommandBuffer,
	metal_deviceEndCommandBuffer,
	metal_deviceWaitCommandBuffers,
	metal_deviceWaitIdle,
	metal_deviceSubmit,
	metal_deviceAcquire,
	metal_devicePresent,

	metal_deviceCmdBeginGraphicsPass,
	metal_deviceCmdEndGraphicsPass,
	metal_deviceCmdSetGraphicsPipeline,
	metal_deviceCmdSetMeshletPipeline,
	metal_deviceCmdSetComputePipeline,
	metal_deviceCmdSetRaytracePipeline,
	metal_deviceCmdSetBindsets,
	metal_deviceCmdSetVertexBuffers,
	metal_deviceCmdSetIndexBuffer,
	metal_deviceCmdDrawIndexedInstanced,
	metal_deviceCmdMeshletDispatch,
	metal_deviceCmdComputeDispatch,
	metal_deviceCmdRaytraceDispatch,
	metal_deviceCmdCopyBufferToBuffer,
	metal_deviceCmdCopyBufferToTexture,
	metal_deviceCmdCopyTextureToBuffer,
	metal_deviceCmdBufferTransitionBarrier,
	metal_deviceCmdBufferQueueGrabBarrier,
	metal_deviceCmdBufferQueueReleaseBarrier,
	metal_deviceCmdTextureTransitionBarrier,
	metal_deviceCmdTextureQueueGrabBarrier,
	metal_deviceCmdTextureQueueReleaseBarrier,
};

/*
 */
Opal_Result metal_deviceInitialize(Metal_Device *device_ptr, Metal_Instance *instance_ptr, id<MTLDevice> device);
{
	assert(device_ptr);
	assert(instance_ptr);
	assert(device);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	device_ptr->device = device;

	return OPAL_SUCCESS;
}
