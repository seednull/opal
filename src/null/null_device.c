#include "null_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Opal_Result null_deviceGetInfo(Opal_Device this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	Null_Device *ptr = (Null_Device *)this;

	memcpy(info, &ptr->info, sizeof(Opal_DeviceInfo));
	return OPAL_SUCCESS;
}

static Opal_Result null_deviceGetQueue(Opal_Device this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	assert(this);
	assert(queue);
	assert(engine_type < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	return OPAL_NOT_SUPPORTED;
}

/*
 */
static Opal_Result null_deviceDestroy(Opal_Device this)
{
	assert(this);

	Null_Device *ptr = (Null_Device *)this;

	free(ptr);
	return OPAL_SUCCESS;
}

/*
 */
static Opal_Result null_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateSampler(Opal_Device device, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateCommandBuffer(Opal_Device device, Opal_DeviceEngineType engine_type, Opal_CommandBuffer *command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateShader(Opal_Device device, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateBindsetLayout(Opal_Device device, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateBindset(Opal_Device device, Opal_BindsetLayout bindset_layout, uint32_t num_bindings, const Opal_BindsetBinding *bindings, Opal_Bindset *bindset)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreatePipelineLayout(Opal_Device device, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateGraphicsPipeline(Opal_Device device, const Opal_GraphicsPipelineDesc *desc, Opal_GraphicsPipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateMeshletPipeline(Opal_Device device, const Opal_MeshletPipelineDesc *desc, Opal_MeshletPipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateComputePipeline(Opal_Device device, const Opal_ComputePipelineDesc *desc, Opal_ComputePipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateRaytracePipeline(Opal_Device device, const Opal_RaytracePipelineDesc *desc, Opal_RaytracePipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateSwapChain(Opal_Device device, const void *handle, Opal_SwapChain *swap_chain)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
static Opal_Result null_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyTexture(Opal_Device this, Opal_Texture texture)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_destroySampler(Opal_Device this, Opal_Sampler sampler)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_destroyCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_destroyShader(Opal_Device this, Opal_Shader shader)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_destroyBindsetLayout(Opal_Device this, Opal_BindsetLayout bindset_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_destroyBindset(Opal_Device this, Opal_Bindset bindset)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_destroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_destroyGraphicsPipeline(Opal_Device this, Opal_GraphicsPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_destroyMeshletPipeline(Opal_Device this, Opal_MeshletPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_destroyComputePipeline(Opal_Device this, Opal_ComputePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_destroyRaytracePipeline(Opal_Device this, Opal_RaytracePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_destroySwapChain(Opal_Device this, Opal_SwapChain swap_chain)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
static Opal_Result null_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceWaitIdle(Opal_Device device)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceUpdateBindset(Opal_Device device, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceBeginCommandBuffer(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceEndCommandBuffer(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceWaitCommandBuffers(Opal_Device device, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceSubmit(Opal_Device device, Opal_Queue queue, uint32_t num_command_buffer, const Opal_CommandBuffer *command_buffers, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceAcquire(Opal_Device device, Opal_SwapChain swap_chain, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_devicePresent(Opal_Device device, Opal_SwapChain swap_chain, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
static Opal_Result null_deviceCmdBeginGraphicsPass(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_attachments, const Opal_FramebufferAttachment *attachments)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdEndGraphicsPass(Opal_Device device, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetGraphicsPipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_GraphicsPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetMeshletPipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_MeshletPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetComputePipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ComputePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetRaytracePipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_RaytracePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetBindsets(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_bindsets, const Opal_Bindset *bindsets)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetVertexBuffers(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetIndexBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdDrawIndexedInstanced(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t base_index, uint32_t num_instances, uint32_t base_instance)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdMeshletDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdComputeDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdRaytraceDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTableView shader_table, uint32_t width, uint32_t height, uint32_t depth)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyBufferToBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyBufferToTexture(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_TextureRegion dst)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyTextureToBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferView dst)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBufferTransitionBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBufferQueueGrabBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBufferQueueReleaseBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdTextureTransitionBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdTextureQueueGrabBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdTextureQueueReleaseBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
static Opal_DeviceTable device_vtbl =
{
	null_deviceDestroy,
	null_deviceGetInfo,
	null_deviceGetQueue,

	null_deviceCreateBuffer,
	null_deviceCreateTexture,
	null_deviceCreateTextureView,
	null_deviceCreateSampler,
	null_deviceCreateCommandBuffer,
	null_deviceCreateShader,
	null_deviceCreateBindsetLayout,
	null_deviceCreateBindset,
	null_deviceCreatePipelineLayout,
	null_deviceCreateGraphicsPipeline,
	null_deviceCreateMeshletPipeline,
	null_deviceCreateComputePipeline,
	null_deviceCreateRaytracePipeline,
	null_deviceCreateSwapChain,

	null_deviceDestroyBuffer,
	null_deviceDestroyTexture,
	null_deviceDestroyTextureView,
	null_destroySampler,
	null_destroyCommandBuffer,
	null_destroyShader,
	null_destroyBindsetLayout,
	null_destroyBindset,
	null_destroyPipelineLayout,
	null_destroyGraphicsPipeline,
	null_destroyMeshletPipeline,
	null_destroyComputePipeline,
	null_destroyRaytracePipeline,
	null_destroySwapChain,

	null_deviceMapBuffer,
	null_deviceUnmapBuffer,
	null_deviceWaitIdle,
	null_deviceUpdateBindset,
	null_deviceBeginCommandBuffer,
	null_deviceEndCommandBuffer,
	null_deviceWaitCommandBuffers,
	null_deviceSubmit,
	null_deviceAcquire,
	null_devicePresent,

	null_deviceCmdBeginGraphicsPass,
	null_deviceCmdEndGraphicsPass,
	null_deviceCmdSetGraphicsPipeline,
	null_deviceCmdSetMeshletPipeline,
	null_deviceCmdSetComputePipeline,
	null_deviceCmdSetRaytracePipeline,
	null_deviceCmdSetBindsets,
	null_deviceCmdSetVertexBuffers,
	null_deviceCmdSetIndexBuffer,
	null_deviceCmdDrawIndexedInstanced,
	null_deviceCmdMeshletDispatch,
	null_deviceCmdComputeDispatch,
	null_deviceCmdRaytraceDispatch,
	null_deviceCmdCopyBufferToBuffer,
	null_deviceCmdCopyBufferToTexture,
	null_deviceCmdCopyTextureToBuffer,
	null_deviceCmdBufferTransitionBarrier,
	null_deviceCmdBufferQueueGrabBarrier,
	null_deviceCmdBufferQueueReleaseBarrier,
	null_deviceCmdTextureTransitionBarrier,
	null_deviceCmdTextureQueueGrabBarrier,
	null_deviceCmdTextureQueueReleaseBarrier,
};

/*
 */
Opal_Result null_fillDeviceInfo(Opal_DeviceInfo *info)
{
	static const char *device_name = "Null Device";

	assert(info);

	memset(info, 0, sizeof(Opal_DeviceInfo));
	memcpy(info->name, device_name, sizeof(char) * 12);

	info->device_type = OPAL_DEVICE_TYPE_UNKNOWN;
	info->queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 1;
	info->max_buffer_alignment = 256;

	return OPAL_SUCCESS;
}

Opal_Result null_deviceInitialize(Null_Device *device_ptr, Null_Instance *instance_ptr)
{
	assert(instance_ptr);
	assert(device_ptr);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	return null_fillDeviceInfo(&device_ptr->info);
}
