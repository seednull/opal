#ifdef OPAL_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <opal.h>
#include <cassert>
#include <iostream>
#include <filesystem>

#include <openexr.h>

Opal_Result createHdriTexture(const float *buffer, uint32_t width, uint32_t height, Opal_Device device, Opal_Queue queue, Opal_CommandPool command_pool, Opal_Texture *texture, Opal_TextureView *texture_view)
{
	uint32_t size = width * height * 4 * sizeof(float);

	Opal_Buffer staging_buffer = OPAL_NULL_HANDLE;
	Opal_BufferDesc staging_buffer_desc =
	{
		static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_TRANSFER_SRC),
		size,
		OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
		OPAL_ALLOCATION_HINT_AUTO,
	};

	Opal_Result result = opalCreateBuffer(device, &staging_buffer_desc, &staging_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_TextureDesc texture_desc =
	{
		OPAL_TEXTURE_TYPE_2D,
		OPAL_FORMAT_RGBA32_SFLOAT,
		width,
		height,
		1,
		1,
		1,
		OPAL_SAMPLES_1,
		static_cast<Opal_TextureUsageFlags>(OPAL_TEXTURE_USAGE_SHADER_SAMPLED | OPAL_TEXTURE_USAGE_TRANSFER_DST),
		OPAL_ALLOCATION_HINT_AUTO
	};

	result = opalCreateTexture(device, &texture_desc, texture);
	assert(result == OPAL_SUCCESS);

	Opal_TextureViewDesc texture_view_desc =
	{
		*texture,
		OPAL_TEXTURE_VIEW_TYPE_2D,
		0,
		1,
		0,
		1
	};

	result = opalCreateTextureView(device, &texture_view_desc, texture_view);
	assert(result == OPAL_SUCCESS);

	// copy
	void *staging_ptr = nullptr;
	result = opalMapBuffer(device, staging_buffer, &staging_ptr);
	assert(result == OPAL_SUCCESS);

	memcpy(staging_ptr, buffer, size);

	result = opalUnmapBuffer(device, staging_buffer);
	assert(result == OPAL_SUCCESS);

	// transfer
	Opal_CommandBuffer staging_command_buffer = OPAL_NULL_HANDLE;
	result = opalAllocateCommandBuffer(device, command_pool, &staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommandBuffer(device, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_BufferView staging_buffer_view = { staging_buffer, 0 };
	Opal_TextureRegion texture_region = { *texture_view, 0, 0, 0, width, height, 1 };

	result = opalCmdTextureTransitionBarrier(device, staging_command_buffer, *texture_view, OPAL_RESOURCE_STATE_GENERAL, OPAL_RESOURCE_STATE_COPY_DEST);
	assert(result == OPAL_SUCCESS);

	result = opalCmdCopyBufferToTexture(device, staging_command_buffer, staging_buffer_view, texture_region);
	assert(result == OPAL_SUCCESS);

	result = opalCmdTextureTransitionBarrier(device, staging_command_buffer, *texture_view, OPAL_RESOURCE_STATE_COPY_DEST, OPAL_RESOURCE_STATE_FRAGMENT_SHADER_RESOURCE);
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

	result = opalFreeCommandBuffer(device, command_pool, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, staging_buffer);
	assert(result == OPAL_SUCCESS);

	return OPAL_SUCCESS;
}

bool loadHdri(const char *name, Opal_Device device, Opal_Queue queue, Opal_CommandPool command_pool, Opal_Texture *texture, Opal_TextureView *texture_view)
{
	exr_context_initializer_t context_init = EXR_DEFAULT_CONTEXT_INITIALIZER;
	exr_context_t context = {};

	exr_result_t result = exr_start_read(&context, name, &context_init);
	assert(result == EXR_ERR_SUCCESS);

	const int part_index = 0;

	exr_attr_box2i_t data_window = {};
	result = exr_get_data_window(context, part_index, &data_window);
	assert(result == EXR_ERR_SUCCESS);

	exr_storage_t storage = {};
	result = exr_get_storage(context, part_index, &storage);
	assert(result == EXR_ERR_SUCCESS);
	assert(storage == EXR_STORAGE_SCANLINE);

	int num_scanlines_per_chunk = 0;
	result = exr_get_scanlines_per_chunk(context, part_index, &num_scanlines_per_chunk);
	assert(result == EXR_ERR_SUCCESS);

	int current_scanline = 0;

	uint32_t width = data_window.max.x + 1;
	uint32_t height = data_window.max.y + 1;

	float *buffer = new float[width * height * 4];
	float *current_ptr = buffer;

	uint32_t channel_stride = width * num_scanlines_per_chunk;

	while (current_scanline < height)
	{
		exr_chunk_info_t chunk_info = {};
		result = exr_read_scanline_chunk_info(context, part_index, current_scanline, &chunk_info);
		assert(result == EXR_ERR_SUCCESS);

		exr_decode_pipeline_t decode = EXR_DECODE_PIPELINE_INITIALIZER;
		result = exr_decoding_initialize(context, part_index, &chunk_info, &decode);
		assert(result == EXR_ERR_SUCCESS);
		assert(decode.channel_count == 3);

		result = exr_decoding_choose_default_routines(context, part_index, &decode);
		assert(result == EXR_ERR_SUCCESS);

		result = exr_decoding_run(context, part_index, &decode);
		assert(result == EXR_ERR_SUCCESS);

		current_scanline += num_scanlines_per_chunk;

		const float *input = reinterpret_cast<const float *>(decode.unpacked_buffer);

		for (uint32_t scanline = 0; scanline < num_scanlines_per_chunk; ++scanline)
		{
			const float *b_channel = input + width * 0;
			const float *g_channel = input + width * 1;
			const float *r_channel = input + width * 2;

			for (uint32_t i = 0; i < width; ++i)
			{
				current_ptr[0] = *r_channel;
				current_ptr[1] = *g_channel;
				current_ptr[2] = *b_channel;
				current_ptr[3] = 1.0f;

				r_channel++;
				g_channel++;
				b_channel++;
				current_ptr += 4;
			}

			input += width * 3;
		}

		result = exr_decoding_destroy(context, &decode);
		assert(result == EXR_ERR_SUCCESS);
	}

	result = exr_finish(&context);
	assert(result == EXR_ERR_SUCCESS);

	Opal_Result opal_result = createHdriTexture(buffer, width, height, device, queue, command_pool, texture, texture_view);
	delete[] buffer;

	return opal_result == OPAL_SUCCESS;
}

bool loadShader(const char *name, Opal_Device device, Opal_Shader *shader)
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

	uint8_t *data = new uint8_t[size];
	fread(data, sizeof(uint8_t), size, f);
	fclose(f);

	Opal_ShaderDesc desc =
	{
		OPAL_SHADER_SOURCE_TYPE_SPIRV_BINARY,
		data,
		size,
	};

	Opal_Result result = opalCreateShader(device, &desc, shader);
	free(data);

	return result == OPAL_SUCCESS;
}

/*
 */
class Application
{
public:
	void init(void *handle, uint32_t width, uint32_t height);
	void shutdown();
	void buildContent();
	void buildPipeline();
	void update(float dt);
	void resize(uint32_t width, uint32_t height);
	void render();
	void present();

private:
	struct Vertex
	{
		float position[4];
		float uv[2];
	};

	struct TriangleData
	{
		Vertex vertices[4];
		uint32_t indices[6];
	};

	struct AppData
	{
		float width {0.0f};
		float height {0.0f};
		float width_inv {0.0f};
		float height_inv {0.0f};
		float time {0.0f};
	};

	enum
	{
		IN_FLIGHT_FRAMES = 2,
		WAIT_TIMEOUT_MS = 1000,
	};

private:
	Opal_Instance instance {OPAL_NULL_HANDLE};
	Opal_Surface surface {OPAL_NULL_HANDLE};
	Opal_Swapchain swapchain {OPAL_NULL_HANDLE};
	Opal_Device device {OPAL_NULL_HANDLE};
	Opal_Queue queue {OPAL_NULL_HANDLE};
	Opal_PipelineLayout pipeline_layout {OPAL_NULL_HANDLE};
	Opal_Pipeline pipeline {OPAL_NULL_HANDLE};
	Opal_BindsetPool bindset_pool {OPAL_NULL_HANDLE};
	Opal_BindsetLayout bindset_layout {OPAL_NULL_HANDLE};
	Opal_Bindset bindset {OPAL_NULL_HANDLE};
	Opal_CommandPool command_pool {OPAL_NULL_HANDLE};
	Opal_CommandBuffer command_buffers[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};
	Opal_Semaphore semaphores[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};
	uint64_t semaphore_values[IN_FLIGHT_FRAMES] {0, 0};

	Opal_Texture hdri {OPAL_NULL_HANDLE};
	Opal_TextureView hdri_view {OPAL_NULL_HANDLE};
	Opal_Sampler hdri_sampler {OPAL_NULL_HANDLE};
	Opal_Buffer application_buffer {OPAL_NULL_HANDLE};
	Opal_Buffer triangle_buffer {OPAL_NULL_HANDLE};

	Opal_Shader vertex_shader {OPAL_NULL_HANDLE};
	Opal_Shader fragment_shader {OPAL_NULL_HANDLE};

	float current_time {0.0f};
	uint32_t current_in_flight_frame {0};
	uint32_t width {0};
	uint32_t height {0};
};

/*
 */
void Application::init(void *handle, uint32_t w, uint32_t h)
{
	width = w;
	height = h;

	// instance & surface
	Opal_InstanceDesc instance_desc =
	{
		"05_opal",
		"Opal",
		0,
		0,
		OPAL_DEFAULT_HEAP_SIZE,
		OPAL_DEFAULT_HEAP_ALLOCATIONS,
		OPAL_DEFAULT_HEAPS,
		OPAL_INSTANCE_CREATION_FLAGS_USE_VULKAN_VALIDATION_LAYERS,
	};

	Opal_Result result = opalCreateInstance(OPAL_API_VULKAN, &instance_desc, &instance);
	assert(result == OPAL_SUCCESS);

	result = opalCreateSurface(instance, handle, &surface);
	assert(result == OPAL_SUCCESS);

	// device & swapchain
	result = opalCreateDefaultDevice(instance, OPAL_DEVICE_HINT_DEFAULT, &device);
	assert(result == OPAL_SUCCESS);

	result = opalGetDeviceQueue(device, OPAL_DEVICE_ENGINE_TYPE_MAIN, 0, &queue);
	assert(result == OPAL_SUCCESS);

	Opal_SwapchainDesc swapchain_desc =
	{
		OPAL_PRESENT_MODE_MAILBOX,
		OPAL_FORMAT_BGRA8_UNORM,
		OPAL_COLORSPACE_SRGB,
		(Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT | OPAL_TEXTURE_USAGE_SHADER_SAMPLED),
		surface
	};

	result = opalCreateSwapchain(device, &swapchain_desc, &swapchain);
	assert(result == OPAL_SUCCESS);

	// command pool
	result = opalCreateCommandPool(device, queue, &command_pool);
	assert(result == OPAL_SUCCESS);

	// content
	buildContent();

	// bindings
	Opal_BindsetLayoutBinding layout_bindings[] =
	{
		0, OPAL_BINDING_TYPE_UNIFORM_BUFFER, OPAL_SHADER_STAGE_ALL_GRAPHICS,
		1, OPAL_BINDING_TYPE_COMBINED_TEXTURE_SAMPLER, OPAL_SHADER_STAGE_ALL_GRAPHICS,
	};

	result = opalCreateBindsetLayout(device, 2, layout_bindings, &bindset_layout);
	assert(result == OPAL_SUCCESS);

	result = opalCreateBindsetPool(device, bindset_layout, 32, &bindset_pool);
	assert(result == OPAL_SUCCESS);

	result = opalAllocateBindset(device, bindset_pool, &bindset);
	assert(result == OPAL_SUCCESS);

	Opal_BindsetBinding bindings[2];
	bindings[0].binding = 0;
	bindings[0].buffer.buffer = application_buffer;
	bindings[0].buffer.offset = 0;
	bindings[0].buffer.size = sizeof(AppData);
	bindings[1].binding = 1;
	bindings[1].texture_view = hdri_view;
	bindings[1].sampler = hdri_sampler;

	result = opalUpdateBindset(device, bindset, 2, bindings);
	assert(result == OPAL_SUCCESS);

	// pipeline layout
	result = opalCreatePipelineLayout(device, 1, &bindset_layout, &pipeline_layout);
	assert(result == OPAL_SUCCESS);

	buildPipeline();

	// command buffers & semaphores
	Opal_SemaphoreDesc semaphore_desc = {};
	semaphore_desc.flags = OPAL_SEMAPHORE_CREATION_FLAGS_HOST_OPERATIONS;
	semaphore_desc.initial_value = 0;

	for (uint32_t i = 0; i < IN_FLIGHT_FRAMES; ++i)
	{
		result = opalAllocateCommandBuffer(device, command_pool, &command_buffers[i]);
		assert(result == OPAL_SUCCESS);

		result = opalCreateSemaphore(device, &semaphore_desc, &semaphores[i]);
		assert(result == OPAL_SUCCESS);
	}

	current_in_flight_frame = 0;
}

void Application::buildContent()
{
	Opal_BufferDesc triangle_buffer_desc =
	{
		static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_VERTEX | OPAL_BUFFER_USAGE_INDEX | OPAL_BUFFER_USAGE_TRANSFER_DST),
		sizeof(TriangleData),
		OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL,
		OPAL_ALLOCATION_HINT_AUTO,
	};

	Opal_Result result = opalCreateBuffer(device, &triangle_buffer_desc, &triangle_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_BufferDesc staging_buffer_desc =
	{
		static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_TRANSFER_SRC),
		sizeof(TriangleData),
		OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
		OPAL_ALLOCATION_HINT_AUTO,
	};

	Opal_Buffer staging_buffer = OPAL_NULL_HANDLE;
	result = opalCreateBuffer(device, &staging_buffer_desc, &staging_buffer);
	assert(result == OPAL_SUCCESS);

	// copy
	void *staging_ptr = nullptr;
	result = opalMapBuffer(device, staging_buffer, &staging_ptr);
	assert(result == OPAL_SUCCESS);

	TriangleData triangle_data =
	{
		// vertices
		{
			// position                  uv
			 -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
			  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
			  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
			 -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		},

		// indices
		0, 1, 2,
		2, 3, 0,
	};

	memcpy(staging_ptr, &triangle_data, sizeof(TriangleData));

	result = opalUnmapBuffer(device, staging_buffer);
	assert(result == OPAL_SUCCESS);

	// transfer
	Opal_CommandBuffer staging_command_buffer = OPAL_NULL_HANDLE;
	result = opalAllocateCommandBuffer(device, command_pool, &staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommandBuffer(device, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_BufferView staging_buffer_view = { staging_buffer, 0 };
	Opal_BufferView triangle_buffer_view = { triangle_buffer, 0 };

	result = opalCmdCopyBufferToBuffer(device, staging_command_buffer, staging_buffer_view, triangle_buffer_view, sizeof(TriangleData));
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

	result = opalFreeCommandBuffer(device, command_pool, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, staging_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_BufferDesc application_buffer_desc =
	{
		static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_UNIFORM),
		sizeof(AppData),
		OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
		OPAL_ALLOCATION_HINT_AUTO,
	};

	result = opalCreateBuffer(device, &application_buffer_desc, &application_buffer);
	assert(result == OPAL_SUCCESS);

	bool loaded = loadHdri("samples/05_opal/hdri/blue_photo_studio_1k.exr", device, queue, command_pool, &hdri, &hdri_view);
	assert(loaded == true);

	Opal_SamplerDesc sampler_desc =
	{
		OPAL_SAMPLER_FILTER_MODE_LINEAR,
		OPAL_SAMPLER_FILTER_MODE_LINEAR,
		OPAL_SAMPLER_FILTER_MODE_LINEAR,
		OPAL_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		OPAL_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		OPAL_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		0.0f,
		1.0f,
		0,
		OPAL_COMPARE_OP_NEVER,
	};

	result = opalCreateSampler(device, &sampler_desc, &hdri_sampler);
	assert(result == OPAL_SUCCESS);
}

void Application::buildPipeline()
{
	Opal_Result result = OPAL_SUCCESS;

	result = opalWaitIdle(device);
	assert(result == OPAL_SUCCESS);
	
	if (vertex_shader != OPAL_NULL_HANDLE)
	{
		result = opalDestroyShader(device, vertex_shader);
		assert(result == OPAL_SUCCESS);
	}

	if (fragment_shader != OPAL_NULL_HANDLE)
	{
		result = opalDestroyShader(device, fragment_shader);
		assert(result == OPAL_SUCCESS);
	}

	if (pipeline != OPAL_NULL_HANDLE)
	{
		result = opalDestroyPipeline(device, pipeline);
		assert(result == OPAL_SUCCESS);
	}

	// shaders
	bool loaded = loadShader("samples/05_opal/shaders/vulkan/main.vert.spv", device, &vertex_shader);
	assert(loaded == true);

	loaded = loadShader("samples/05_opal/shaders/vulkan/main.frag.spv", device, &fragment_shader);
	assert(loaded == true);

	// pipeline
	Opal_VertexAttribute vertex_attributes[] =
	{
		{ OPAL_FORMAT_RGBA32_SFLOAT, offsetof(Vertex, position) },
		{ OPAL_FORMAT_RG32_SFLOAT, offsetof(Vertex, uv) },
	};

	Opal_VertexStream vertex_stream =
	{
		sizeof(Vertex),
		2, vertex_attributes,
		OPAL_VERTEX_INPUT_RATE_VERTEX
	};

	Opal_GraphicsPipelineDesc pipeline_desc = {};
	pipeline_desc.pipeline_layout = pipeline_layout;

	pipeline_desc.vertex_shader = vertex_shader;
	pipeline_desc.fragment_shader = fragment_shader;

	pipeline_desc.num_vertex_streams = 1;
	pipeline_desc.vertex_streams = &vertex_stream;
	pipeline_desc.primitive_type = OPAL_PRIMITIVE_TYPE_TRIANGLE;

	pipeline_desc.cull_mode = OPAL_CULL_MODE_NONE;
	pipeline_desc.front_face = OPAL_FRONT_FACE_COUNTER_CLOCKWISE;
	pipeline_desc.rasterization_samples = OPAL_SAMPLES_1;

	pipeline_desc.num_color_attachments = 1;
	pipeline_desc.color_attachment_formats[0] = OPAL_FORMAT_BGRA8_UNORM;

	result = opalCreateGraphicsPipeline(device, &pipeline_desc, &pipeline);
	assert(result == OPAL_SUCCESS);
}

void Application::shutdown()
{
	Opal_Result result = opalWaitIdle(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, triangle_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyTextureView(device, hdri_view);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyTexture(device, hdri);
	assert(result == OPAL_SUCCESS);

	result = opalDestroySampler(device, hdri_sampler);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyShader(device, vertex_shader);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyShader(device, fragment_shader);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyPipeline(device, pipeline);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyPipelineLayout(device, pipeline_layout);
	assert(result == OPAL_SUCCESS);

	for (uint32_t i = 0; i < IN_FLIGHT_FRAMES; ++i)
	{
		result = opalFreeCommandBuffer(device, command_pool, command_buffers[i]);
		assert(result == OPAL_SUCCESS);

		result = opalDestroySemaphore(device, semaphores[i]);
		assert(result == OPAL_SUCCESS);
	}

	result = opalDestroyCommandPool(device, command_pool);
	assert(result == OPAL_SUCCESS);

	result = opalFreeBindset(device, bindset_pool, bindset);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBindsetPool(device, bindset_pool);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBindsetLayout(device, bindset_layout);
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
	AppData *application_buffer_ptr = nullptr;

	Opal_Result result = opalMapBuffer(device, application_buffer, reinterpret_cast<void **>(&application_buffer_ptr));
	assert(result == OPAL_SUCCESS);

	application_buffer_ptr->width = static_cast<float>(width);
	application_buffer_ptr->height = static_cast<float>(height);
	application_buffer_ptr->width_inv = static_cast<float>(1.0f / width);
	application_buffer_ptr->height_inv = static_cast<float>(1.0f / height);
	application_buffer_ptr->time = current_time;

	result = opalUnmapBuffer(device, application_buffer);
	assert(result == OPAL_SUCCESS);

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

	Opal_SwapchainDesc swapchain_desc =
	{
		OPAL_PRESENT_MODE_MAILBOX,
		OPAL_FORMAT_BGRA8_UNORM,
		OPAL_COLORSPACE_SRGB,
		(Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT | OPAL_TEXTURE_USAGE_SHADER_SAMPLED),
		surface
	};

	result = opalCreateSwapchain(device, &swapchain_desc, &swapchain);
	assert(result == OPAL_SUCCESS);
}

void Application::render()
{
	assert(current_in_flight_frame < IN_FLIGHT_FRAMES);

	Opal_TextureView swapchain_texture_view = OPAL_NULL_HANDLE;
	Opal_CommandBuffer command_buffer = command_buffers[current_in_flight_frame];
	Opal_Semaphore semaphore = semaphores[current_in_flight_frame];
	uint64_t semaphore_value = semaphore_values[current_in_flight_frame];

	Opal_Result result = opalAcquire(device, swapchain, &swapchain_texture_view);
	assert(result == OPAL_SUCCESS);

	result = opalWaitSemaphore(device, semaphore, semaphore_value, WAIT_TIMEOUT_MS);
	assert(result == OPAL_SUCCESS);

	result = opalResetCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_FramebufferAttachment attachments =
	{
		swapchain_texture_view,
		OPAL_NULL_HANDLE,
		OPAL_LOAD_OP_CLEAR,
		OPAL_STORE_OP_STORE,
		{0.4f, 0.4f, 0.4f, 1.0f}
	};

	result = opalCmdTextureTransitionBarrier(device, command_buffer, swapchain_texture_view, OPAL_RESOURCE_STATE_GENERAL, OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT);
	assert(result == OPAL_SUCCESS);

	result = opalCmdBeginGraphicsPass(device, command_buffer, 1, &attachments, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalCmdSetBindsets(device, command_buffer, pipeline_layout, 1, &bindset);
	assert(result == OPAL_SUCCESS);

	result = opalCmdSetPipeline(device, command_buffer, pipeline);
	assert(result == OPAL_SUCCESS);

	Opal_BufferView vertex_buffer = {triangle_buffer, 0};
	Opal_BufferView index_buffer = {triangle_buffer, offsetof(TriangleData, indices)};

	result = opalCmdSetVertexBuffers(device, command_buffer, 1, &vertex_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalCmdSetIndexBuffer(device, command_buffer, index_buffer, OPAL_INDEX_FORMAT_UINT32);
	assert(result == OPAL_SUCCESS);

	Opal_Viewport viewport =
	{
		0, 0,
		static_cast<float>(width), static_cast<float>(height),
		0.0f, 1.0f,
	};

	result = opalCmdSetViewport(device, command_buffer, viewport);
	assert(result == OPAL_SUCCESS);

	result = opalCmdSetScissor(device, command_buffer, 0, 0, width, height);
	assert(result == OPAL_SUCCESS);

	result = opalCmdDrawIndexedInstanced(device, command_buffer, 6, 0, 1, 0);
	assert(result == OPAL_SUCCESS);

	result = opalCmdEndGraphicsPass(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalCmdTextureTransitionBarrier(device, command_buffer, swapchain_texture_view, OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT, OPAL_RESOURCE_STATE_PRESENT);
	assert(result == OPAL_SUCCESS);

	result = opalEndCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	semaphore_value++;
	semaphore_values[current_in_flight_frame] = semaphore_value;

	Opal_SubmitDesc submit = {0};
	submit.num_wait_swapchains = 1;
	submit.wait_swapchains = &swapchain;
	submit.num_command_buffers = 1;
	submit.command_buffers = &command_buffer;
	submit.num_signal_swapchains = 1;
	submit.signal_swapchains = &swapchain;
	submit.num_signal_semaphores = 1;
	submit.signal_semaphores = &semaphore;
	submit.signal_values = &semaphore_value;

	result = opalSubmit(device, queue, &submit);
	assert(result == OPAL_SUCCESS);
}

void Application::present()
{
	assert(current_in_flight_frame < IN_FLIGHT_FRAMES);

	Opal_Result result = opalPresent(device, swapchain);
	assert(result == OPAL_SUCCESS);

	current_in_flight_frame = (current_in_flight_frame + 1) % IN_FLIGHT_FRAMES;
}

#ifdef OPAL_PLATFORM_WINDOWS

/*
 */
LRESULT CALLBACK windowProc(HWND handle, UINT message, WPARAM w_param, LPARAM l_param)
{
	Application *app = reinterpret_cast<Application *>(GetWindowLongPtrW(handle, GWLP_USERDATA));

	switch (message)
	{
		case WM_SIZE:
		{
			uint32_t width = static_cast<uint32_t>(LOWORD(l_param));
			uint32_t height = static_cast<uint32_t>(HIWORD(l_param));

			if (app)
				app->resize(width, height);
		}
		break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);
		}
		break;

		default:
			return DefWindowProcW(handle, message, w_param, l_param);
	}

	return 0;
}

HWND createWindow(const char *title, uint32_t width, uint32_t height)
{
	HINSTANCE instance = GetModuleHandle(nullptr);
	WNDCLASSEXW wc = {};

	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = windowProc;
	wc.hInstance = instance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"Opal Window";

	RegisterClassExW(&wc);

	assert(title);
	int size = static_cast<int>(strlen(title));

	wchar_t titlew[4096] = {};
	MultiByteToWideChar(CP_UTF8, 0, title, size, titlew, 4096);

	RECT rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindowExW(
		0,
		wc.lpszClassName,
		titlew,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		instance,
		NULL
	);

	assert(handle != NULL);
	return handle;
}

void destroyWindow(HWND handle)
{
	DestroyWindow(handle);
}

bool updateShaders()
{
	static std::filesystem::file_time_type last_vertex_mtime;
	static std::filesystem::file_time_type last_fragment_mtime;

	std::filesystem::file_time_type vertex_mtime = std::filesystem::last_write_time("samples/05_opal/shaders/vulkan/main.vert");
	std::filesystem::file_time_type fragment_mtime = std::filesystem::last_write_time("samples/05_opal/shaders/vulkan/main.frag");

	bool need_update = false;

	if (vertex_mtime != last_vertex_mtime)
	{
		std::cout << "Rebuilding vertex shader..." << std::endl;
		int result = std::system("glslc samples/05_opal/shaders/vulkan/main.vert -o samples/05_opal/shaders/vulkan/main.vert.spv");
		last_vertex_mtime = vertex_mtime;
		need_update = (result == 0);
	}

	if (fragment_mtime != last_fragment_mtime)
	{
		std::cout << "Rebuilding fragment shader..." << std::endl;
		int result = std::system("glslc samples/05_opal/shaders/vulkan/main.frag -o samples/05_opal/shaders/vulkan/main.frag.spv");
		last_fragment_mtime = fragment_mtime;
		need_update = (result == 0);
	}

	return need_update;
}

void mainloop()
{
	const char *title = "Opal Sample (05_opal) Привет! ÁÉ¢¿耷靼";
	const uint32_t width = 800;
	const uint32_t height = 600;

	Application app;

	HWND handle = createWindow(title, width, height);

	SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&app));
	ShowWindow(handle, SW_SHOW);
	UpdateWindow(handle);

	app.init(handle, width, height);

	MSG msg = {};
	LARGE_INTEGER begin_time = {};
	LARGE_INTEGER end_time = {};
	LARGE_INTEGER frequency = {};
	float dt = 0.0f;

	QueryPerformanceFrequency(&frequency);
	double denominator = 1.0 / frequency.QuadPart;

	while (true)
	{
		QueryPerformanceCounter(&begin_time);
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		if (updateShaders())
			app.buildPipeline();

		app.update(dt);
		app.render();
		app.present();

		QueryPerformanceCounter(&end_time);
		dt = static_cast<float>((end_time.QuadPart - begin_time.QuadPart) * denominator);
	}

	app.shutdown();

	destroyWindow(handle);
}

#else

void mainloop()
{

}

#endif

/*
 */
int main()
{
	mainloop();
	return 0;
}
