#include "app.h"

#include <cstddef>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cmath>

#ifdef OPAL_PLATFORM_WINDOWS
static constexpr Opal_Api target_api = OPAL_API_DIRECTX12;
#elif OPAL_PLATFORM_MACOS
static constexpr Opal_Api target_api = OPAL_API_METAL;
#elif OPAL_PLATFORM_WEB
static constexpr Opal_Api target_api = OPAL_API_WEBGPU;
#endif

#define UNUSED(x) do { (void)(x); } while(0)

#ifdef OPAL_PLATFORM_MACOS
static const char *compute_shader_paths[] =
{
	nullptr,
	nullptr,
	"samples/05_rt_triangle/shaders/metal/main.comp.metallib",
	nullptr,
	nullptr,
};
#else
static const char *raygen_shader_paths[] =
{
	"samples/05_rt_triangle/shaders/vulkan/main.rgen.spv",
	"samples/05_rt_triangle/shaders/directx12/main.rgen.cso",
	nullptr,
	nullptr,
	nullptr,
};

static const char *closest_hit_shader_paths[] =
{
	"samples/05_rt_triangle/shaders/vulkan/main.rchit.spv",
	"samples/05_rt_triangle/shaders/directx12/main.rchit.cso",
	nullptr,
	nullptr,
	nullptr,
};

static const char *miss_shader_paths[] =
{
	"samples/05_rt_triangle/shaders/vulkan/main.rmiss.spv",
	"samples/05_rt_triangle/shaders/directx12/main.rmiss.cso",
	nullptr,
	nullptr,
	nullptr,
};
#endif

static Opal_ShaderSourceType shader_types[] =
{
	OPAL_SHADER_SOURCE_TYPE_SPIRV_BINARY,
	OPAL_SHADER_SOURCE_TYPE_DXIL_BINARY,
	OPAL_SHADER_SOURCE_TYPE_METALLIB_BINARY,
	OPAL_SHADER_SOURCE_TYPE_WGSL_SOURCE,
	OPAL_SHADER_SOURCE_ENUM_FORCE32,
};

struct Triangle
{
	float vertices[3][3];
	uint32_t indices[3];
	float transform[3][4];
};

static Triangle triangle_data =
{
	// vertices
	-0.8660254f,  0.5f, 0.0f,
	 0.8660254f,  0.5f, 0.0f,
	 0.0f,       -1.0f, 0.0f,

	// indices
	0, 1, 2,
};

bool loadShader(const char *name, Opal_ShaderSourceType type, Opal_Device device, Opal_Shader *shader)
{
	assert(name);
	assert(device);
	assert(shader);

	FILE *f = fopen(name, "rb");
	if (f == nullptr)
		return false;

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	size_t storage_size = size;
	if (type == OPAL_SHADER_SOURCE_TYPE_WGSL_SOURCE)
		storage_size += 1;

	uint8_t *data = new uint8_t[storage_size];
	memset(data, 0, storage_size);

	fread(data, sizeof(uint8_t), size, f);
	fclose(f);

	Opal_ShaderDesc desc =
	{
		type,
		data,
		storage_size,
	};

	Opal_Result result = opalCreateShader(device, &desc, shader);
	assert(result == OPAL_SUCCESS);

	delete[] data;
	return result == OPAL_SUCCESS;
}

/*
 */
void Application::init(void *handle, uint32_t w, uint32_t h)
{
	width = w;
	height = h;

	// instance & surface
	Opal_InstanceDesc instance_desc =
	{
		"05_rt_triangle",
		"Opal",
		0,
		0,
		OPAL_DEFAULT_HEAP_SIZE,
		OPAL_DEFAULT_HEAP_ALLOCATIONS,
		OPAL_DEFAULT_HEAPS,
		OPAL_INSTANCE_CREATION_FLAGS_USE_DEBUG_LAYERS,
	};

	Opal_Result result = opalCreateInstance(target_api, &instance_desc, &instance);
	assert(result == OPAL_SUCCESS);

	result = opalCreateSurface(instance, handle, &surface);
	assert(result == OPAL_SUCCESS);

	// device & swapchain
	result = opalCreateDefaultDevice(instance, OPAL_DEVICE_HINT_DEFAULT, &device);
	assert(result == OPAL_SUCCESS);

	result = opalGetDeviceInfo(device, &device_info);
	assert(result == OPAL_SUCCESS);

	result = opalGetDeviceQueue(device, OPAL_DEVICE_ENGINE_TYPE_MAIN, 0, &queue);
	assert(result == OPAL_SUCCESS);

	Opal_SwapchainDesc swapchain_desc = {};
	swapchain_desc.surface = surface;
	swapchain_desc.usage = (Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_FRAGMENT_SHADER_SAMPLED | OPAL_TEXTURE_USAGE_COPY_DST);

	result = opalGetPreferredSurfaceFormat(device, surface, &swapchain_desc.format);
	assert(result == OPAL_SUCCESS);

	result = opalGetPreferredSurfacePresentMode(device, surface, &swapchain_desc.mode);
	assert(result == OPAL_SUCCESS);

	result = opalCreateSwapchain(device, &swapchain_desc, &swapchain);
	assert(result == OPAL_SUCCESS);

	// semaphore
	Opal_SemaphoreDesc semaphore_desc = {};
	semaphore_desc.flags = OPAL_SEMAPHORE_CREATION_FLAGS_HOST_OPERATIONS;
	semaphore_desc.initial_value = 0;

	result = opalCreateSemaphore(device, &semaphore_desc, &semaphore);
	assert(result == OPAL_SUCCESS);

	result = opalCreateFence(device, &fence);
	assert(result == OPAL_SUCCESS);

	buildPipelineLayout();
	buildPipeline();
#ifndef OPAL_PLATFORM_MACOS
	buildSBT();
#endif
	buildBLAS();
	buildTLAS();

	constexpr uint64_t camera_buffer_size = 256;
	Opal_BufferDesc buffer_desc =
	{
		camera_buffer_size,
		OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
		OPAL_ALLOCATION_HINT_AUTO,
		OPAL_BUFFER_USAGE_UNIFORM,
		OPAL_BUFFER_STATE_GENERIC_READ,
	};

	result = opalCreateBuffer(device, &buffer_desc, &camera_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalMapBuffer(device, camera_buffer, reinterpret_cast<void**>(&camera_ptr));
	assert(result == OPAL_SUCCESS);

	// per frame data
	for (uint32_t i = 0; i < IN_FLIGHT_FRAMES; ++i)
	{
		result = opalCreateCommandAllocator(device, queue, &command_allocators[i]);
		assert(result == OPAL_SUCCESS);

		result = opalCreateCommandBuffer(device, command_allocators[i], &command_buffers[i]);
		assert(result == OPAL_SUCCESS);

		Opal_TextureDesc frame_texture_desc = {};
		frame_texture_desc.type = OPAL_TEXTURE_TYPE_2D;
		frame_texture_desc.format = swapchain_desc.format.texture_format;
		frame_texture_desc.width = width;
		frame_texture_desc.height = height;
		frame_texture_desc.depth = 1;
		frame_texture_desc.mip_count = 1;
		frame_texture_desc.layer_count = 1;
		frame_texture_desc.samples = OPAL_SAMPLES_1;
		frame_texture_desc.usage = (Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_COPY_SRC | OPAL_TEXTURE_USAGE_UNORDERED_ACCESS);
		frame_texture_desc.hint = OPAL_ALLOCATION_HINT_AUTO;

		result = opalCreateTexture(device, &frame_texture_desc, &frame_textures[i]);
		assert(result == OPAL_SUCCESS);

		Opal_TextureViewDesc frame_texture_view_desc = {};
		frame_texture_view_desc.texture = frame_textures[i];
		frame_texture_view_desc.type = OPAL_TEXTURE_VIEW_TYPE_2D;
		frame_texture_view_desc.base_mip = 0;
		frame_texture_view_desc.mip_count = 1;
		frame_texture_view_desc.base_layer = 0;
		frame_texture_view_desc.layer_count = 1;

		result = opalCreateTextureView(device, &frame_texture_view_desc, &frame_texture_views[i]);
		assert(result == OPAL_SUCCESS);

		Opal_DescriptorSetEntry entries[] =
		{
			{0, camera_buffer, 0, camera_buffer_size},
			{1, tlas},
			{2, frame_texture_views[i]},
		};

		Opal_DescriptorSetAllocationDesc alloc_desc =
		{
			descriptor_set_layout,
			descriptor_heap,
			3,
			entries
		};

		result = opalAllocateDescriptorSet(device, &alloc_desc, &descriptor_sets[i]);
		assert(result == OPAL_SUCCESS);
	}

	current_frame = 0;
	wait_frame = 0;
}

void Application::shutdown()
{
	Opal_Result result = opalWaitIdle(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyAccelerationStructure(device, blas);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyAccelerationStructure(device, tlas);
	assert(result == OPAL_SUCCESS);

#ifndef OPAL_PLATFORM_MACOS
	result = opalDestroyShaderBindingTable(device, sbt);
	assert(result == OPAL_SUCCESS);
#endif

	result = opalDestroyBuffer(device, camera_buffer);
	assert(result == OPAL_SUCCESS);

	for (uint32_t i = 0; i < IN_FLIGHT_FRAMES; ++i)
	{
		result = opalDestroyTextureView(device, frame_texture_views[i]);
		assert(result == OPAL_SUCCESS);

		result = opalDestroyTexture(device, frame_textures[i]);
		assert(result == OPAL_SUCCESS);

		result = opalFreeDescriptorSet(device, descriptor_sets[i]);
		assert(result == OPAL_SUCCESS);

		result = opalDestroyCommandBuffer(device, command_buffers[i]);
		assert(result == OPAL_SUCCESS);

		result = opalDestroyCommandAllocator(device, command_allocators[i]);
		assert(result == OPAL_SUCCESS);
	}

	result = opalDestroyDescriptorHeap(device, descriptor_heap);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyDescriptorSetLayout(device, descriptor_set_layout);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyPipelineLayout(device, pipeline_layout);
	assert(result == OPAL_SUCCESS);

#ifndef OPAL_PLATFORM_MACOS
	result = opalDestroyRaytracePipeline(device, raytrace_pipeline);
	assert(result == OPAL_SUCCESS);
#else
	result = opalDestroyComputePipeline(device, compute_pipeline);
	assert(result == OPAL_SUCCESS);
#endif

	result = opalDestroyFence(device, fence);
	assert(result == OPAL_SUCCESS);

	result = opalDestroySemaphore(device, semaphore);
	assert(result == OPAL_SUCCESS);

	result = opalDestroySwapchain(device, swapchain);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyDevice(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroySurface(instance, surface);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyInstance(instance);
	assert(result == OPAL_SUCCESS);
}

void Application::update(float dt)
{
	constexpr float target[3] = {0.0f, 0.0f, 0.0f};
	constexpr float speed = 0.5f;
	
	static float angle = 0.0f;

	float pos[3] = {3.0f, 3.0f, 3.0f};
	pos[0] *= cos(angle);
	pos[1] *= sin(angle);

	float dir[3] = {target[0] - pos[0], target[1] - pos[1], target[2] - pos[2]};
	float l_inv = 1.0f / sqrtf(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);

	angle += speed * dt;

	camera_ptr->pos[0] = pos[0];
	camera_ptr->pos[1] = pos[1];
	camera_ptr->pos[2] = pos[2];
	camera_ptr->pos[3] = 0.0f;

	camera_ptr->dir[0] = dir[0] * l_inv;
	camera_ptr->dir[1] = dir[1] * l_inv;
	camera_ptr->dir[2] = dir[2] * l_inv;
	camera_ptr->dir[3] = 0.0f;
}

void Application::resize(uint32_t w, uint32_t h)
{
	if (device == OPAL_NULL_HANDLE)
		return;

	width = w;
	height = h;

	Opal_Result result = opalWaitIdle(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroySwapchain(device, swapchain);
	assert(result == OPAL_SUCCESS);

	for (uint32_t i = 0; i < IN_FLIGHT_FRAMES; ++i)
	{
		result = opalDestroyTextureView(device, frame_texture_views[i]);
		assert(result == OPAL_SUCCESS);

		result = opalDestroyTexture(device, frame_textures[i]);
		assert(result == OPAL_SUCCESS);

		frame_initialized[i] = false;
	}

	Opal_SwapchainDesc swapchain_desc = {};
	swapchain_desc.surface = surface;
	swapchain_desc.usage = (Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_FRAGMENT_SHADER_SAMPLED | OPAL_TEXTURE_USAGE_COPY_DST);

	result = opalGetPreferredSurfaceFormat(device, surface, &swapchain_desc.format);
	assert(result == OPAL_SUCCESS);

	result = opalGetPreferredSurfacePresentMode(device, surface, &swapchain_desc.mode);
	assert(result == OPAL_SUCCESS);

	result = opalCreateSwapchain(device, &swapchain_desc, &swapchain);
	assert(result == OPAL_SUCCESS);

	for (uint32_t i = 0; i < IN_FLIGHT_FRAMES; ++i)
	{
		Opal_TextureDesc frame_texture_desc = {};
		frame_texture_desc.type = OPAL_TEXTURE_TYPE_2D;
		frame_texture_desc.format = swapchain_desc.format.texture_format;
		frame_texture_desc.width = width;
		frame_texture_desc.height = height;
		frame_texture_desc.depth = 1;
		frame_texture_desc.mip_count = 1;
		frame_texture_desc.layer_count = 1;
		frame_texture_desc.samples = OPAL_SAMPLES_1;
		frame_texture_desc.usage = (Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_COPY_SRC | OPAL_TEXTURE_USAGE_UNORDERED_ACCESS);
		frame_texture_desc.hint = OPAL_ALLOCATION_HINT_AUTO;

		result = opalCreateTexture(device, &frame_texture_desc, &frame_textures[i]);
		assert(result == OPAL_SUCCESS);

		Opal_TextureViewDesc frame_texture_view_desc = {};
		frame_texture_view_desc.texture = frame_textures[i];
		frame_texture_view_desc.type = OPAL_TEXTURE_VIEW_TYPE_2D;
		frame_texture_view_desc.base_mip = 0;
		frame_texture_view_desc.mip_count = 1;
		frame_texture_view_desc.base_layer = 0;
		frame_texture_view_desc.layer_count = 1;

		result = opalCreateTextureView(device, &frame_texture_view_desc, &frame_texture_views[i]);
		assert(result == OPAL_SUCCESS);

		Opal_DescriptorSetEntry entries[] =
		{
			{2, frame_texture_views[i]},
		};

		result = opalUpdateDescriptorSet(device, descriptor_sets[i], 1, entries);
		assert(result == OPAL_SUCCESS);
	}
}

void Application::render()
{
	uint64_t index = current_frame % IN_FLIGHT_FRAMES;

	Opal_TextureView swapchain_texture_view = OPAL_NULL_HANDLE;
	Opal_CommandAllocator command_allocator = command_allocators[index];
	Opal_CommandBuffer command_buffer = command_buffers[index];

	Opal_TextureView frame_texture_view = frame_texture_views[index];
	Opal_DescriptorSet frame_descriptor_set = descriptor_sets[index];

	Opal_Result result = opalWaitSemaphore(device, semaphore, wait_frame, WAIT_TIMEOUT_MS);
	assert(result == OPAL_SUCCESS);

	result = opalResetCommandAllocator(device, command_allocator);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalCmdSetDescriptorHeap(device, command_buffer, descriptor_heap);
	assert(result == OPAL_SUCCESS);

	Opal_TextureTransitionDesc compute_begin_transitions[] =
	{
		frame_texture_view, OPAL_TEXTURE_STATE_UNDEFINED, OPAL_TEXTURE_STATE_UNORDERED_ACCESS,
	};

	Opal_BarrierDesc compute_begin_barrier = {};
	compute_begin_barrier.wait_stages = OPAL_BARRIER_STAGE_NONE;
	compute_begin_barrier.block_stages = OPAL_BARRIER_STAGE_ALL_COMPUTE;
	compute_begin_barrier.num_texture_transitions = 1;
	compute_begin_barrier.texture_transitions = compute_begin_transitions;

	Opal_PassBarriersDesc compute_begin = { 1, &compute_begin_barrier };

	Opal_PassBarriersDesc *compute_begin_ptr = frame_initialized[index] ? NULL : &compute_begin;
	frame_initialized[index] = true;

	Opal_TextureTransitionDesc copy_begin_transitions[] =
	{
		frame_texture_view, OPAL_TEXTURE_STATE_UNORDERED_ACCESS, OPAL_TEXTURE_STATE_COPY_SRC,
		swapchain_texture_view, OPAL_TEXTURE_STATE_UNDEFINED, OPAL_TEXTURE_STATE_COPY_DST,
	};

	Opal_BarrierDesc compute_to_copy_barrier = {};
	compute_to_copy_barrier.wait_stages = OPAL_BARRIER_STAGE_ALL_COMPUTE;
	compute_to_copy_barrier.block_stages = OPAL_BARRIER_STAGE_COPY;
	compute_to_copy_barrier.num_texture_transitions = 2;
	compute_to_copy_barrier.texture_transitions = copy_begin_transitions;
	compute_to_copy_barrier.fence = fence;
	compute_to_copy_barrier.fence_op = OPAL_FENCE_OP_BEGIN;

	Opal_PassBarriersDesc compute_end = { 1, &compute_to_copy_barrier };

#ifdef OPAL_PLATFORM_MACOS
	result = opalCmdBeginComputePass(device, command_buffer, compute_begin_ptr);
	assert(result == OPAL_SUCCESS);

	result = opalCmdComputeSetPipelineLayout(device, command_buffer, pipeline_layout);
	assert(result == OPAL_SUCCESS);

	result = opalCmdComputeSetDescriptorSet(device, command_buffer, 0, frame_descriptor_set, 0, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalCmdComputeSetPipeline(device, command_buffer, compute_pipeline);
	assert(result == OPAL_SUCCESS);

	result = opalCmdComputeDispatch(device, command_buffer, width, height, 1);
	assert(result == OPAL_SUCCESS);

	result = opalCmdEndComputePass(device, command_buffer, &compute_end);
	assert(result == OPAL_SUCCESS);
#else
	result = opalCmdBeginRaytracePass(device, command_buffer, compute_begin_ptr);
	assert(result == OPAL_SUCCESS);

	result = opalCmdRaytraceSetPipelineLayout(device, command_buffer, pipeline_layout);
	assert(result == OPAL_SUCCESS);

	result = opalCmdRaytraceSetDescriptorSet(device, command_buffer, 0, frame_descriptor_set, 0, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalCmdRaytraceSetPipeline(device, command_buffer, raytrace_pipeline);
	assert(result == OPAL_SUCCESS);

	result = opalCmdRaytraceSetShaderBindingTable(device, command_buffer, sbt);
	assert(result == OPAL_SUCCESS);

	result = opalCmdRaytraceDispatch(device, command_buffer, width, height, 1);
	assert(result == OPAL_SUCCESS);

	result = opalCmdEndRaytracePass(device, command_buffer, &compute_end);
	assert(result == OPAL_SUCCESS);
#endif

	result = opalAcquire(device, swapchain, &swapchain_texture_view);
	assert(result == OPAL_SUCCESS);

	compute_to_copy_barrier.fence_op = OPAL_FENCE_OP_END;

	Opal_PassBarriersDesc copy_begin = { 1, &compute_to_copy_barrier };

	Opal_TextureRegion src = { frame_texture_view, 0, 0, 0 };
	Opal_TextureRegion dst = { swapchain_texture_view, 0, 0, 0 };
	Opal_Extent3D size = { width, height, 1 };

	result = opalCmdBeginCopyPass(device, command_buffer, &copy_begin);
	assert(result == OPAL_SUCCESS);

	result = opalCmdCopyTextureToTexture(device, command_buffer, src, dst, size);
	assert(result == OPAL_SUCCESS);

	Opal_TextureTransitionDesc copy_end_transitions[] =
	{
		frame_texture_view, OPAL_TEXTURE_STATE_COPY_SRC, OPAL_TEXTURE_STATE_UNORDERED_ACCESS,
		swapchain_texture_view, OPAL_TEXTURE_STATE_COPY_DST, OPAL_TEXTURE_STATE_PRESENT,
	};

	Opal_BarrierDesc copy_end_barrier = {};
	copy_end_barrier.wait_stages = OPAL_BARRIER_STAGE_COPY;
	copy_end_barrier.block_stages = OPAL_BARRIER_STAGE_NONE;
	copy_end_barrier.num_texture_transitions = 2;
	copy_end_barrier.texture_transitions = copy_end_transitions;

	Opal_PassBarriersDesc copy_end = { 1, &copy_end_barrier };

	result = opalCmdEndCopyPass(device, command_buffer, &copy_end);
	assert(result == OPAL_SUCCESS);

	result = opalEndCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	uint64_t next_frame = current_frame + 1;

	Opal_SubmitDesc submit = {0};
	submit.num_wait_swapchains = 1;
	submit.wait_swapchains = &swapchain;
	submit.num_command_buffers = 1;
	submit.command_buffers = &command_buffer;
	submit.num_signal_swapchains = 1;
	submit.signal_swapchains = &swapchain;
	submit.num_signal_semaphores = 1;
	submit.signal_semaphores = &semaphore;
	submit.signal_values = &next_frame;

	result = opalSubmit(device, queue, &submit);
	assert(result == OPAL_SUCCESS);
}

void Application::present()
{
	Opal_Result result = opalPresent(device, swapchain);
	assert(result == OPAL_SUCCESS);
	UNUSED(result);

	current_frame++;
	if ((current_frame % IN_FLIGHT_FRAMES) == 0)
		wait_frame = current_frame;
}

/*
 */
void Application::buildPipelineLayout()
{
	// descriptor set layout
	Opal_DescriptorSetLayoutEntry layout_entries[] =
	{
		{0, OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER, OPAL_SHADER_STAGE_ALL_RAYTRACE, OPAL_TEXTURE_FORMAT_UNDEFINED},
		{1, OPAL_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE, OPAL_SHADER_STAGE_ALL_RAYTRACE, OPAL_TEXTURE_FORMAT_UNDEFINED},
		{2, OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_2D, OPAL_SHADER_STAGE_ALL_RAYTRACE, OPAL_TEXTURE_FORMAT_BGRA8_UNORM},
	};

	Opal_Result result = opalCreateDescriptorSetLayout(device, 3, layout_entries, &descriptor_set_layout);
	assert(result == OPAL_SUCCESS);

	// descriptor heap
	Opal_DescriptorHeapDesc heap_desc =
	{
		32,
		0,
	};

	result = opalCreateDescriptorHeap(device, &heap_desc, &descriptor_heap);
	assert(result == OPAL_SUCCESS);

	// pipeline layout
	result = opalCreatePipelineLayout(device, 1, &descriptor_set_layout, &pipeline_layout);
	assert(result == OPAL_SUCCESS);
}

#ifdef OPAL_PLATFORM_MACOS
void Application::buildPipeline()
{
	// shaders
	Opal_Shader shaders[1] =
	{
		OPAL_NULL_HANDLE,
	};

	const char *paths[1] =
	{
		compute_shader_paths[target_api],
	};

	Opal_ShaderSourceType shader_type = shader_types[target_api];

	for (uint32_t i = 0; i < 1; ++i)
	{
		bool success = loadShader(paths[i], shader_type, device, &shaders[i]);
		assert(success);
	}

	// pipeline
	Opal_ComputePipelineDesc pipeline_desc = {};
	pipeline_desc.pipeline_layout = pipeline_layout;

	pipeline_desc.compute_function = {shaders[0], "computeMain"};
	pipeline_desc.threadgroup_size_x = 1;
	pipeline_desc.threadgroup_size_y = 1;
	pipeline_desc.threadgroup_size_z = 1;

	Opal_Result result = opalCreateComputePipeline(device, &pipeline_desc, &compute_pipeline);
	assert(result == OPAL_SUCCESS);

	for (uint32_t i = 0; i < 1; ++i)
	{
		result = opalDestroyShader(device, shaders[i]);
		assert(result == OPAL_SUCCESS);
	}
}
#else
void Application::buildPipeline()
{
	// shaders
	Opal_Shader shaders[3] =
	{
		OPAL_NULL_HANDLE,
		OPAL_NULL_HANDLE,
		OPAL_NULL_HANDLE,
	};

	const char *paths[3] =
	{
		raygen_shader_paths[target_api],
		closest_hit_shader_paths[target_api],
		miss_shader_paths[target_api],
	};

	Opal_ShaderSourceType shader_type = shader_types[target_api];

	for (uint32_t i = 0; i < 3; ++i)
	{
		bool success = loadShader(paths[i], shader_type, device, &shaders[i]);
		assert(success);
	}

	// pipeline
	Opal_RaytracePipelineDesc pipeline_desc = {};
	pipeline_desc.pipeline_layout = pipeline_layout;

	Opal_ShaderFunction raygen_functions[] = {{shaders[0], "rayGenerationMain"}};
	Opal_ShaderIntersectionGroup intersection_functions[] = {{{}, {}, {shaders[1], "rayClosestHitMain"}}};
	Opal_ShaderFunction miss_functions[] = {{shaders[2], "rayMissMain"}};

	pipeline_desc.num_raygen_functions = 1;
	pipeline_desc.raygen_functions = raygen_functions;

	pipeline_desc.num_miss_functions = 1;
	pipeline_desc.miss_functions = miss_functions;

	pipeline_desc.num_intersection_functions = 1;
	pipeline_desc.intersection_functions = intersection_functions;

	pipeline_desc.max_recursion_depth = 1;
	pipeline_desc.max_ray_payload_size = sizeof(float) * 4;
	pipeline_desc.max_hit_attribute_size = sizeof(float) * 2;

	Opal_Result result = opalCreateRaytracePipeline(device, &pipeline_desc, &raytrace_pipeline);
	assert(result == OPAL_SUCCESS);

	for (uint32_t i = 0; i < 3; ++i)
	{
		result = opalDestroyShader(device, shaders[i]);
		assert(result == OPAL_SUCCESS);
	}
}

void Application::buildSBT()
{
	Opal_Result result = opalCreateShaderBindingTable(device, raytrace_pipeline, &sbt);
	assert(result == OPAL_SUCCESS);

	Opal_ShaderBindingTableBuildDesc sbt_build_desc = {};

	uint32_t raygen_indices[] = {0};
	uint32_t miss_indices[] = {0};
	uint32_t intersection_indices[] = {0};

	sbt_build_desc.num_raygen_indices = 1;
	sbt_build_desc.raygen_indices = raygen_indices;

	sbt_build_desc.num_miss_indices = 1;
	sbt_build_desc.miss_indices = miss_indices;

	sbt_build_desc.num_intersection_indices = 1;
	sbt_build_desc.intersection_indices = intersection_indices;

	result = opalBuildShaderBindingTable(device, sbt, &sbt_build_desc);
	assert(result == OPAL_SUCCESS);
}
#endif

void Application::buildBLAS()
{
	uint32_t num_vertices = 3;
	uint32_t num_indices = 3;
	uint32_t vertex_stride = sizeof(float) * 3;

	// create geometry
	Opal_BufferDesc buffer_desc = {};
	buffer_desc.usage = OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT;
	buffer_desc.size = sizeof(Triangle);
	buffer_desc.memory_type = OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD;
	buffer_desc.hint = OPAL_ALLOCATION_HINT_AUTO;

	Opal_Buffer triangle_buffer = OPAL_NULL_HANDLE;
	Opal_Result result = opalCreateBuffer(device, &buffer_desc, &triangle_buffer);
	assert(result == OPAL_SUCCESS);

	void *ptr = nullptr;
	result = opalMapBuffer(device, triangle_buffer, &ptr);
	assert(result == OPAL_SUCCESS);

	memcpy(ptr, &triangle_data, sizeof(Triangle));

	result = opalUnmapBuffer(device, triangle_buffer);
	assert(result == OPAL_SUCCESS);

	// fetch sizes
	Opal_AccelerationStructureGeometry blas_geometry = {};
	blas_geometry.type = OPAL_ACCELERATION_STRUCTURE_GEOMETRY_TYPE_TRIANGLES;
	blas_geometry.flags = OPAL_ACCELERATION_STRUCTURE_GEOMETRY_FLAGS_OPAQUE;
	blas_geometry.data.triangles.num_vertices = num_vertices;
	blas_geometry.data.triangles.num_indices = num_indices;
	blas_geometry.data.triangles.vertex_format = OPAL_VERTEX_FORMAT_RGB32_SFLOAT;
	blas_geometry.data.triangles.index_format = OPAL_INDEX_FORMAT_UINT32;
	blas_geometry.data.triangles.vertex_stride = vertex_stride;
	blas_geometry.data.triangles.vertex_buffer = {triangle_buffer, offsetof(Triangle, vertices), vertex_stride * num_vertices};
	blas_geometry.data.triangles.index_buffer = {triangle_buffer, offsetof(Triangle, indices), sizeof(uint32_t) * num_indices};

	Opal_AccelerationStructureBuildDesc blas_build_desc = {};
	blas_build_desc.type = OPAL_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	blas_build_desc.build_flags = OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_PREFER_FAST_TRACE;
	blas_build_desc.build_mode = OPAL_ACCELERATION_STRUCTURE_BUILD_MODE_BUILD;
	blas_build_desc.input.bottom_level.num_geometries = 1;
	blas_build_desc.input.bottom_level.geometries = &blas_geometry;

	Opal_AccelerationStructurePrebuildInfo blas_build_info = {};

	result = opalGetAccelerationStructurePrebuildInfo(device, &blas_build_desc, &blas_build_info);
	assert(result == OPAL_SUCCESS);

	// create acceleration structure
	buffer_desc = {};
	buffer_desc.size = blas_build_info.build_scratch_size;
	buffer_desc.memory_type = OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL;
	buffer_desc.hint = OPAL_ALLOCATION_HINT_AUTO;
	buffer_desc.usage = (Opal_BufferUsageFlags)(OPAL_BUFFER_USAGE_UNORDERED_ACCESS);
	buffer_desc.initial_state = OPAL_BUFFER_STATE_UNORDERED_ACCESS;

	Opal_Buffer scratch_buffer = OPAL_NULL_HANDLE;
	result = opalCreateBuffer(device, &buffer_desc, &scratch_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_AccelerationStructureDesc blas_desc = {};
	blas_desc.type = OPAL_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	blas_desc.size = blas_build_info.acceleration_structure_size;

	result = opalCreateAccelerationStructure(device, &blas_desc, &blas);
	assert(result == OPAL_SUCCESS);

	// build acceleration structure
	blas_build_desc.dst_acceleration_structure = blas;
	blas_build_desc.scratch_buffer = {scratch_buffer, 0, blas_build_info.build_scratch_size};

	Opal_CommandAllocator command_allocator = OPAL_NULL_HANDLE;
	result = opalCreateCommandAllocator(device, queue, &command_allocator);
	assert(result == OPAL_SUCCESS);

	Opal_CommandBuffer command_buffer = OPAL_NULL_HANDLE;
	result = opalCreateCommandBuffer(device, command_allocator, &command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalCmdBeginAccelerationStructurePass(device, command_buffer, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalCmdAccelerationStructureBuild(device, command_buffer, &blas_build_desc);
	assert(result == OPAL_SUCCESS);

	result = opalCmdEndAccelerationStructurePass(device, command_buffer, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalEndCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_SubmitDesc submit_desc = {};
	submit_desc.num_command_buffers = 1;
	submit_desc.command_buffers = &command_buffer;

	result = opalSubmit(device, queue, &submit_desc);
	assert(result == OPAL_SUCCESS);

	result = opalWaitIdle(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyCommandAllocator(device, command_allocator);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, triangle_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, scratch_buffer);
	assert(result == OPAL_SUCCESS);
}

void Application::buildTLAS()
{
	Opal_AccelerationStructureInstance tlas_instance = {};
	tlas_instance.transform[0][0] = 1.0f;
	tlas_instance.transform[1][1] = 1.0f;
	tlas_instance.transform[2][2] = 1.0f;
	tlas_instance.blas = blas;
	tlas_instance.mask = 0xFF;

	Opal_BufferDesc buffer_desc = {};
	buffer_desc.size = device_info.features.acceleration_structure_instance_size;
	buffer_desc.memory_type = OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD;
	buffer_desc.hint = OPAL_ALLOCATION_HINT_AUTO;
	buffer_desc.usage = OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT;
	buffer_desc.initial_state = OPAL_BUFFER_STATE_GENERIC_READ;

	Opal_Buffer instance_buffer = OPAL_NULL_HANDLE;
	Opal_Result result = opalCreateBuffer(device, &buffer_desc, &instance_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_AccelerationStructureInstanceBufferBuildDesc instance_build_desc = {};
	instance_build_desc.buffer = {instance_buffer, 0, sizeof(Opal_AccelerationStructureInstance)};
	instance_build_desc.num_instances = 1;
	instance_build_desc.instances = &tlas_instance;

	result = opalBuildAccelerationStructureInstanceBuffer(device, &instance_build_desc);
	assert(result == OPAL_SUCCESS);

	// fetch size
	Opal_AccelerationStructureBuildDesc tlas_build_desc = {};
	tlas_build_desc.type = OPAL_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	tlas_build_desc.build_flags = OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_PREFER_FAST_TRACE;
	tlas_build_desc.build_mode = OPAL_ACCELERATION_STRUCTURE_BUILD_MODE_BUILD;
	tlas_build_desc.input.top_level.num_instances = 1;
	tlas_build_desc.input.top_level.instance_buffer = {instance_buffer, 0, sizeof(Opal_AccelerationStructureInstance)};

	Opal_AccelerationStructurePrebuildInfo tlas_build_info = {};

	result = opalGetAccelerationStructurePrebuildInfo(device, &tlas_build_desc, &tlas_build_info);
	assert(result == OPAL_SUCCESS);

	// create acceleration structure
	buffer_desc = {};
	buffer_desc.size = tlas_build_info.build_scratch_size;
	buffer_desc.memory_type = OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL;
	buffer_desc.hint = OPAL_ALLOCATION_HINT_AUTO;
	buffer_desc.usage = (Opal_BufferUsageFlags)(OPAL_BUFFER_USAGE_UNORDERED_ACCESS);
	buffer_desc.initial_state = OPAL_BUFFER_STATE_UNORDERED_ACCESS;

	Opal_Buffer scratch_buffer = OPAL_NULL_HANDLE;
	result = opalCreateBuffer(device, &buffer_desc, &scratch_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_AccelerationStructureDesc tlas_desc = {};
	tlas_desc.type = OPAL_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	tlas_desc.size = tlas_build_info.acceleration_structure_size;

	result = opalCreateAccelerationStructure(device, &tlas_desc, &tlas);
	assert(result == OPAL_SUCCESS);

	// build acceleration structure
	tlas_build_desc.dst_acceleration_structure = tlas;
	tlas_build_desc.scratch_buffer = {scratch_buffer, 0, tlas_build_info.build_scratch_size};

	Opal_CommandAllocator command_allocator = OPAL_NULL_HANDLE;
	result = opalCreateCommandAllocator(device, queue, &command_allocator);
	assert(result == OPAL_SUCCESS);

	Opal_CommandBuffer command_buffer = OPAL_NULL_HANDLE;
	result = opalCreateCommandBuffer(device, command_allocator, &command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalCmdBeginAccelerationStructurePass(device, command_buffer, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalCmdAccelerationStructureBuild(device, command_buffer, &tlas_build_desc);
	assert(result == OPAL_SUCCESS);

	result = opalCmdEndAccelerationStructurePass(device, command_buffer, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalEndCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_SubmitDesc submit_desc = {};
	submit_desc.num_command_buffers = 1;
	submit_desc.command_buffers = &command_buffer;

	result = opalSubmit(device, queue, &submit_desc);
	assert(result == OPAL_SUCCESS);

	result = opalWaitIdle(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyCommandAllocator(device, command_allocator);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, instance_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, scratch_buffer);
	assert(result == OPAL_SUCCESS);
}
