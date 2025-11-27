#include "app.h"

#include <cstddef>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cmath>

#ifdef OPAL_PLATFORM_WINDOWS
static constexpr Opal_Api target_api = OPAL_API_VULKAN;
#elif OPAL_PLATFORM_MACOS
static constexpr Opal_Api target_api = OPAL_API_METAL;
#elif OPAL_PLATFORM_WEB
static constexpr Opal_Api target_api = OPAL_API_WEBGPU;
#endif

#define UNUSED(x) do { (void)(x); } while(0)

static const char *emit_shader_paths[] =
{
	"samples/06_gpu_particles/shaders/vulkan/emit.comp.spv",
	"samples/06_gpu_particles/shaders/directx12/emit.comp.cso",
	nullptr,
	nullptr,
	nullptr,
};

static const char *simulate_shader_paths[] =
{
	"samples/06_gpu_particles/shaders/vulkan/simulate.comp.spv",
	"samples/06_gpu_particles/shaders/directx12/simulate.comp.cso",
	nullptr,
	nullptr,
	nullptr,
};

static const char *vertex_shader_paths[] =
{
	"samples/06_gpu_particles/shaders/vulkan/render.vert.spv",
	"samples/06_gpu_particles/shaders/directx12/render.vert.cso",
	nullptr,
	nullptr,
	nullptr,
};

static const char *fragment_shader_paths[] =
{
	"samples/06_gpu_particles/shaders/vulkan/render.frag.spv",
	"samples/06_gpu_particles/shaders/directx12/render.frag.cso",
	nullptr,
	nullptr,
	nullptr,
};

static Opal_ShaderSourceType shader_types[] =
{
	OPAL_SHADER_SOURCE_TYPE_SPIRV_BINARY,
	OPAL_SHADER_SOURCE_TYPE_DXIL_BINARY,
	OPAL_SHADER_SOURCE_TYPE_METALLIB_BINARY,
	OPAL_SHADER_SOURCE_TYPE_WGSL_SOURCE,
	OPAL_SHADER_SOURCE_ENUM_FORCE32,
};

struct LogoMeshData
{
	float vertices[80];
	uint32_t indices[120];
};

static constexpr uint32_t LOGO_NUM_VERTICES = 20;
static constexpr uint32_t LOGO_NUM_INDICES = 120;

static LogoMeshData logo =
{
	// vertex data
	1.953087f, -0.257403f, 2.370207f, 1.0f,
	-1.366913f, -0.257403f, 2.370207f, 1.0f,
	-1.366913f, -0.257403f, -3.449793f, 1.0f,
	1.953087f, -0.257403f, -0.334793f, 1.0f,
	-1.366913f, -0.257403f, -0.334793f, 1.0f,
	-0.977783f, -0.257403f, 1.981077f, 1.0f,
	1.563957f, -0.257403f, 1.981077f, 1.0f,
	-0.977783f, -0.257403f, -2.551098f, 1.0f,
	1.563957f, -0.257403f, -0.166302f, 1.0f,
	-0.977783f, -0.257403f, -0.334793f, 1.0f,
	-0.977783f, 0.286478f, 1.981077f, 1.0f,
	1.563957f, 0.286478f, 1.981077f, 1.0f,
	-0.977783f, 0.286478f, -2.551098f, 1.0f,
	1.563957f, 0.286478f, -0.166302f, 1.0f,
	-0.977783f, 0.286478f, -0.334793f, 1.0f,
	-1.366913f, 0.286478f, 2.370207f, 1.0f,
	1.953087f, 0.286478f, 2.370207f, 1.0f,
	-1.366913f, 0.286478f, -3.449793f, 1.0f,
	1.953087f, 0.286478f, -0.334793f, 1.0f,
	-1.366913f, 0.286478f, -0.334793f, 1.0f,

	// index data
	12, 18, 17,
	10, 16, 11,
	0, 5, 6,
	7, 3, 8,
	5, 4, 9,
	3, 6, 8,
	4, 7, 9,
	10, 19, 15,
	11, 18, 13,
	12, 19, 14,
	0, 15, 1,
	6, 13, 8,
	4, 17, 2,
	5, 11, 6,
	2, 18, 3,
	7, 14, 9,
	8, 12, 7,
	1, 19, 4,
	9, 10, 5,
	3, 16, 0,
	12, 13, 18,
	10, 15, 16,
	0, 1, 5,
	7, 2, 3,
	5, 1, 4,
	3, 0, 6,
	4, 2, 7,
	10, 14, 19,
	11, 16, 18,
	12, 17, 19,
	0, 16, 15,
	6, 11, 13,
	4, 19, 17,
	5, 10, 11,
	2, 17, 18,
	7, 12, 14,
	8, 13, 12,
	1, 15, 19,
	9, 14, 10,
	3, 18, 16,
};

/*
 */
struct vec3
{
	float x, y, z;
};

inline float dot(const vec3 &v0, const vec3 &v1)
{
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

inline vec3 add(const vec3 &v0, const vec3 &v1)
{
	return { v0.x + v1.x, v0.y + v1.y, v0.z + v1.z };
}

inline vec3 sub(const vec3 &v0, const vec3 &v1)
{
	return { v0.x - v1.x, v0.y - v1.y, v0.z - v1.z };
}

inline vec3 cross(const vec3 &v0, const vec3 &v1)
{
	return { v0.y * v1.z - v1.y * v0.z, v0.z * v1.x  - v1.z * v0.x, v0.x * v1.y - v1.x * v0.y };
}

inline vec3 normalize(const vec3 &v)
{
	float l_inv = 1.0f / sqrtf(dot(v, v));
	return { v.x * l_inv, v.y * l_inv, v.z * l_inv };
}

/*
 */
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

bool uploadDataToBuffer(Opal_Device device, Opal_Queue queue, const void *data, size_t size, Opal_BufferState target_state, Opal_Buffer destination_buffer)
{
	// staging buffer
	Opal_BufferDesc staging_buffer_desc =
	{
		size,
		OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
		OPAL_ALLOCATION_HINT_AUTO,
		static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_COPY_SRC),
		OPAL_BUFFER_STATE_GENERIC_READ,
	};

	Opal_Buffer staging_buffer = OPAL_NULL_HANDLE;
	Opal_Result result = opalCreateBuffer(device, &staging_buffer_desc, &staging_buffer);
	assert(result == OPAL_SUCCESS);

	// copy
	result = opalWriteBuffer(device, staging_buffer, 0, data, size);
	assert(result == OPAL_SUCCESS);

	// transfer
	Opal_CommandAllocator staging_command_allocator = OPAL_NULL_HANDLE;
	result = opalCreateCommandAllocator(device, queue, &staging_command_allocator);
	assert(result == OPAL_SUCCESS);

	Opal_CommandBuffer staging_command_buffer = OPAL_NULL_HANDLE;
	result = opalCreateCommandBuffer(device, staging_command_allocator, &staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommandBuffer(device, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_BufferTransitionDesc copy_begin_transitions[] =
	{
		{ destination_buffer, target_state, OPAL_BUFFER_STATE_COPY_DST },
	};

	Opal_BarrierDesc copy_begin_barrier = {};
	copy_begin_barrier.wait_stages = OPAL_BARRIER_STAGE_NONE;
	copy_begin_barrier.block_stages = OPAL_BARRIER_STAGE_COPY;
	copy_begin_barrier.num_buffer_transitions = 1;
	copy_begin_barrier.buffer_transitions = copy_begin_transitions;

	Opal_PassBarriersDesc copy_begin = {1, &copy_begin_barrier};

	result = opalCmdBeginCopyPass(device, staging_command_buffer, &copy_begin);
	assert(result == OPAL_SUCCESS);

	result = opalCmdCopyBufferToBuffer(device, staging_command_buffer, staging_buffer, 0, destination_buffer, 0, size);
	assert(result == OPAL_SUCCESS);

	Opal_BufferTransitionDesc copy_end_transitions[] =
	{
		{ destination_buffer, OPAL_BUFFER_STATE_COPY_DST, target_state },
	};

	Opal_BarrierDesc copy_end_barrier = {};
	copy_end_barrier.wait_stages = OPAL_BARRIER_STAGE_COPY;
	copy_end_barrier.block_stages = OPAL_BARRIER_STAGE_NONE;
	copy_end_barrier.num_buffer_transitions = 1;
	copy_end_barrier.buffer_transitions = copy_end_transitions;

	Opal_PassBarriersDesc copy_end = {1, &copy_end_barrier};

	result = opalCmdEndCopyPass(device, staging_command_buffer, &copy_end);
	assert(result == OPAL_SUCCESS);

	result = opalEndCommandBuffer(device, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_SubmitDesc submit = {};
	submit.num_command_buffers = 1;
	submit.command_buffers = &staging_command_buffer;

	result = opalSubmit(device, queue, &submit);
	assert(result == OPAL_SUCCESS);

	result = opalWaitQueue(device, queue);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyCommandBuffer(device, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyCommandAllocator(device, staging_command_allocator);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, staging_buffer);
	assert(result == OPAL_SUCCESS);

	return true;
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
		"06_gpu_particles",
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

	result = opalGetDeviceQueue(device, OPAL_DEVICE_ENGINE_TYPE_MAIN, 0, &queue);
	assert(result == OPAL_SUCCESS);

	Opal_SwapchainDesc swapchain_desc = {};
	swapchain_desc.surface = surface;
	swapchain_desc.queue = queue;
	swapchain_desc.usage = (Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_FRAGMENT_SHADER_SAMPLED | OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT);

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

	// fence
	result = opalCreateFence(device, &fence);
	assert(result == OPAL_SUCCESS);

	// command allocator & command buffer
	result = opalCreateCommandAllocator(device, queue, &command_allocator);
	assert(result == OPAL_SUCCESS);

	result = opalCreateCommandBuffer(device, command_allocator, &command_buffer);
	assert(result == OPAL_SUCCESS);

	// descriptor heap
	Opal_DescriptorHeapDesc descriptor_heap_desc = {};
	descriptor_heap_desc.num_resource_descriptors = 1024;

	result = opalCreateDescriptorHeap(device, &descriptor_heap_desc, &descriptor_heap);
	assert(result == OPAL_SUCCESS);

	// particles
	buildLayout();
	buildContent();
	buildDescriptors();
	buildPipelines();

	current_frame = 0;
}

void Application::shutdown()
{
	Opal_Result result = opalWaitIdle(device);
	assert(result == OPAL_SUCCESS);

	destroyPipelines();
	destroyDescriptors();
	destroyContent();
	destroyLayout();

	result = opalDestroyDescriptorHeap(device, descriptor_heap);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyFence(device, fence);
	assert(result == OPAL_SUCCESS);

	result = opalDestroySemaphore(device, semaphore);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyCommandAllocator(device, command_allocator);
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
	static float angle = 90.0f;

	AppData application =
	{
		static_cast<float>(width),
		static_cast<float>(height),
		static_cast<float>(1.0f / width),
		static_cast<float>(1.0f / height),
		current_time,
		dt,
	};

	CameraData camera = {};

	{
		const float fov = 60.0f * DEG2RAD;
		const float znear = 0.01f;
		const float zfar = 1000.0f;
		const float aspect = static_cast<float>(width) / height;

		const float tan_fov = tanf(fov * 0.5f);

		camera.projection[0] = 1.0f / (aspect * tan_fov);
		camera.projection[1] = 0.0f;
		camera.projection[2] = 0.0f;
		camera.projection[3] = 0.0f;

		camera.projection[4] = 0.0f;
		camera.projection[5] = 1.0f / (tan_fov);
		camera.projection[6] = 0.0f;
		camera.projection[7] = 0.0f;

		camera.projection[8] = 0.0f;
		camera.projection[9] = 0.0f;
		camera.projection[10] = zfar / (znear - zfar);
		camera.projection[11] = -1.0f;

		camera.projection[12] = 0.0f;
		camera.projection[13] = 0.0f;
		camera.projection[14] = -(zfar * znear) / (zfar - znear);
		camera.projection[15] = 0.0f;

		if constexpr (target_api == OPAL_API_VULKAN)
			camera.projection[5] *= -1.0f;
	}

	{
		const float zoom = 6.0;

		float sin_theta = sinf(angle * DEG2RAD);
		float cos_theta = cosf(angle * DEG2RAD);

		float sin_phi = 0.0f;
		float cos_phi = 1.0f;

		vec3 eye = { cos_theta * cos_phi * zoom, sin_theta * cos_phi * zoom, sin_phi * zoom };
		vec3 center = {};
		vec3 up = { 0.0f, 0.0f, 1.0f };

		const vec3 &f = normalize(sub(center, eye));
		const vec3 &s = normalize(cross(f, up));
		const vec3 &u = cross(s, f);

		camera.view[0] = s.x;
		camera.view[1] = u.x;
		camera.view[2] = -f.x;
		camera.view[3] = 0.0f;

		camera.view[4] = s.y;
		camera.view[5] = u.y;
		camera.view[6] = -f.y;
		camera.view[7] = 0.0f;

		camera.view[8] = s.z;
		camera.view[9] = u.z;
		camera.view[10] = -f.z;
		camera.view[11] = 0.0f;

		camera.view[12] =-dot(s, eye);
		camera.view[13] =-dot(u, eye);
		camera.view[14] = dot(f, eye);
		camera.view[15] = 1.0f;
	}

	opalWriteBuffer(device, render_application, 0, &application, sizeof(AppData));
	opalWriteBuffer(device, render_camera, 0, &camera, sizeof(CameraData));

	angle += dt * 2.0f;
	current_time += dt;
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

	Opal_SwapchainDesc swapchain_desc = {};
	swapchain_desc.surface = surface;
	swapchain_desc.queue = queue;
	swapchain_desc.usage = (Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_FRAGMENT_SHADER_SAMPLED | OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT);

	result = opalGetPreferredSurfaceFormat(device, surface, &swapchain_desc.format);
	assert(result == OPAL_SUCCESS);

	result = opalGetPreferredSurfacePresentMode(device, surface, &swapchain_desc.mode);
	assert(result == OPAL_SUCCESS);

	result = opalCreateSwapchain(device, &swapchain_desc, &swapchain);
	assert(result == OPAL_SUCCESS);
}

void Application::render()
{
	Opal_TextureView swapchain_texture_view = OPAL_NULL_HANDLE;

	Opal_Result result = opalAcquire(device, swapchain, &swapchain_texture_view);
	assert(result == OPAL_SUCCESS);

	result = opalWaitSemaphore(device, semaphore, current_frame, WAIT_TIMEOUT_MS);
	assert(result == OPAL_SUCCESS);

	result = opalResetCommandAllocator(device, command_allocator);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalCmdSetDescriptorHeap(device, command_buffer, descriptor_heap);
	assert(result == OPAL_SUCCESS);

	result = opalCmdBeginComputePass(device, command_buffer, NULL);
	assert(result == OPAL_SUCCESS);

	uint32_t num_triangles = LOGO_NUM_INDICES / 3;
	result = opalCmdComputeSetPipelineLayout(device, command_buffer, compute_pipeline_layout);
	assert(result == OPAL_SUCCESS);

	result = opalCmdComputeSetDescriptorSet(device, command_buffer, 0, render_descriptor_set, 0, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalCmdComputeSetDescriptorSet(device, command_buffer, 1, compute_descriptor_set, 0, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalCmdComputeSetPipeline(device, command_buffer, emit_pipeline);
	assert(result == OPAL_SUCCESS);

	result = opalCmdComputeDispatch(device, command_buffer, num_triangles, 1, 1);
	assert(result == OPAL_SUCCESS);

	Opal_Buffer sync_buffers[] = { particle_positions, particle_velocities, particle_parameters, particle_indices, particle_counters };
	Opal_MemoryBarrierDesc barriers = {};
	barriers.num_buffers = 5;
	barriers.buffers = sync_buffers;

	result = opalCmdComputeMemoryBarrier(device, command_buffer, &barriers);
	assert(result == OPAL_SUCCESS);

	uint32_t num_workgroups = (NUM_PARTICLES + 31) / 32;

	result = opalCmdComputeSetPipeline(device, command_buffer, simulate_pipeline);
	assert(result == OPAL_SUCCESS);

	result = opalCmdComputeDispatch(device, command_buffer, num_workgroups, 1, 1);
	assert(result == OPAL_SUCCESS);

	Opal_TextureTransitionDesc compute_to_graphics_texture_transitions[] =
	{
		{ swapchain_texture_view, OPAL_TEXTURE_STATE_UNDEFINED, OPAL_TEXTURE_STATE_FRAMEBUFFER_ATTACHMENT },
	};

	Opal_BufferTransitionDesc compute_to_graphics_buffer_transitions[] =
	{
		{ particle_positions, OPAL_BUFFER_STATE_UNORDERED_ACCESS, OPAL_BUFFER_STATE_GENERIC_READ },
	};

	Opal_BarrierDesc compute_end_barrier = {};
	compute_end_barrier.wait_stages = OPAL_BARRIER_STAGE_COMPUTE;
	compute_end_barrier.block_stages = OPAL_BARRIER_STAGE_GRAPHICS_VERTEX;
	compute_end_barrier.num_buffer_transitions = 1;
	compute_end_barrier.buffer_transitions = compute_to_graphics_buffer_transitions;
	compute_end_barrier.num_texture_transitions = 1;
	compute_end_barrier.texture_transitions = compute_to_graphics_texture_transitions;
	compute_end_barrier.fence = fence;
	compute_end_barrier.fence_op = OPAL_FENCE_OP_BEGIN;

	Opal_PassBarriersDesc compute_end = { 1, &compute_end_barrier };

	result = opalCmdEndComputePass(device, command_buffer, &compute_end);
	assert(result == OPAL_SUCCESS);

	Opal_BarrierDesc graphics_begin_barrier = {};
	graphics_begin_barrier.wait_stages = OPAL_BARRIER_STAGE_COMPUTE;
	graphics_begin_barrier.block_stages = OPAL_BARRIER_STAGE_GRAPHICS_VERTEX;
	graphics_begin_barrier.num_buffer_transitions = 1;
	graphics_begin_barrier.buffer_transitions = compute_to_graphics_buffer_transitions;
	graphics_begin_barrier.num_texture_transitions = 1;
	graphics_begin_barrier.texture_transitions = compute_to_graphics_texture_transitions;
	compute_end_barrier.fence = fence;
	graphics_begin_barrier.fence_op = OPAL_FENCE_OP_END;

	Opal_PassBarriersDesc graphics_begin = { 1, &graphics_begin_barrier };

	Opal_ClearColor clear_color = {0.15f, 0.15f, 0.15f, 1.0f};
	Opal_FramebufferAttachment attachments[] =
	{
		{ swapchain_texture_view, OPAL_NULL_HANDLE, OPAL_LOAD_OP_CLEAR, OPAL_STORE_OP_STORE, clear_color },
	};

	Opal_FramebufferDesc framebuffer = { 1, attachments, NULL };

	result = opalCmdBeginGraphicsPass(device, command_buffer, &framebuffer, &graphics_begin);
	assert(result == OPAL_SUCCESS);

	result = opalCmdGraphicsSetPipelineLayout(device, command_buffer, render_pipeline_layout);
	assert(result == OPAL_SUCCESS);

	result = opalCmdGraphicsSetDescriptorSet(device, command_buffer, 0, render_descriptor_set, 0, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalCmdGraphicsSetPipeline(device, command_buffer, render_pipeline);
	assert(result == OPAL_SUCCESS);

	Opal_VertexBufferView vertex_buffer = { particle_positions, 0, NUM_PARTICLES * sizeof(float) * 4, sizeof(float) * 4 };

	result = opalCmdGraphicsSetVertexBuffers(device, command_buffer, 1, &vertex_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_Viewport viewport =
	{
		0, 0,
		static_cast<float>(width), static_cast<float>(height),
		0.0f, 1.0f,
	};

	result = opalCmdGraphicsSetViewport(device, command_buffer, viewport);
	assert(result == OPAL_SUCCESS);

	result = opalCmdGraphicsSetScissor(device, command_buffer, 0, 0, width, height);
	assert(result == OPAL_SUCCESS);

	result = opalCmdGraphicsDraw(device, command_buffer, 6, NUM_PARTICLES, 0, 0);
	assert(result == OPAL_SUCCESS);

	Opal_BufferTransitionDesc graphics_end_buffer_transitions[] =
	{
		{ particle_positions, OPAL_BUFFER_STATE_GENERIC_READ, OPAL_BUFFER_STATE_UNORDERED_ACCESS },
	};

	Opal_TextureTransitionDesc graphics_end_texture_transitions[] =
	{
		{ swapchain_texture_view, OPAL_TEXTURE_STATE_FRAMEBUFFER_ATTACHMENT, OPAL_TEXTURE_STATE_PRESENT },
	};

	Opal_BarrierDesc graphics_end_barrier = {};
	graphics_end_barrier.wait_stages = static_cast<Opal_BarrierStageFlags>(OPAL_BARRIER_STAGE_GRAPHICS_FRAGMENT | OPAL_BARRIER_STAGE_GRAPHICS_VERTEX);
	graphics_end_barrier.block_stages = OPAL_BARRIER_STAGE_NONE;
	graphics_end_barrier.num_buffer_transitions = 1;
	graphics_end_barrier.buffer_transitions = graphics_end_buffer_transitions;
	graphics_end_barrier.num_texture_transitions = 1;
	graphics_end_barrier.texture_transitions = graphics_end_texture_transitions;

	Opal_PassBarriersDesc graphics_end = { 1, &graphics_end_barrier };

	result = opalCmdEndGraphicsPass(device, command_buffer, &graphics_end);
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
}

/*
 */
void Application::buildLayout()
{
	// render
	{
		Opal_DescriptorSetLayoutEntry layout_bindings[] =
		{
			0, OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER, OPAL_SHADER_STAGE_ALL, OPAL_TEXTURE_FORMAT_UNDEFINED, // application
			1, OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER, OPAL_SHADER_STAGE_ALL, OPAL_TEXTURE_FORMAT_UNDEFINED, // camera
		};

		Opal_Result result = opalCreateDescriptorSetLayout(device, 2, layout_bindings, &render_descriptor_set_layout);
		assert(result == OPAL_SUCCESS);

		result = opalCreatePipelineLayout(device, 1, &render_descriptor_set_layout, &render_pipeline_layout);
		assert(result == OPAL_SUCCESS);
	}

	// compute
	{
		Opal_DescriptorSetLayoutEntry layout_bindings[] =
		{
			0, OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER, OPAL_SHADER_STAGE_COMPUTE, OPAL_TEXTURE_FORMAT_UNDEFINED, // positions
			1, OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER, OPAL_SHADER_STAGE_COMPUTE, OPAL_TEXTURE_FORMAT_UNDEFINED, // velocities
			2, OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER, OPAL_SHADER_STAGE_COMPUTE, OPAL_TEXTURE_FORMAT_UNDEFINED, // params
			3, OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER, OPAL_SHADER_STAGE_COMPUTE, OPAL_TEXTURE_FORMAT_UNDEFINED, // free indices
			4, OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER, OPAL_SHADER_STAGE_COMPUTE, OPAL_TEXTURE_FORMAT_UNDEFINED, // counters (alive, free)
			5, OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER, OPAL_SHADER_STAGE_COMPUTE, OPAL_TEXTURE_FORMAT_UNDEFINED, // mesh vertices
			6, OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER, OPAL_SHADER_STAGE_COMPUTE, OPAL_TEXTURE_FORMAT_UNDEFINED, // mesh indices
		};

		Opal_Result result = opalCreateDescriptorSetLayout(device, 7, layout_bindings, &compute_descriptor_set_layout);
		assert(result == OPAL_SUCCESS);

		Opal_DescriptorSetLayout layouts[] = { render_descriptor_set_layout, compute_descriptor_set_layout };

		result = opalCreatePipelineLayout(device, 2, layouts, &compute_pipeline_layout);
		assert(result == OPAL_SUCCESS);
	}
}

void Application::buildContent()
{
	// render
	{
		Opal_BufferDesc desc =
		{
			sizeof(AppData),
			OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
			OPAL_ALLOCATION_HINT_AUTO,
			static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_UNIFORM),
			OPAL_BUFFER_STATE_GENERIC_READ,
		};

		Opal_Result result = opalCreateBuffer(device, &desc, &render_application);
		assert(result == OPAL_SUCCESS);
	}

	{
		Opal_BufferDesc desc =
		{
			sizeof(CameraData),
			OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
			OPAL_ALLOCATION_HINT_AUTO,
			static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_UNIFORM),
			OPAL_BUFFER_STATE_GENERIC_READ,
		};

		Opal_Result result = opalCreateBuffer(device, &desc, &render_camera);
		assert(result == OPAL_SUCCESS);
	}

	// particles
	{
		Opal_BufferDesc desc =
		{
			sizeof(float) * NUM_PARTICLES * 4,
			OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL,
			OPAL_ALLOCATION_HINT_AUTO,
			static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_VERTEX | OPAL_BUFFER_USAGE_UNORDERED_ACCESS),
			OPAL_BUFFER_STATE_UNORDERED_ACCESS,
		};

		Opal_Result result = opalCreateBuffer(device, &desc, &particle_positions);
		assert(result == OPAL_SUCCESS);
	}

	{
		Opal_BufferDesc desc =
		{
			sizeof(float) * NUM_PARTICLES * 4,
			OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL,
			OPAL_ALLOCATION_HINT_AUTO,
			static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_UNORDERED_ACCESS),
			OPAL_BUFFER_STATE_UNORDERED_ACCESS,
		};

		Opal_Result result = opalCreateBuffer(device, &desc, &particle_velocities);
		assert(result == OPAL_SUCCESS);

		result = opalCreateBuffer(device, &desc, &particle_parameters);
		assert(result == OPAL_SUCCESS);
	}

	{
		Opal_BufferDesc desc =
		{
			sizeof(uint32_t) * NUM_PARTICLES,
			OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL,
			OPAL_ALLOCATION_HINT_AUTO,
			static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_UNORDERED_ACCESS | OPAL_BUFFER_USAGE_COPY_DST),
			OPAL_BUFFER_STATE_UNORDERED_ACCESS,
		};

		Opal_Result result = opalCreateBuffer(device, &desc, &particle_indices);
		assert(result == OPAL_SUCCESS);

		uint32_t *indices = new uint32_t[NUM_PARTICLES];
		for (uint32_t i = 0; i < NUM_PARTICLES; ++i)
			indices[i] = i;

		bool success = uploadDataToBuffer(device, queue, indices, sizeof(uint32_t) * NUM_PARTICLES, OPAL_BUFFER_STATE_UNORDERED_ACCESS, particle_indices);
		assert(success);

		delete[] indices;
	}

	{
		Opal_BufferDesc desc =
		{
			sizeof(EmitterData),
			OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL,
			OPAL_ALLOCATION_HINT_AUTO,
			static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_UNORDERED_ACCESS | OPAL_BUFFER_USAGE_COPY_DST),
			OPAL_BUFFER_STATE_UNORDERED_ACCESS,
		};

		Opal_Result result = opalCreateBuffer(device, &desc, &particle_counters);
		assert(result == OPAL_SUCCESS);

		EmitterData data = {};
		data.counters[0] = NUM_PARTICLES;
		data.counters[1] = NUM_PARTICLES;
		data.counters[2] = NUM_PARTICLES;
		data.counters[3] = 0;
		data.random[0] = MIN_PARTICLE_LIFETIME;
		data.random[1] = MAX_PARTICLE_LIFETIME;
		data.random[2] = MIN_PARTICLE_IMASS;
		data.random[3] = MAX_PARTICLE_IMASS;

		bool success = uploadDataToBuffer(device, queue, &data, sizeof(EmitterData), OPAL_BUFFER_STATE_UNORDERED_ACCESS, particle_counters);
		assert(success);
	}

	// mesh
	{
		Opal_BufferDesc desc =
		{
			sizeof(LogoMeshData),
			OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL,
			OPAL_ALLOCATION_HINT_AUTO,
			static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_UNORDERED_ACCESS | OPAL_BUFFER_USAGE_COPY_DST),
			OPAL_BUFFER_STATE_UNORDERED_ACCESS,
		};

		Opal_Result result = opalCreateBuffer(device, &desc, &mesh);
		assert(result == OPAL_SUCCESS);

		bool success = uploadDataToBuffer(device, queue, &logo, sizeof(LogoMeshData), OPAL_BUFFER_STATE_UNORDERED_ACCESS, mesh);
		assert(success);
	}
}

void Application::buildDescriptors()
{
	// render
	{
		Opal_DescriptorSetEntry bindings[2] = {};
		bindings[0].binding = 0;
		bindings[0].data.buffer_view.buffer = render_application;
		bindings[0].data.buffer_view.offset = 0;
		bindings[0].data.buffer_view.size = sizeof(AppData);

		bindings[1].binding = 1;
		bindings[1].data.buffer_view.buffer = render_camera;
		bindings[1].data.buffer_view.offset = 0;
		bindings[1].data.buffer_view.size = sizeof(CameraData);

		Opal_DescriptorSetAllocationDesc desc =
		{
			render_descriptor_set_layout,
			descriptor_heap,
			2, bindings
		};

		opalAllocateDescriptorSet(device, &desc, &render_descriptor_set);
	}

	// compute
	{
		Opal_DescriptorSetEntry bindings[7] = {};
		bindings[0].binding = 0;
		bindings[0].data.storage_buffer_view.buffer = particle_positions;
		bindings[0].data.storage_buffer_view.element_size = sizeof(float) * 4;
		bindings[0].data.storage_buffer_view.num_elements = NUM_PARTICLES;

		bindings[1].binding = 1;
		bindings[1].data.storage_buffer_view.buffer = particle_velocities;
		bindings[1].data.storage_buffer_view.element_size = sizeof(float) * 4;
		bindings[1].data.storage_buffer_view.num_elements = NUM_PARTICLES;

		bindings[2].binding = 2;
		bindings[2].data.storage_buffer_view.buffer = particle_parameters;
		bindings[2].data.storage_buffer_view.element_size = sizeof(float) * 4;
		bindings[2].data.storage_buffer_view.num_elements = NUM_PARTICLES;

		bindings[3].binding = 3;
		bindings[3].data.storage_buffer_view.buffer = particle_indices;
		bindings[3].data.storage_buffer_view.element_size = sizeof(uint32_t);
		bindings[3].data.storage_buffer_view.num_elements = NUM_PARTICLES;

		bindings[4].binding = 4;
		bindings[4].data.storage_buffer_view.buffer = particle_counters;
		bindings[4].data.storage_buffer_view.element_size = sizeof(EmitterData);
		bindings[4].data.storage_buffer_view.num_elements = 1;

		bindings[5].binding = 5;
		bindings[5].data.storage_buffer_view.buffer = mesh;
		bindings[5].data.storage_buffer_view.element_size = sizeof(float) * 4;
		bindings[5].data.storage_buffer_view.num_elements = LOGO_NUM_VERTICES;

		bindings[6].binding = 6;
		bindings[6].data.storage_buffer_view.buffer = mesh;
		bindings[6].data.storage_buffer_view.element_size = sizeof(uint32_t);
		bindings[6].data.storage_buffer_view.num_elements = LOGO_NUM_INDICES;
		bindings[6].data.storage_buffer_view.offset = offsetof(LogoMeshData, indices);

		Opal_DescriptorSetAllocationDesc desc =
		{
			compute_descriptor_set_layout,
			descriptor_heap,
			7, bindings
		};

		opalAllocateDescriptorSet(device, &desc, &compute_descriptor_set);
	}
}

void Application::buildPipelines()
{
	bool loaded = loadShader(emit_shader_paths[target_api], shader_types[target_api], device, &emit_shader);
	assert(loaded == true);

	loaded = loadShader(simulate_shader_paths[target_api], shader_types[target_api], device, &simulate_shader);
	assert(loaded == true);

	loaded = loadShader(vertex_shader_paths[target_api], shader_types[target_api], device, &render_vertex_shader);
	assert(loaded == true);

	loaded = loadShader(fragment_shader_paths[target_api], shader_types[target_api], device, &render_fragment_shader);
	assert(loaded == true);

	{
		Opal_VertexAttribute instance_attributes[] =
		{
			{ OPAL_VERTEX_FORMAT_RGBA32_SFLOAT, 0 },
		};

		Opal_VertexStream vertex_streams[] =
		{
			{ sizeof(float) * 4, 1, instance_attributes, OPAL_VERTEX_INPUT_RATE_INSTANCE },
		};

		Opal_GraphicsPipelineDesc pipeline_desc = {};
		pipeline_desc.pipeline_layout = render_pipeline_layout;

		pipeline_desc.vertex_function = { render_vertex_shader, "vertexMain" };
		pipeline_desc.fragment_function = { render_fragment_shader, "fragmentMain" };

		pipeline_desc.num_vertex_streams = 1;
		pipeline_desc.vertex_streams = vertex_streams;
		pipeline_desc.primitive_type = OPAL_PRIMITIVE_TYPE_TRIANGLE_LIST;

		pipeline_desc.cull_mode = OPAL_CULL_MODE_NONE;
		pipeline_desc.front_face = OPAL_FRONT_FACE_COUNTER_CLOCKWISE;
		pipeline_desc.rasterization_samples = OPAL_SAMPLES_1;

		pipeline_desc.color_blend_states[0].enable = 1;
		pipeline_desc.color_blend_states[0].color_op = OPAL_BLEND_OP_ADD;
		pipeline_desc.color_blend_states[0].alpha_op = OPAL_BLEND_OP_ADD;
		pipeline_desc.color_blend_states[0].src_color = OPAL_BLEND_FACTOR_ONE;
		pipeline_desc.color_blend_states[0].src_alpha = OPAL_BLEND_FACTOR_ONE;
		pipeline_desc.color_blend_states[0].dst_color = OPAL_BLEND_FACTOR_ONE;
		pipeline_desc.color_blend_states[0].dst_alpha = OPAL_BLEND_FACTOR_ONE;

		pipeline_desc.num_color_attachments = 1;
		pipeline_desc.color_attachment_formats[0] = OPAL_TEXTURE_FORMAT_BGRA8_UNORM;

		opalCreateGraphicsPipeline(device, &pipeline_desc, &render_pipeline);
	}

	{
		Opal_ComputePipelineDesc pipeline_desc = {};
		pipeline_desc.pipeline_layout = compute_pipeline_layout;
		pipeline_desc.compute_function = { emit_shader, "computeMain" };
		pipeline_desc.threadgroup_size_x = 1;
		pipeline_desc.threadgroup_size_y = 1;
		pipeline_desc.threadgroup_size_z = 1;

		opalCreateComputePipeline(device, &pipeline_desc, &emit_pipeline);
	}

	{
		Opal_ComputePipelineDesc pipeline_desc = {};
		pipeline_desc.pipeline_layout = compute_pipeline_layout;
		pipeline_desc.compute_function = { simulate_shader, "computeMain" };
		pipeline_desc.threadgroup_size_x = 32;
		pipeline_desc.threadgroup_size_y = 1;
		pipeline_desc.threadgroup_size_z = 1;

		opalCreateComputePipeline(device, &pipeline_desc, &simulate_pipeline);
	}
}

void Application::destroyLayout()
{
	Opal_Result result = opalDestroyPipelineLayout(device, compute_pipeline_layout);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyDescriptorSetLayout(device, compute_descriptor_set_layout);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyPipelineLayout(device, render_pipeline_layout);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyDescriptorSetLayout(device, render_descriptor_set_layout);
	assert(result == OPAL_SUCCESS);
}

void Application::destroyContent()
{
	Opal_Result result = opalDestroyBuffer(device, render_application);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, render_camera);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, mesh);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, particle_positions);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, particle_velocities);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, particle_parameters);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, particle_indices);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, particle_counters);
	assert(result == OPAL_SUCCESS);
}

void Application::destroyDescriptors()
{
	Opal_Result result = opalFreeDescriptorSet(device, compute_descriptor_set);
	assert(result == OPAL_SUCCESS);

	result = opalFreeDescriptorSet(device, render_descriptor_set);
	assert(result == OPAL_SUCCESS);
}

void Application::destroyPipelines()
{
	Opal_Result result = opalDestroyShader(device, render_vertex_shader);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyShader(device, render_fragment_shader);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyGraphicsPipeline(device, render_pipeline);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyShader(device, emit_shader);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyComputePipeline(device, emit_pipeline);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyShader(device, simulate_shader);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyComputePipeline(device, simulate_pipeline);
	assert(result == OPAL_SUCCESS);
}
