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

static Opal_Result null_deviceGetSupportedSurfaceFormats(Opal_Device this, Opal_Surface surface, uint32_t *num_formats, Opal_SurfaceFormat *formats)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(num_formats);
	OPAL_UNUSED(formats);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceGetSupportedPresentModes(Opal_Device this, Opal_Surface surface, uint32_t *num_present_modes, Opal_PresentMode *present_modes)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(num_present_modes);
	OPAL_UNUSED(present_modes);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceGetPreferredSurfaceFormat(Opal_Device this, Opal_Surface surface, Opal_SurfaceFormat *format)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(format);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceGetPreferredSurfacePresentMode(Opal_Device this, Opal_Surface surface, Opal_PresentMode *present_mode)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);
	OPAL_UNUSED(present_mode);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateSemaphore(Opal_Device this, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(semaphore);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateFence(Opal_Device this, Opal_Fence *fence)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(fence);

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

static Opal_Result null_deviceCreateShaderBindingTable(Opal_Device this, Opal_RaytracePipeline pipeline, Opal_ShaderBindingTable *shader_binding_table)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline);
	OPAL_UNUSED(shader_binding_table);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateCommandAllocator(Opal_Device this, Opal_Queue queue, Opal_CommandAllocator *command_allocator)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(queue);
	OPAL_UNUSED(command_allocator);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateCommandBuffer(Opal_Device this, Opal_CommandAllocator command_allocator, Opal_CommandBuffer *command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_allocator);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateShader(Opal_Device this, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(shader);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateDescriptorHeap(Opal_Device this, const Opal_DescriptorHeapDesc *desc, Opal_DescriptorHeap *descriptor_heap)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(descriptor_heap);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateDescriptorSetLayout(Opal_Device this, uint32_t num_entries, const Opal_DescriptorSetLayoutEntry *entries, Opal_DescriptorSetLayout *descriptor_set_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(num_entries);
	OPAL_UNUSED(entries);
	OPAL_UNUSED(descriptor_set_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_descriptor_set_layouts, const Opal_DescriptorSetLayout *descriptor_set_layouts, Opal_PipelineLayout *pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(num_descriptor_set_layouts);
	OPAL_UNUSED(descriptor_set_layouts);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_GraphicsPipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateMeshletPipeline(Opal_Device this, const Opal_MeshletPipelineDesc *desc, Opal_GraphicsPipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateComputePipeline(Opal_Device this, const Opal_ComputePipelineDesc *desc, Opal_ComputePipeline *pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCreateRaytracePipeline(Opal_Device this, const Opal_RaytracePipelineDesc *desc, Opal_RaytracePipeline *pipeline)
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

static Opal_Result null_deviceDestroyFence(Opal_Device this, Opal_Fence fence)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(fence);

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

static Opal_Result null_deviceDestroyShaderBindingTable(Opal_Device this, Opal_ShaderBindingTable shader_binding_table)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(shader_binding_table);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyCommandAllocator(Opal_Device this, Opal_CommandAllocator command_allocator)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_allocator);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(shader);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyDescriptorHeap(Opal_Device this, Opal_DescriptorHeap descriptor_heap)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(descriptor_heap);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyDescriptorSetLayout(Opal_Device this, Opal_DescriptorSetLayout descriptor_set_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(descriptor_set_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyGraphicsPipeline(Opal_Device this, Opal_GraphicsPipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyComputePipeline(Opal_Device this, Opal_ComputePipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceDestroyRaytracePipeline(Opal_Device this, Opal_RaytracePipeline pipeline)
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

static Opal_Result null_deviceBuildShaderBindingTable(Opal_Device this, Opal_ShaderBindingTable shader_binding_table, const Opal_ShaderBindingTableBuildDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(shader_binding_table);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceBuildAccelerationStructureInstanceBuffer(Opal_Device this, const Opal_AccelerationStructureInstanceBufferBuildDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceResetCommandAllocator(Opal_Device this, Opal_CommandAllocator command_allocator)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_allocator);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceAllocateDescriptorSet(Opal_Device this, const Opal_DescriptorSetAllocationDesc *desc, Opal_DescriptorSet *descriptor_set)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(descriptor_set);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceFreeDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(descriptor_set);

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

static Opal_Result null_deviceWriteBuffer(Opal_Device this, Opal_Buffer buffer, uint64_t offset, const void *data, uint64_t size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(offset);
	OPAL_UNUSED(data);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceUpdateDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set, uint32_t num_entries, const Opal_DescriptorSetEntry *entries)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(descriptor_set);
	OPAL_UNUSED(num_entries);
	OPAL_UNUSED(entries);

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

static Opal_Result null_deviceCmdSetDescriptorHeap(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_DescriptorHeap descriptor_heap)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(descriptor_heap);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_FramebufferDesc *framebuffer, const Opal_PassBarriersDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(framebuffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdGraphicsSetPipelineLayout(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdGraphicsSetPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_GraphicsPipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdGraphicsSetDescriptorSet(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(index);
	OPAL_UNUSED(descriptor_set);
	OPAL_UNUSED(num_dynamic_offsets);
	OPAL_UNUSED(dynamic_offsets);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdGraphicsSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t first_index, uint32_t num_vertex_buffers, const Opal_VertexBufferView *vertex_buffers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(first_index);
	OPAL_UNUSED(num_vertex_buffers);
	OPAL_UNUSED(vertex_buffers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdGraphicsSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_IndexBufferView index_buffer)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(index_buffer);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdGraphicsSetViewport(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Viewport viewport)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(viewport);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdGraphicsSetScissor(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(x);
	OPAL_UNUSED(y);
	OPAL_UNUSED(width);
	OPAL_UNUSED(height);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdGraphicsDraw(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertices, uint32_t num_instances, uint32_t base_vertex, uint32_t base_instance)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_vertices);
	OPAL_UNUSED(num_instances);
	OPAL_UNUSED(base_vertex);
	OPAL_UNUSED(base_instance);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdGraphicsDrawIndexed(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t num_instances, uint32_t base_index, int32_t vertex_offset, uint32_t base_instance)
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

static Opal_Result null_deviceCmdGraphicsMeshletDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_threadgroups_x, uint32_t num_threadgroups_y, uint32_t num_threadgroups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_threadgroups_x);
	OPAL_UNUSED(num_threadgroups_y);
	OPAL_UNUSED(num_threadgroups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_PassBarriersDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBeginComputePass(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_PassBarriersDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdComputeSetPipelineLayout(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdComputeSetPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_GraphicsPipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdComputeSetDescriptorSet(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(index);
	OPAL_UNUSED(descriptor_set);
	OPAL_UNUSED(num_dynamic_offsets);
	OPAL_UNUSED(dynamic_offsets);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdComputeMemoryBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_MemoryBarrierDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdComputeDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_threadgroups_x, uint32_t num_threadgroups_y, uint32_t num_threadgroups_z)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(num_threadgroups_x);
	OPAL_UNUSED(num_threadgroups_y);
	OPAL_UNUSED(num_threadgroups_z);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdEndComputePass(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_PassBarriersDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBeginRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_PassBarriersDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdRaytraceSetPipelineLayout(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline_layout);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdRaytraceSetPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_GraphicsPipeline pipeline)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(pipeline);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdRaytraceSetDescriptorSet(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(index);
	OPAL_UNUSED(descriptor_set);
	OPAL_UNUSED(num_dynamic_offsets);
	OPAL_UNUSED(dynamic_offsets);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdRaytraceSetShaderBindingTable(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTable shader_binding_table)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(shader_binding_table);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdRaytraceMemoryBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_MemoryBarrierDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdRaytraceDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t width, uint32_t height, uint32_t depth)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(width);
	OPAL_UNUSED(height);
	OPAL_UNUSED(depth);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdEndRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_PassBarriersDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBeginCopyPass(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_PassBarriersDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyBufferToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Buffer src_buffer, uint64_t src_offset, Opal_Buffer dst_buffer, uint64_t dst_offset, uint64_t size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src_buffer);
	OPAL_UNUSED(src_offset);
	OPAL_UNUSED(dst_buffer);
	OPAL_UNUSED(dst_offset);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferTextureRegion src, Opal_TextureRegion dst, Opal_Extent3D size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyTextureToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferTextureRegion dst, Opal_Extent3D size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdCopyTextureToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_TextureRegion dst, Opal_Extent3D size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(src);
	OPAL_UNUSED(dst);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdEndCopyPass(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_PassBarriersDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdBeginAccelerationStructurePass(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_PassBarriersDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdAccelerationStructureBuild(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_AccelerationStructureBuildDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdAccelerationStructureCopy(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_AccelerationStructureCopyDesc *desc)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(desc);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_deviceCmdEndAccelerationStructurePass(Opal_Device this, Opal_CommandBuffer command_buffer, const Opal_PassBarriersDesc *barriers)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(barriers);

	return OPAL_NOT_SUPPORTED;
}

/*
 */
static Opal_DeviceTable device_vtbl =
{
	null_deviceGetInfo,
	null_deviceGetQueue,
	null_deviceGetAccelerationStructurePrebuildInfo,
	null_deviceGetSupportedSurfaceFormats,
	null_deviceGetSupportedPresentModes,
	null_deviceGetPreferredSurfaceFormat,
	null_deviceGetPreferredSurfacePresentMode,

	null_deviceCreateSemaphore,
	null_deviceCreateFence,
	null_deviceCreateBuffer,
	null_deviceCreateTexture,
	null_deviceCreateTextureView,
	null_deviceCreateSampler,
	null_deviceCreateAccelerationStructure,
	null_deviceCreateShaderBindingTable,
	null_deviceCreateCommandAllocator,
	null_deviceCreateCommandBuffer,
	null_deviceCreateShader,
	null_deviceCreateDescriptorHeap,
	null_deviceCreateDescriptorSetLayout,
	null_deviceCreatePipelineLayout,
	null_deviceCreateGraphicsPipeline,
	null_deviceCreateMeshletPipeline,
	null_deviceCreateComputePipeline,
	null_deviceCreateRaytracePipeline,
	null_deviceCreateSwapchain,

	null_deviceDestroySemaphore,
	null_deviceDestroyFence,
	null_deviceDestroyBuffer,
	null_deviceDestroyTexture,
	null_deviceDestroyTextureView,
	null_deviceDestroySampler,
	null_deviceDestroyAccelerationStructure,
	null_deviceDestroyShaderBindingTable,
	null_deviceDestroyCommandAllocator,
	null_deviceDestroyCommandBuffer,
	null_deviceDestroyShader,
	null_deviceDestroyDescriptorHeap,
	null_deviceDestroyDescriptorSetLayout,
	null_deviceDestroyPipelineLayout,
	null_deviceDestroyGraphicsPipeline,
	null_deviceDestroyComputePipeline,
	null_deviceDestroyRaytracePipeline,
	null_deviceDestroySwapchain,
	null_deviceDestroy,

	null_deviceBuildShaderBindingTable,
	null_deviceBuildAccelerationStructureInstanceBuffer,
	null_deviceResetCommandAllocator,
	null_deviceAllocateDescriptorSet,
	null_deviceFreeDescriptorSet,
	null_deviceMapBuffer,
	null_deviceUnmapBuffer,
	null_deviceWriteBuffer,
	null_deviceUpdateDescriptorSet,
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

	null_deviceCmdSetDescriptorHeap,

	null_deviceCmdBeginGraphicsPass,
	null_deviceCmdGraphicsSetPipelineLayout,
	null_deviceCmdGraphicsSetPipeline,
	null_deviceCmdGraphicsSetDescriptorSet,
	null_deviceCmdGraphicsSetVertexBuffers,
	null_deviceCmdGraphicsSetIndexBuffer,
	null_deviceCmdGraphicsSetViewport,
	null_deviceCmdGraphicsSetScissor,
	null_deviceCmdGraphicsDraw,
	null_deviceCmdGraphicsDrawIndexed,
	null_deviceCmdGraphicsMeshletDispatch,
	null_deviceCmdEndGraphicsPass,

	null_deviceCmdBeginComputePass,
	null_deviceCmdComputeSetPipelineLayout,
	null_deviceCmdComputeSetPipeline,
	null_deviceCmdComputeSetDescriptorSet,
	null_deviceCmdComputeMemoryBarrier,
	null_deviceCmdComputeDispatch,
	null_deviceCmdEndComputePass,

	null_deviceCmdBeginRaytracePass,
	null_deviceCmdRaytraceSetPipelineLayout,
	null_deviceCmdRaytraceSetPipeline,
	null_deviceCmdRaytraceSetDescriptorSet,
	null_deviceCmdRaytraceSetShaderBindingTable,
	null_deviceCmdRaytraceMemoryBarrier,
	null_deviceCmdRaytraceDispatch,
	null_deviceCmdEndRaytracePass,

	null_deviceCmdBeginCopyPass,
	null_deviceCmdCopyBufferToBuffer,
	null_deviceCmdCopyBufferToTexture,
	null_deviceCmdCopyTextureToBuffer,
	null_deviceCmdCopyTextureToTexture,
	null_deviceCmdEndCopyPass,

	null_deviceCmdBeginAccelerationStructurePass,
	null_deviceCmdAccelerationStructureBuild,
	null_deviceCmdAccelerationStructureCopy,
	null_deviceCmdEndAccelerationStructurePass,
};

/*
 */
Opal_Result null_fillDeviceInfo(Opal_DeviceInfo *info)
{
	static const char *device_name = "Null Device";

	assert(info);

	memset(info, 0, sizeof(Opal_DeviceInfo));
	memcpy(info->name, device_name, sizeof(char) * 12);

	info->api = OPAL_API_NULL;
	info->device_type = OPAL_DEVICE_TYPE_UNKNOWN;
	info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 1;

	info->limits.max_texture_dimension_1d = 16384;
	info->limits.max_texture_dimension_2d = 16384;
	info->limits.max_texture_dimension_3d = 2048;
	info->limits.max_texture_array_layers = 2048;
	info->limits.max_buffer_size = 0xFFFFFFFF;
	info->limits.min_uniform_buffer_offset_alignment = 16;
	info->limits.min_storage_buffer_offset_alignment = 4;
	info->limits.max_descriptor_sets = 8;
	info->limits.max_uniform_buffer_binding_size = 0x0000FFFF;
	info->limits.max_storage_buffer_binding_size = 0xFFFFFFFF;
	info->limits.max_vertex_buffers = 32;
	info->limits.max_vertex_attributes = 64;
	info->limits.max_vertex_buffer_stride = 0x00003FFF;
	info->limits.max_color_attachments = 8;

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
