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
	OPAL_UNUSED(this);
	OPAL_UNUSED(engine_type);
	OPAL_UNUSED(index);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceGetAccelerationStructurePrebuildInfo(Opal_Device this, const Opal_AccelerationStructureBuildDesc *desc, Opal_AccelerationStructurePrebuildInfo *info)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(info);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceGetShaderBindingTablePrebuildInfo(Opal_Device this, const Opal_ShaderBindingTableLayoutDesc *desc, Opal_ShaderBindingTablePrebuildInfo *info)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(info);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateSemaphore(Opal_Device this, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(semaphore);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(texture);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(texture_view);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateSampler(Opal_Device this, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(sampler);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateAccelerationStructure(Opal_Device this, const Opal_AccelerationStructureDesc *desc, Opal_AccelerationStructure *acceleration_structure)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(acceleration_structure);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateCommandPool(Opal_Device this, Opal_Queue queue, Opal_CommandPool *command_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateShader(Opal_Device this, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(shader);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateBindsetLayout(Opal_Device this, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(num_bindings);
	OPAL_UNUSED(bindings);
	OPAL_UNUSED(bindset_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateBindsetPool(Opal_Device this, Opal_BindsetLayout bindset_layout, uint32_t max_bindsets, Opal_BindsetPool *bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_layout);
	OPAL_UNUSED(max_bindsets);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(num_bindset_layouts);
	OPAL_UNUSED(bindset_layouts);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateMeshletPipeline(Opal_Device this, const Opal_MeshletPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateComputePipeline(Opal_Device this, const Opal_ComputePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateRaytracePipeline(Opal_Device this, const Opal_RaytracePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCreateSwapchain(Opal_Device this, const Opal_SwapchainDesc *desc, Opal_Swapchain *swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroySemaphore(Opal_Device this, Opal_Semaphore semaphore)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyTexture(Opal_Device this, Opal_Texture texture)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(texture);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(texture_view);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroySampler(Opal_Device this, Opal_Sampler sampler)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(sampler);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyAccelerationStructure(Opal_Device this, Opal_AccelerationStructure acceleration_structure)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(acceleration_structure);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyCommandPool(Opal_Device this, Opal_CommandPool command_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(shader);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyBindsetLayout(Opal_Device this, Opal_BindsetLayout bindset_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyBindsetPool(Opal_Device this, Opal_BindsetPool bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroyPipeline(Opal_Device this, Opal_Pipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceDestroySwapchain(Opal_Device this, Opal_Swapchain swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);

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

static Opal_Result webgpu_deviceBuildShaderBindingTable(Opal_Device this, const Opal_ShaderBindingTableBuildDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceAllocateCommandBuffer(Opal_Device this, Opal_CommandPool command_pool, Opal_CommandBuffer *command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceFreeCommandBuffer(Opal_Device this, Opal_CommandPool command_pool, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceResetCommandPool(Opal_Device this, Opal_CommandPool command_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceResetCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceAllocateBindset(Opal_Device this, Opal_BindsetPool bindset_pool, Opal_Bindset *bindset)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);
	OPAL_UNUSED(bindset);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceFreeBindset(Opal_Device this, Opal_BindsetPool bindset_pool, Opal_Bindset bindset)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);
	OPAL_UNUSED(bindset);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceResetBindsetPool(Opal_Device this, Opal_BindsetPool bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(ptr);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceUpdateBindset(Opal_Device this, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset);
	OPAL_UNUSED(num_bindings);
	OPAL_UNUSED(bindings);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceBeginCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceQuerySemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t *value)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceSignalSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceWaitSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);
	OPAL_UNUSED(timeout_milliseconds);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceWaitQueue(Opal_Device this, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceWaitIdle(Opal_Device this)
{
	OPAL_UNUSED(this);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceSubmit(Opal_Device this, Opal_Queue queue, const Opal_SubmitDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceAcquire(Opal_Device this, Opal_Swapchain swapchain, Opal_TextureView *texture_view)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);
	OPAL_UNUSED(texture_view);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_devicePresent(Opal_Device this, Opal_Swapchain swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depth_stencil_attachment)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_color_attachments);
	OPAL_UNUSED(color_attachments);
	OPAL_UNUSED(depth_stencil_attachment);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBeginComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdEndComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBeginRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdEndRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Pipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetBindsets(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout, uint32_t num_bindsets, const Opal_Bindset *bindsets)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline_layout);
	OPAL_UNUSED(num_bindsets);
	OPAL_UNUSED(bindsets);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_vertex_buffers);
	OPAL_UNUSED(vertex_buffers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer, Opal_IndexFormat index_format)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(index_buffer);
	OPAL_UNUSED(index_format);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetViewport(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Viewport viewport)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(viewport);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdSetScissor(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(x);
	OPAL_UNUSED(y);
	OPAL_UNUSED(width);
	OPAL_UNUSED(height);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdDrawIndexedInstanced(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t base_index, uint32_t num_instances, uint32_t base_instance)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_indices);
	OPAL_UNUSED(base_index);
	OPAL_UNUSED(num_instances);
	OPAL_UNUSED(base_instance);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdMeshletDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_groups_x);
	OPAL_UNUSED(num_groups_y);
	OPAL_UNUSED(num_groups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdComputeDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_groups_x);
	OPAL_UNUSED(num_groups_y);
	OPAL_UNUSED(num_groups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdRaytraceDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView raygen_entry, Opal_BufferView hitgroup_entry, Opal_BufferView miss_entry, uint32_t width, uint32_t height, uint32_t depth)
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

static Opal_Result webgpu_deviceCmdBuildAccelerationStructures(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_build_descs, const Opal_AccelerationStructureBuildDesc *descs)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_build_descs);
	OPAL_UNUSED(descs);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyAccelerationStructure(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_AccelerationStructure src, Opal_AccelerationStructure dst, Opal_AccelerationStructureCopyMode mode)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(mode);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyAccelerationStructuresPostbuildInfo(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_src_acceleration_structures, const Opal_AccelerationStructure *src_acceleration_structures, Opal_BufferView dst_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_src_acceleration_structures);
	OPAL_UNUSED(src_acceleration_structures);
	OPAL_UNUSED(dst_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyBufferToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_TextureRegion dst)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdCopyTextureToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferView dst)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBufferTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBufferQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdBufferQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdTextureTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdTextureQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_deviceCmdTextureQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
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
	webgpu_deviceGetInfo,
	webgpu_deviceGetQueue,
	webgpu_deviceGetAccelerationStructurePrebuildInfo,
	webgpu_deviceGetShaderBindingTablePrebuildInfo,

	webgpu_deviceCreateSemaphore,
	webgpu_deviceCreateBuffer,
	webgpu_deviceCreateTexture,
	webgpu_deviceCreateTextureView,
	webgpu_deviceCreateSampler,
	webgpu_deviceCreateAccelerationStructure,
	webgpu_deviceCreateCommandPool,
	webgpu_deviceCreateShader,
	webgpu_deviceCreateBindsetLayout,
	webgpu_deviceCreateBindsetPool,
	webgpu_deviceCreatePipelineLayout,
	webgpu_deviceCreateGraphicsPipeline,
	webgpu_deviceCreateMeshletPipeline,
	webgpu_deviceCreateComputePipeline,
	webgpu_deviceCreateRaytracePipeline,
	webgpu_deviceCreateSwapchain,

	webgpu_deviceDestroySemaphore,
	webgpu_deviceDestroyBuffer,
	webgpu_deviceDestroyTexture,
	webgpu_deviceDestroyTextureView,
	webgpu_deviceDestroySampler,
	webgpu_deviceDestroyAccelerationStructure,
	webgpu_deviceDestroyCommandPool,
	webgpu_deviceDestroyShader,
	webgpu_deviceDestroyBindsetLayout,
	webgpu_deviceDestroyBindsetPool,
	webgpu_deviceDestroyPipelineLayout,
	webgpu_deviceDestroyPipeline,
	webgpu_deviceDestroySwapchain,
	webgpu_deviceDestroy,

	webgpu_deviceBuildShaderBindingTable,
	webgpu_deviceAllocateCommandBuffer,
	webgpu_deviceFreeCommandBuffer,
	webgpu_deviceResetCommandPool,
	webgpu_deviceResetCommandBuffer,
	webgpu_deviceAllocateBindset,
	webgpu_deviceFreeBindset,
	webgpu_deviceResetBindsetPool,
	webgpu_deviceMapBuffer,
	webgpu_deviceUnmapBuffer,
	webgpu_deviceUpdateBindset,
	webgpu_deviceBeginCommandBuffer,
	webgpu_deviceEndCommandBuffer,
	webgpu_deviceQuerySemaphore,
	webgpu_deviceSignalSemaphore,
	webgpu_deviceWaitSemaphore,
	webgpu_deviceWaitQueue,
	webgpu_deviceWaitIdle,
	webgpu_deviceSubmit,
	webgpu_deviceAcquire,
	webgpu_devicePresent,

	webgpu_deviceCmdBeginGraphicsPass,
	webgpu_deviceCmdEndGraphicsPass,
	webgpu_deviceCmdBeginComputePass,
	webgpu_deviceCmdEndComputePass,
	webgpu_deviceCmdBeginRaytracePass,
	webgpu_deviceCmdEndRaytracePass,
	webgpu_deviceCmdSetPipeline,
	webgpu_deviceCmdSetBindsets,
	webgpu_deviceCmdSetVertexBuffers,
	webgpu_deviceCmdSetIndexBuffer,
	webgpu_deviceCmdSetViewport,
	webgpu_deviceCmdSetScissor,
	webgpu_deviceCmdDrawIndexedInstanced,
	webgpu_deviceCmdMeshletDispatch,
	webgpu_deviceCmdComputeDispatch,
	webgpu_deviceCmdRaytraceDispatch,
	webgpu_deviceCmdBuildAccelerationStructures,
	webgpu_deviceCmdCopyAccelerationStructure,
	webgpu_deviceCmdCopyAccelerationStructuresPostbuildInfo,
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
