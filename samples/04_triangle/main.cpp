#ifdef OPAL_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <opal.h>
#include <cassert>
#include <iostream>

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

	Opal_ShaderDesc desc =
	{
		OPAL_SHADER_SOURCE_TYPE_SPIRV_BINARY,
		data,
		size,
	};

	Opal_Result result = opalCreateShader(device, &desc, shader);
	return result == OPAL_SUCCESS;
}

/*
 */
class Application
{
public:
	void init();
	void shutdown();
	void update(float dt);
	void render(Opal_Swapchain swapchain);
	void present(Opal_Swapchain swapchain);

	OPAL_INLINE Opal_Device getDevice() const { return device; }

private:
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

	enum
	{
		IN_FLIGHT_FRAMES = 2,
		WAIT_TIMEOUT_MS = 1000,
	};

private:
	Opal_Instance instance {OPAL_NULL_HANDLE};
	Opal_Device device {OPAL_NULL_HANDLE};
	Opal_Queue queue {OPAL_NULL_HANDLE};
	Opal_Buffer triangle_buffer {OPAL_NULL_HANDLE};
	Opal_Shader vertex_shader {OPAL_NULL_HANDLE};
	Opal_Shader fragment_shader {OPAL_NULL_HANDLE};
	Opal_PipelineLayout pipeline_layout {OPAL_NULL_HANDLE};
	Opal_GraphicsPipeline pipeline {OPAL_NULL_HANDLE};
	Opal_CommandBuffer command_buffers[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};

	uint32_t current_in_flight_frame {0};
};

/*
 */
void Application::init()
{
	// instance & device
	Opal_InstanceDesc instance_desc =
	{
		"04_triangle",
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

	result = opalCreateDefaultDevice(instance, OPAL_DEVICE_HINT_DEFAULT, &device);
	assert(result == OPAL_SUCCESS);

	result = opalGetDeviceQueue(device, OPAL_DEVICE_ENGINE_TYPE_MAIN, 0, &queue);
	assert(result == OPAL_SUCCESS);

	// buffers
	Opal_BufferDesc triangle_buffer_desc =
	{
		static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_VERTEX | OPAL_BUFFER_USAGE_INDEX | OPAL_BUFFER_USAGE_TRANSFER_DST),
		sizeof(TriangleData),
		OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL,
		OPAL_ALLOCATION_HINT_AUTO,
	};

	result = opalCreateBuffer(device, &triangle_buffer_desc, &triangle_buffer);
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
			// position                  color
			 -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			  0.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		},

		// indices
		0, 1, 2,
	};

	memcpy(staging_ptr, &triangle_data, sizeof(TriangleData));

	result = opalUnmapBuffer(device, staging_buffer);
	assert(result == OPAL_SUCCESS);

	// transfer
	Opal_CommandBuffer staging_command_buffer = OPAL_NULL_HANDLE;
	result = opalCreateCommandBuffer(device, OPAL_DEVICE_ENGINE_TYPE_MAIN, &staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommandBuffer(device, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_BufferView staging_buffer_view = { staging_buffer, 0 };
	Opal_BufferView triangle_buffer_view = { triangle_buffer, 0 };

	result = opalCmdCopyBufferToBuffer(device, staging_command_buffer, staging_buffer_view, triangle_buffer_view, sizeof(TriangleData));
	assert(result == OPAL_SUCCESS);

	result = opalEndCommandBuffer(device, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_SubmitDesc submit =
	{
		1, &staging_command_buffer,
		0, nullptr,
		0, nullptr,
	};

	result = opalSubmit(device, queue, &submit);
	assert(result == OPAL_SUCCESS);

	result = opalWaitQueue(device, queue, WAIT_TIMEOUT_MS);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyCommandBuffer(device, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, staging_buffer);
	assert(result == OPAL_SUCCESS);

	// shaders
	bool loaded = loadShader("samples/04_triangle/shaders/vulkan/main.vert.spv", device, &vertex_shader);
	assert(loaded == true);

	loaded = loadShader("samples/04_triangle/shaders/vulkan/main.frag.spv", device, &fragment_shader);
	assert(loaded == true);

	// pipeline
	result = opalCreatePipelineLayout(device, 0, nullptr, &pipeline_layout);
	assert(result == OPAL_SUCCESS);

	Opal_VertexAttribute vertex_attributes[] =
	{
		{ OPAL_FORMAT_RGBA32_SFLOAT, offsetof(Vertex, position) },
		{ OPAL_FORMAT_RGBA32_SFLOAT, offsetof(Vertex, color) },
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

	// command buffer
	for (uint32_t i = 0; i < IN_FLIGHT_FRAMES; ++i)
	{
		result = opalCreateCommandBuffer(device, OPAL_DEVICE_ENGINE_TYPE_MAIN, &command_buffers[i]);
		assert(result == OPAL_SUCCESS);
	}

	current_in_flight_frame = 0;
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

	for (uint32_t i = 0; i < IN_FLIGHT_FRAMES; ++i)
	{
		result = opalDestroyCommandBuffer(device, command_buffers[i]);
		assert(result == OPAL_SUCCESS);
	}

	result = opalDestroyDevice(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyInstance(instance);
	assert(result == OPAL_SUCCESS);
}

void Application::update(float dt)
{

}

void Application::render(Opal_Swapchain swapchain)
{
	assert(current_in_flight_frame < IN_FLIGHT_FRAMES);

	Opal_TextureView swapchain_texture_view = OPAL_NULL_HANDLE;
	Opal_CommandBuffer command_buffer = command_buffers[current_in_flight_frame];

	Opal_Result result = opalAcquire(device, swapchain, &swapchain_texture_view);
	assert(result == OPAL_SUCCESS);

	result = opalWaitQueue(device, queue, WAIT_TIMEOUT_MS);
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

	result = opalCmdTextureTransitionBarrier(device, command_buffer, swapchain_texture_view, OPAL_RESOURCE_STATE_PRESENT, OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT);
	assert(result == OPAL_SUCCESS);

	result = opalCmdBeginGraphicsPass(device, command_buffer, 1, &attachments, NULL);
	assert(result == OPAL_SUCCESS);

	result = opalCmdSetGraphicsPipeline(device, command_buffer, pipeline);
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
		800, 600,
		0.0f, 1.0f,
	};

	result = opalCmdSetViewport(device, command_buffer, viewport);
	assert(result == OPAL_SUCCESS);

	result = opalCmdSetScissor(device, command_buffer, 0, 0, 800, 600);
	assert(result == OPAL_SUCCESS);

	result = opalCmdDrawIndexedInstanced(device, command_buffer, 3, 0, 1, 0);
	assert(result == OPAL_SUCCESS);

	result = opalCmdEndGraphicsPass(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalCmdTextureTransitionBarrier(device, command_buffer, swapchain_texture_view, OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT, OPAL_RESOURCE_STATE_PRESENT);
	assert(result == OPAL_SUCCESS);

	result = opalEndCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_SubmitDesc submit =
	{
		1, &command_buffer,
		0, nullptr,
		1, &swapchain,
	};

	result = opalSubmit(device, queue, &submit);
	assert(result == OPAL_SUCCESS);
}

void Application::present(Opal_Swapchain swapchain)
{
	assert(current_in_flight_frame < IN_FLIGHT_FRAMES);

	Opal_CommandBuffer command_buffer = command_buffers[current_in_flight_frame];

	Opal_Result result = opalPresent(device, swapchain, 1, &command_buffer);
	assert(result == OPAL_SUCCESS);

	current_in_flight_frame = (current_in_flight_frame + 1) % IN_FLIGHT_FRAMES;
}



#ifdef OPAL_PLATFORM_WINDOWS

/*
 */
LRESULT CALLBACK windowProc(HWND handle, UINT message, WPARAM w_param, LPARAM l_param)
{
	return DefWindowProcW(handle, message, w_param, l_param);
}

HWND createWindow(const char *title)
{
	HINSTANCE instance = GetModuleHandle(nullptr);
	WNDCLASSW wc = {};

	wc.lpfnWndProc = windowProc;
	wc.hInstance = instance;
	wc.lpszClassName = L"Opal Window";

	RegisterClassW(&wc);

	assert(title);
	int size = static_cast<int>(strlen(title));

	wchar_t titlew[4096] = {};
	MultiByteToWideChar(CP_UTF8, 0, title, size, titlew, 4096);

	RECT rect = {0, 0, 800, 600};
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

void mainloop()
{
	const char *title = "Opal Sample (04_triangle) Привет! ÁÉ¢¿耷靼";

	bool running = true;

	Application app;
	app.init();

	HWND handle = createWindow(title);
	ShowWindow(handle, SW_SHOW);

	Opal_SwapchainDesc swapchain_desc =
	{
		OPAL_PRESENT_MODE_MAILBOX,
		OPAL_FORMAT_BGRA8_UNORM,
		(Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT | OPAL_TEXTURE_USAGE_SHADER_SAMPLED),
		handle
	};

	Opal_Swapchain swapchain = OPAL_NULL_HANDLE;
	Opal_Result result = opalCreateSwapchain(app.getDevice(), &swapchain_desc, &swapchain);
	assert(result == OPAL_SUCCESS);
	
	MSG msg = {};
	LARGE_INTEGER begin_time = {};
	LARGE_INTEGER end_time = {};
	LARGE_INTEGER frequency = {};
	float dt = 0.0f;

	QueryPerformanceFrequency(&frequency);
	double denominator = 1.0 / frequency.QuadPart;

	while (running)
	{
		QueryPerformanceCounter(&begin_time);
		if (PeekMessage(&msg, handle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		app.update(dt);
		app.render(swapchain);
		app.present(swapchain);

		QueryPerformanceCounter(&end_time);
		dt = static_cast<float>((end_time.QuadPart - begin_time.QuadPart) * denominator);
	}

	opalDestroySwapchain(app.getDevice(), swapchain);
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
