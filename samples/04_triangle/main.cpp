#ifdef OPAL_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif OPAL_PLATFORM_WEB
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <opal.h>
#include <cassert>
#include <iostream>

#define UNUSED(x) do { (void)(x); } while(0)

#ifdef OPAL_PLATFORM_WINDOWS
#define VERTEX_SHADER_PATH "samples/04_triangle/shaders/vulkan/main.vert.spv"
#define FRAGMENT_SHADER_PATH "samples/04_triangle/shaders/vulkan/main.frag.spv"
#define SHADER_TYPE OPAL_SHADER_SOURCE_TYPE_SPIRV_BINARY
#elif OPAL_PLATFORM_WEB
#define VERTEX_SHADER_PATH "samples/04_triangle/shaders/webgpu/main.vert.wgsl"
#define FRAGMENT_SHADER_PATH "samples/04_triangle/shaders/webgpu/main.frag.wgsl"
#define SHADER_TYPE OPAL_SHADER_SOURCE_TYPE_WGSL
#endif

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
	if (type == OPAL_SHADER_SOURCE_TYPE_WGSL || type == OPAL_SHADER_SOURCE_TYPE_MSL)
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
	void update(float dt);
	void resize(uint32_t width, uint32_t height);
	void render();
	void present();

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
	Opal_Surface surface {OPAL_NULL_HANDLE};
	Opal_Swapchain swapchain {OPAL_NULL_HANDLE};
	Opal_Device device {OPAL_NULL_HANDLE};
	Opal_Queue queue {OPAL_NULL_HANDLE};
	Opal_Buffer triangle_buffer {OPAL_NULL_HANDLE};
	Opal_Shader vertex_shader {OPAL_NULL_HANDLE};
	Opal_Shader fragment_shader {OPAL_NULL_HANDLE};
	Opal_PipelineLayout pipeline_layout {OPAL_NULL_HANDLE};
	Opal_Pipeline pipeline {OPAL_NULL_HANDLE};
	Opal_CommandPool command_pool {OPAL_NULL_HANDLE};
	Opal_CommandBuffer command_buffers[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};
	Opal_Semaphore semaphores[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};
	uint64_t semaphore_values[IN_FLIGHT_FRAMES] {0, 0};

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
		"04_triangle",
		"Opal",
		0,
		0,
		OPAL_DEFAULT_HEAP_SIZE,
		OPAL_DEFAULT_HEAP_ALLOCATIONS,
		OPAL_DEFAULT_HEAPS,
		OPAL_INSTANCE_CREATION_FLAGS_USE_VULKAN_VALIDATION_LAYERS,
	};

	Opal_Result result = opalCreateInstance(OPAL_API_AUTO, &instance_desc, &instance);
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
	swapchain_desc.usage = (Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT | OPAL_TEXTURE_USAGE_SHADER_SAMPLED);

	result = opalGetPreferredSurfaceFormat(device, surface, &swapchain_desc.format);
	assert(result == OPAL_SUCCESS);

	result = opalGetPreferredSurfacePresentMode(device, surface, &swapchain_desc.mode);
	assert(result == OPAL_SUCCESS);

	result = opalCreateSwapchain(device, &swapchain_desc, &swapchain);
	assert(result == OPAL_SUCCESS);

	// command pool
	result = opalCreateCommandPool(device, queue, &command_pool);
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

	// copy
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

	void *staging_ptr = nullptr;
	result = opalMapBuffer(device, staging_buffer, &staging_ptr);
	assert(result == OPAL_SUCCESS);

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

	// shaders
	bool loaded = loadShader(VERTEX_SHADER_PATH, SHADER_TYPE, device, &vertex_shader);
	assert(loaded == true);

	loaded = loadShader(FRAGMENT_SHADER_PATH, SHADER_TYPE, device, &fragment_shader);
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

	pipeline_desc.vertex_shader = vertex_shader;
	pipeline_desc.fragment_shader = fragment_shader;

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

	result = opalDestroyPipeline(device, pipeline);
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
	swapchain_desc.usage = (Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT | OPAL_TEXTURE_USAGE_SHADER_SAMPLED);

	result = opalGetPreferredSurfaceFormat(device, surface, &swapchain_desc.format);
	assert(result == OPAL_SUCCESS);

	result = opalGetPreferredSurfacePresentMode(device, surface, &swapchain_desc.mode);
	assert(result == OPAL_SUCCESS);

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

	result = opalCmdSetPipeline(device, command_buffer, pipeline);
	assert(result == OPAL_SUCCESS);

	Opal_BufferView vertex_buffer = {triangle_buffer, 0, sizeof(Vertex) * 3};
	Opal_BufferView index_buffer = {triangle_buffer, offsetof(TriangleData, indices), sizeof(uint32_t) * 3};

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

	result = opalCmdDrawIndexed(device, command_buffer, 3, 1, 0, 0, 0);
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
	UNUSED(result);

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

int main()
{
	const char *title = "Opal Sample (04_triangle) Привет! ÁÉ¢¿耷靼";
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

		app.update(dt);
		app.render();
		app.present();

		QueryPerformanceCounter(&end_time);
		dt = static_cast<float>((end_time.QuadPart - begin_time.QuadPart) * denominator);
	}

	app.shutdown();

	destroyWindow(handle);
	return 0;
}

#elif OPAL_PLATFORM_WEB

static int default_width = 800;
static int default_height = 600;
static int current_width = 800;
static int current_height = 600;
static bool need_resize = false;
static char canvas_handle[] = "#canvas";

static EM_BOOL onFullscreenChange(int event_type, const EmscriptenFullscreenChangeEvent *event, void *user_data)
{
	current_width = (event->isFullscreen) ? event->screenWidth : default_width;
	current_height = (event->isFullscreen) ? event->screenHeight : default_height;

	need_resize = true;
	return EM_FALSE;
}

void frame(void *user_data)
{
	Application *app = reinterpret_cast<Application *>(user_data);
	assert(app);

	if (need_resize)
	{
		assert(current_width > 0 && current_height > 0);
		emscripten_set_canvas_element_size(canvas_handle, current_width, current_height);

		app->resize(static_cast<uint32_t>(current_width), static_cast<uint32_t>(current_height));
		need_resize = false;
	}

	static float dt = 0.0f;
	const float denominator = 1.0f / 1000.0f;

	double start_time = emscripten_performance_now();

	app->update(dt);
	app->render();
	app->present();

	dt = static_cast<float>((emscripten_performance_now() - start_time) * denominator);
}

int main()
{
	emscripten_set_canvas_element_size(canvas_handle, default_width, default_height);

	Application app;
	app.init(canvas_handle, static_cast<uint32_t>(default_width), static_cast<uint32_t>(default_height));

	emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, onFullscreenChange);
	emscripten_set_main_loop_arg(frame, &app, 0, 1);

	app.shutdown();
	return 0;
}

#else

int main()
{
	return 0;
}

#endif
