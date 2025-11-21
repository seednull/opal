#include "app.h"

#include <cstddef>
#include <cassert>
#include <cstdio>
#include <cstring>

#ifdef OPAL_PLATFORM_WINDOWS
static constexpr Opal_Api target_api = OPAL_API_DIRECTX12;
#elif OPAL_PLATFORM_MACOS
static constexpr Opal_Api target_api = OPAL_API_METAL;
#elif OPAL_PLATFORM_WEB
static constexpr Opal_Api target_api = OPAL_API_WEBGPU;
#endif

#define UNUSED(x) do { (void)(x); } while(0)

static const char *vertex_paths[] =
{
	"samples/04_triangle/shaders/vulkan/main.vert.spv",
	"samples/04_triangle/shaders/directx12/main.vert.cso",
	"samples/04_triangle/shaders/metal/main.vert.metallib",
	"samples/04_triangle/shaders/webgpu/main.vert.wgsl",
	nullptr,
};

static const char *fragment_paths[] =
{
	"samples/04_triangle/shaders/vulkan/main.frag.spv",
	"samples/04_triangle/shaders/directx12/main.frag.cso",
	"samples/04_triangle/shaders/metal/main.frag.metallib",
	"samples/04_triangle/shaders/webgpu/main.frag.wgsl",
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

struct Vertex
{
	float position[4];
	float color[4];
};

struct TriangleData
{
	Vertex vertices[3];
	uint32_t indices[4];
};

static TriangleData triangle_data =
{
	// vertices
	// position               color
	-1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	 1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,

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
		"04_triangle",
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

	// buffers
	Opal_BufferDesc triangle_buffer_desc =
	{
		sizeof(TriangleData),
		OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL,
		OPAL_ALLOCATION_HINT_AUTO,
		static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_VERTEX | OPAL_BUFFER_USAGE_INDEX | OPAL_BUFFER_USAGE_COPY_DST),
		OPAL_BUFFER_STATE_COPY_DST,
	};

	result = opalCreateBuffer(device, &triangle_buffer_desc, &triangle_buffer);
	assert(result == OPAL_SUCCESS);

	// copy
	Opal_BufferDesc staging_buffer_desc =
	{
		sizeof(TriangleData),
		OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
		OPAL_ALLOCATION_HINT_AUTO,
		static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_COPY_SRC),
		OPAL_BUFFER_STATE_GENERIC_READ,
	};

	Opal_Buffer staging_buffer = OPAL_NULL_HANDLE;
	result = opalCreateBuffer(device, &staging_buffer_desc, &staging_buffer);
	assert(result == OPAL_SUCCESS);

	void *staging_ptr = nullptr;
	result = opalMapBuffer(device, staging_buffer, &staging_ptr);
	assert(result == OPAL_SUCCESS);

	memcpy(staging_ptr, &triangle_data, sizeof(TriangleData));

	result = opalUnmapBuffer(device, staging_buffer);
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

	result = opalCmdBeginCopyPass(device, staging_command_buffer, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalCmdCopyBufferToBuffer(device, staging_command_buffer, staging_buffer, 0, triangle_buffer, 0, sizeof(TriangleData));
	assert(result == OPAL_SUCCESS);

	Opal_BufferTransitionDesc copy_end_transitions[] =
	{
		{ triangle_buffer, OPAL_BUFFER_STATE_COPY_DST, OPAL_BUFFER_STATE_GENERIC_READ },
	};

	Opal_BarrierDesc copy_end_barrier = {};
	copy_end_barrier.wait_stages = OPAL_BARRIER_STAGE_COPY;
	copy_end_barrier.block_stages = OPAL_BARRIER_STAGE_NONE;
	copy_end_barrier.num_buffer_transitions = 1;
	copy_end_barrier.buffer_transitions = copy_end_transitions;

	Opal_PassBarriersDesc copy_end = { 1, &copy_end_barrier };

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

	// shaders
	bool loaded = loadShader(vertex_paths[target_api], shader_types[target_api], device, &vertex_shader);
	assert(loaded == true);

	loaded = loadShader(fragment_paths[target_api], shader_types[target_api], device, &fragment_shader);
	assert(loaded == true);

	// pipeline
	result = opalCreatePipelineLayout(device, 0, nullptr, &pipeline_layout);
	assert(result == OPAL_SUCCESS);

	Opal_VertexAttribute vertex_attributes[] =
	{
		{ OPAL_VERTEX_FORMAT_RGBA32_SFLOAT, offsetof(Vertex, position) },
		{ OPAL_VERTEX_FORMAT_RGBA32_SFLOAT, offsetof(Vertex, color) },
	};

	Opal_VertexStream vertex_stream =
	{
		sizeof(Vertex),
		2, vertex_attributes,
		OPAL_VERTEX_INPUT_RATE_VERTEX
	};

	Opal_GraphicsPipelineDesc pipeline_desc = {};
	pipeline_desc.pipeline_layout = pipeline_layout;

	pipeline_desc.vertex_function = { vertex_shader, "vertexMain" };
	pipeline_desc.fragment_function = { fragment_shader, "fragmentMain" };

	pipeline_desc.num_vertex_streams = 1;
	pipeline_desc.vertex_streams = &vertex_stream;
	pipeline_desc.primitive_type = OPAL_PRIMITIVE_TYPE_TRIANGLE_STRIP;
	pipeline_desc.strip_index_format = OPAL_INDEX_FORMAT_UINT32;

	pipeline_desc.cull_mode = OPAL_CULL_MODE_NONE;
	pipeline_desc.front_face = OPAL_FRONT_FACE_COUNTER_CLOCKWISE;
	pipeline_desc.rasterization_samples = OPAL_SAMPLES_1;

	pipeline_desc.num_color_attachments = 1;
	pipeline_desc.color_attachment_formats[0] = OPAL_TEXTURE_FORMAT_BGRA8_UNORM;

	result = opalCreateGraphicsPipeline(device, &pipeline_desc, &pipeline);
	assert(result == OPAL_SUCCESS);

	// semaphore
	Opal_SemaphoreDesc semaphore_desc = {};
	semaphore_desc.flags = OPAL_SEMAPHORE_CREATION_FLAGS_HOST_OPERATIONS;
	semaphore_desc.initial_value = 0;

	result = opalCreateSemaphore(device, &semaphore_desc, &semaphore);
	assert(result == OPAL_SUCCESS);

	// per frame command buffers
	for (uint32_t i = 0; i < IN_FLIGHT_FRAMES; ++i)
	{
		result = opalCreateCommandAllocator(device, queue, &command_allocators[i]);
		assert(result == OPAL_SUCCESS);

		result = opalCreateCommandBuffer(device, command_allocators[i], &command_buffers[i]);
		assert(result == OPAL_SUCCESS);
	}

	current_frame = 0;
	wait_frame = 0;
}

void Application::shutdown()
{
	Opal_Result result = opalWaitIdle(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, triangle_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyShader(device, vertex_shader);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyShader(device, fragment_shader);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyPipelineLayout(device, pipeline_layout);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyGraphicsPipeline(device, pipeline);
	assert(result == OPAL_SUCCESS);
	
	result = opalDestroySemaphore(device, semaphore);
	assert(result == OPAL_SUCCESS);

	for (uint32_t i = 0; i < IN_FLIGHT_FRAMES; ++i)
	{
		result = opalDestroyCommandBuffer(device, command_buffers[i]);
		assert(result == OPAL_SUCCESS);

		result = opalDestroyCommandAllocator(device, command_allocators[i]);
		assert(result == OPAL_SUCCESS);
	}

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
	UNUSED(dt);
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
	uint64_t index = current_frame % IN_FLIGHT_FRAMES;

	Opal_TextureView swapchain_texture_view = OPAL_NULL_HANDLE;
	Opal_CommandAllocator command_allocator = command_allocators[index];
	Opal_CommandBuffer command_buffer = command_buffers[index];

	Opal_Result result = opalAcquire(device, swapchain, &swapchain_texture_view);
	assert(result == OPAL_SUCCESS);

	result = opalWaitSemaphore(device, semaphore, wait_frame, WAIT_TIMEOUT_MS);
	assert(result == OPAL_SUCCESS);

	result = opalResetCommandAllocator(device, command_allocator);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_TextureTransitionDesc graphics_begin_transitions[] =
	{
		{ swapchain_texture_view, OPAL_TEXTURE_STATE_UNDEFINED, OPAL_TEXTURE_STATE_FRAMEBUFFER_ATTACHMENT },
	};

	Opal_BarrierDesc graphics_begin_barrier = {};
	graphics_begin_barrier.wait_stages = OPAL_BARRIER_STAGE_NONE;
	graphics_begin_barrier.block_stages = OPAL_BARRIER_STAGE_GRAPHICS_FRAGMENT;
	graphics_begin_barrier.num_texture_transitions = 1;
	graphics_begin_barrier.texture_transitions = graphics_begin_transitions;

	Opal_PassBarriersDesc graphics_begin = { 1, &graphics_begin_barrier };

	Opal_ClearColor clear_color = {0.4f, 0.4f, 0.4f, 1.0f};
	Opal_FramebufferAttachment attachments[] =
	{
		{ swapchain_texture_view, OPAL_NULL_HANDLE, OPAL_LOAD_OP_CLEAR, OPAL_STORE_OP_STORE, clear_color },
	};

	Opal_FramebufferDesc framebuffer = { 1, attachments, NULL };

	result = opalCmdBeginGraphicsPass(device, command_buffer, &framebuffer, &graphics_begin);
	assert(result == OPAL_SUCCESS);

	result = opalCmdGraphicsSetPipelineLayout(device, command_buffer, pipeline_layout);
	assert(result == OPAL_SUCCESS);

	result = opalCmdGraphicsSetPipeline(device, command_buffer, pipeline);
	assert(result == OPAL_SUCCESS);

	Opal_VertexBufferView vertex_buffer = { triangle_buffer, 0, sizeof(Vertex) * 3, sizeof(Vertex) };
	Opal_IndexBufferView index_buffer = { triangle_buffer, offsetof(TriangleData, indices), sizeof(uint32_t) * 3, OPAL_INDEX_FORMAT_UINT32 };

	result = opalCmdGraphicsSetVertexBuffers(device, command_buffer, 1, &vertex_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalCmdGraphicsSetIndexBuffer(device, command_buffer, index_buffer);
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

	result = opalCmdGraphicsDrawIndexed(device, command_buffer, 3, 1, 0, 0, 0);
	assert(result == OPAL_SUCCESS);

	Opal_TextureTransitionDesc graphics_end_transitions[] =
	{
		{ swapchain_texture_view, OPAL_TEXTURE_STATE_FRAMEBUFFER_ATTACHMENT, OPAL_TEXTURE_STATE_PRESENT },
	};

	Opal_BarrierDesc graphics_end_barrier = {};
	graphics_end_barrier.wait_stages = OPAL_BARRIER_STAGE_GRAPHICS_FRAGMENT;
	graphics_end_barrier.block_stages = OPAL_BARRIER_STAGE_NONE;
	graphics_end_barrier.num_texture_transitions = 1;
	graphics_end_barrier.texture_transitions = graphics_end_transitions;

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
	if ((current_frame % IN_FLIGHT_FRAMES) == 0)
		wait_frame = current_frame;
}
