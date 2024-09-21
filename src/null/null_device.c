#include "null_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Opal_Result null_deviceFreeBindset(Opal_Device this, Opal_BindsetPool bindset_pool, Opal_Bindset bindset);
static Opal_Result null_deviceUpdateBindset(Opal_Device this, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings);

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
	OPAL_UNUSED(this);
	OPAL_UNUSED(engine_type);
	OPAL_UNUSED(index);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceGetAccelerationStructurePrebuildInfo(Opal_Device this, const Opal_AccelerationStructureBuildDesc *desc, Opal_AccelerationStructurePrebuildInfo *info)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(info);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceGetShaderBindingTablePrebuildInfo(Opal_Device this, const Opal_ShaderBindingTableLayoutDesc *desc, Opal_ShaderBindingTablePrebuildInfo *info)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(info);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateSemaphore(Opal_Device this, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(semaphore);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(texture);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(texture_view);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateSampler(Opal_Device this, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(sampler);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateAccelerationStructure(Opal_Device this, const Opal_AccelerationStructureDesc *desc, Opal_AccelerationStructure *acceleration_structure)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(acceleration_structure);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateCommandPool(Opal_Device this, Opal_Queue queue, Opal_CommandPool *command_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateShader(Opal_Device this, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(shader);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateBindsetLayout(Opal_Device this, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(num_bindings);
	OPAL_UNUSED(bindings);
	OPAL_UNUSED(bindset_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateBindsetPool(Opal_Device this, const Opal_BindsetPoolDesc *desc, Opal_BindsetPool *bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(num_bindset_layouts);
	OPAL_UNUSED(bindset_layouts);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateMeshletPipeline(Opal_Device this, const Opal_MeshletPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateComputePipeline(Opal_Device this, const Opal_ComputePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateRaytracePipeline(Opal_Device this, const Opal_RaytracePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateSwapchain(Opal_Device this, const Opal_SwapchainDesc *desc, Opal_Swapchain *swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroySemaphore(Opal_Device this, Opal_Semaphore semaphore)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyTexture(Opal_Device this, Opal_Texture texture)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(texture);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(texture_view);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroySampler(Opal_Device this, Opal_Sampler sampler)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(sampler);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyAccelerationStructure(Opal_Device this, Opal_AccelerationStructure acceleration_structure)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(acceleration_structure);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyCommandPool(Opal_Device this, Opal_CommandPool command_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(shader);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyBindsetLayout(Opal_Device this, Opal_BindsetLayout bindset_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyBindsetPool(Opal_Device this, Opal_BindsetPool bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyPipeline(Opal_Device this, Opal_Pipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroySwapchain(Opal_Device this, Opal_Swapchain swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroy(Opal_Device this)
{
	assert(this);

	Null_Device *ptr = (Null_Device *)this;

	free(ptr);
	return OPAL_SUCCESS;
}

static Opal_Result null_deviceBuildShaderBindingTable(Opal_Device this, const Opal_ShaderBindingTableBuildDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceAllocateCommandBuffer(Opal_Device this, Opal_CommandPool command_pool, Opal_CommandBuffer *command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceFreeCommandBuffer(Opal_Device this, Opal_CommandPool command_pool, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceResetCommandPool(Opal_Device this, Opal_CommandPool command_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceResetCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceAllocateEmptyBindset(Opal_Device this, Opal_BindsetLayout bindset_layout, Opal_BindsetPool bindset_pool, Opal_Bindset *bindset)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_layout);
	OPAL_UNUSED(bindset_pool);
	OPAL_UNUSED(bindset);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceAllocatePrefilledBindset(Opal_Device this, Opal_BindsetLayout bindset_layout, Opal_BindsetPool bindset_pool, uint32_t num_bindings, const Opal_BindsetBinding *bindings, Opal_Bindset *bindset)
{
	Opal_Result result = null_deviceAllocateEmptyBindset(this, bindset_layout, bindset_pool, bindset);
	if (result != OPAL_SUCCESS)
		return result;

	assert(bindset);
	result = null_deviceUpdateBindset(this, *bindset, num_bindings, bindings);
	if (result != OPAL_SUCCESS)
	{
		null_deviceFreeBindset(this, bindset_pool, *bindset);
		*bindset = OPAL_NULL_HANDLE;
		return result;
	}

	return OPAL_SUCCESS;
}

static Opal_Result null_deviceFreeBindset(Opal_Device this, Opal_BindsetPool bindset_pool, Opal_Bindset bindset)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);
	OPAL_UNUSED(bindset);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceResetBindsetPool(Opal_Device this, Opal_BindsetPool bindset_pool)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset_pool);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(ptr);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceWriteBuffer(Opal_Device this, Opal_Queue queue, Opal_BufferView buffer, const void *data, uint64_t size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(data);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceUpdateBindset(Opal_Device this, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(bindset);
	OPAL_UNUSED(num_bindings);
	OPAL_UNUSED(bindings);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceBeginCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceQuerySemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t *value)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceSignalSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceWaitSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(semaphore);
	OPAL_UNUSED(value);
	OPAL_UNUSED(timeout_milliseconds);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceWaitQueue(Opal_Device this, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceWaitIdle(Opal_Device this)
{
	OPAL_UNUSED(this);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceSubmit(Opal_Device this, Opal_Queue queue, const Opal_SubmitDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceAcquire(Opal_Device this, Opal_Swapchain swapchain, Opal_TextureView *texture_view)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);
	OPAL_UNUSED(texture_view);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_devicePresent(Opal_Device this, Opal_Swapchain swapchain)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(swapchain);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depth_stencil_attachment)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_color_attachments);
	OPAL_UNUSED(color_attachments);
	OPAL_UNUSED(depth_stencil_attachment);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBeginComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdEndComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBeginRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdEndRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Pipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetBindsets(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout, uint32_t num_bindsets, const Opal_Bindset *bindsets)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline_layout);
	OPAL_UNUSED(num_bindsets);
	OPAL_UNUSED(bindsets);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_vertex_buffers);
	OPAL_UNUSED(vertex_buffers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer, Opal_IndexFormat index_format)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(index_buffer);
	OPAL_UNUSED(index_format);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetViewport(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Viewport viewport)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(viewport);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdSetScissor(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(x);
	OPAL_UNUSED(y);
	OPAL_UNUSED(width);
	OPAL_UNUSED(height);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdDraw(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertices, uint32_t num_instances, uint32_t base_vertex, uint32_t base_instance)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_vertices);
	OPAL_UNUSED(num_instances);
	OPAL_UNUSED(base_vertex);
	OPAL_UNUSED(base_instance);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdDrawIndexed(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t num_instances, uint32_t base_index, int32_t vertex_offset, uint32_t base_instance)
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

static Opal_Result null_deviceCmdMeshletDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_groups_x);
	OPAL_UNUSED(num_groups_y);
	OPAL_UNUSED(num_groups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdComputeDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_groups_x);
	OPAL_UNUSED(num_groups_y);
	OPAL_UNUSED(num_groups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdRaytraceDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView raygen_entry, Opal_BufferView hitgroup_entry, Opal_BufferView miss_entry, uint32_t width, uint32_t height, uint32_t depth)
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

static Opal_Result null_deviceCmdBuildAccelerationStructures(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_build_descs, const Opal_AccelerationStructureBuildDesc *descs)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_build_descs);
	OPAL_UNUSED(descs);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyAccelerationStructure(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_AccelerationStructure src, Opal_AccelerationStructure dst, Opal_AccelerationStructureCopyMode mode)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(mode);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyAccelerationStructuresPostbuildInfo(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_src_acceleration_structures, const Opal_AccelerationStructure *src_acceleration_structures, Opal_BufferView dst_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_src_acceleration_structures);
	OPAL_UNUSED(src_acceleration_structures);
	OPAL_UNUSED(dst_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyBufferToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferTextureRegion src, Opal_TextureRegion dst)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyTextureToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferTextureRegion dst)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBufferTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBufferQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBufferQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdTextureTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(state_before);
	OPAL_UNUSED(state_after);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdTextureQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdTextureQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
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
	null_deviceGetInfo,
	null_deviceGetQueue,
	null_deviceGetAccelerationStructurePrebuildInfo,
	null_deviceGetShaderBindingTablePrebuildInfo,

	null_deviceCreateSemaphore,
	null_deviceCreateBuffer,
	null_deviceCreateTexture,
	null_deviceCreateTextureView,
	null_deviceCreateSampler,
	null_deviceCreateAccelerationStructure,
	null_deviceCreateCommandPool,
	null_deviceCreateShader,
	null_deviceCreateBindsetLayout,
	null_deviceCreateBindsetPool,
	null_deviceCreatePipelineLayout,
	null_deviceCreateGraphicsPipeline,
	null_deviceCreateMeshletPipeline,
	null_deviceCreateComputePipeline,
	null_deviceCreateRaytracePipeline,
	null_deviceCreateSwapchain,

	null_deviceDestroySemaphore,
	null_deviceDestroyBuffer,
	null_deviceDestroyTexture,
	null_deviceDestroyTextureView,
	null_deviceDestroySampler,
	null_deviceDestroyAccelerationStructure,
	null_deviceDestroyCommandPool,
	null_deviceDestroyShader,
	null_deviceDestroyBindsetLayout,
	null_deviceDestroyBindsetPool,
	null_deviceDestroyPipelineLayout,
	null_deviceDestroyPipeline,
	null_deviceDestroySwapchain,
	null_deviceDestroy,

	null_deviceBuildShaderBindingTable,
	null_deviceAllocateCommandBuffer,
	null_deviceFreeCommandBuffer,
	null_deviceResetCommandPool,
	null_deviceResetCommandBuffer,
	null_deviceAllocateEmptyBindset,
	null_deviceAllocatePrefilledBindset,
	null_deviceFreeBindset,
	null_deviceResetBindsetPool,
	null_deviceMapBuffer,
	null_deviceUnmapBuffer,
	null_deviceWriteBuffer,
	null_deviceUpdateBindset,
	null_deviceBeginCommandBuffer,
	null_deviceEndCommandBuffer,
	null_deviceQuerySemaphore,
	null_deviceSignalSemaphore,
	null_deviceWaitSemaphore,
	null_deviceWaitQueue,
	null_deviceWaitIdle,
	null_deviceSubmit,
	null_deviceAcquire,
	null_devicePresent,

	null_deviceCmdBeginGraphicsPass,
	null_deviceCmdEndGraphicsPass,
	null_deviceCmdBeginComputePass,
	null_deviceCmdEndComputePass,
	null_deviceCmdBeginRaytracePass,
	null_deviceCmdEndRaytracePass,
	null_deviceCmdSetPipeline,
	null_deviceCmdSetBindsets,
	null_deviceCmdSetVertexBuffers,
	null_deviceCmdSetIndexBuffer,
	null_deviceCmdSetViewport,
	null_deviceCmdSetScissor,
	null_deviceCmdDraw,
	null_deviceCmdDrawIndexed,
	null_deviceCmdMeshletDispatch,
	null_deviceCmdComputeDispatch,
	null_deviceCmdRaytraceDispatch,
	null_deviceCmdBuildAccelerationStructures,
	null_deviceCmdCopyAccelerationStructure,
	null_deviceCmdCopyAccelerationStructuresPostbuildInfo,
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
	info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 1;

	info->limits.maxTextureDimension1D = 16384;
	info->limits.maxTextureDimension2D = 16384;
	info->limits.maxTextureDimension3D = 2048;
	info->limits.maxTextureArrayLayers = 2048;
	info->limits.maxBufferSize = 0xFFFFFFFF;
	info->limits.minUniformBufferOffsetAlignment = 16;
	info->limits.minStorageBufferOffsetAlignment = 4;
	info->limits.maxBindsets = 8;
	info->limits.maxUniformBufferBindingSize = 0x0000FFFF;
	info->limits.maxStorageBufferBindingSize = 0xFFFFFFFF;
	info->limits.maxVertexBuffers = 32;
	info->limits.maxVertexAttributes = 64;
	info->limits.maxVertexBufferStride = 0x00003FFF;
	info->limits.maxColorAttachments = 8;

	return OPAL_SUCCESS;
}

Opal_Result null_deviceInitialize(Null_Device *device_ptr, Null_Instance *instance_ptr)
{
	assert(instance_ptr);
	assert(device_ptr);

	OPAL_UNUSED(instance_ptr);

	// vtable
	device_ptr->vtbl = &device_vtbl;

	// data
	return null_fillDeviceInfo(&device_ptr->info);
}
