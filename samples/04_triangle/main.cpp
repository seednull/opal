#ifdef OPAL_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <opal.h>
#include <cassert>
#include <iostream>

/*
 */
class Application
{
public:
	void init();
	void shutdown();
	void update(float dt);
	void render(Opal_SwapChain swap_chain);
	void present(Opal_SwapChain swap_chain);

	OPAL_INLINE Opal_Device getDevice() const { return device; }

private:
	struct TriangleData
	{
		float coordinates[12];
		uint32_t indices[4];
	};

private:
	Opal_Instance instance {OPAL_NULL_HANDLE};
	Opal_Device device {OPAL_NULL_HANDLE};
	Opal_Buffer triangle_buffer {OPAL_NULL_HANDLE};
	Opal_Shader vertex_shader {OPAL_NULL_HANDLE};
	Opal_Shader fragment_shader {OPAL_NULL_HANDLE};
	Opal_BindsetLayout bindset_layout {OPAL_NULL_HANDLE};
	Opal_Bindset bindset {OPAL_NULL_HANDLE};
	Opal_PipelineLayout pipeline_layout {OPAL_NULL_HANDLE};
	Opal_GraphicsPipeline pipeline {OPAL_NULL_HANDLE};
	Opal_CommandBuffer command_buffer {OPAL_NULL_HANDLE};
};

/*
 */
void Application::init()
{
	// instance & device
	Opal_InstanceDesc instance_desc = {
		"04_triangle",
		"Opal",
		0,
		0,
		OPAL_DEFAULT_HEAP_SIZE,
		OPAL_DEFAULT_HEAP_ALLOCATIONS,
		OPAL_DEFAULT_HEAPS,
	};

	Opal_Result result = opalCreateInstance(OPAL_API_VULKAN, &instance_desc, &instance);
	assert(result == OPAL_SUCCESS);

	result = opalCreateDefaultDevice(instance, OPAL_DEVICE_HINT_DEFAULT, &device);
	assert(result == OPAL_SUCCESS);

	// buffers
	Opal_BufferDesc triangle_buffer_desc = {
		static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_VERTEX | OPAL_BUFFER_USAGE_INDEX | OPAL_BUFFER_USAGE_TRANSFER_DST),
		sizeof(TriangleData),
		OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL,
		OPAL_ALLOCATION_HINT_AUTO,
	};

	result = opalCreateBuffer(device, &triangle_buffer_desc, &triangle_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_BufferDesc staging_buffer_desc = {
		static_cast<Opal_BufferUsageFlags>(OPAL_BUFFER_USAGE_TRANSFER_SRC),
		sizeof(TriangleData),
		OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
		OPAL_ALLOCATION_HINT_AUTO,
	};

	Opal_Buffer staging_buffer {OPAL_NULL_HANDLE};
	result = opalCreateBuffer(device, &staging_buffer_desc, &staging_buffer);
	assert(result == OPAL_SUCCESS);

	// copy
	void *staging_ptr = nullptr;
	result = opalMapBuffer(device, staging_buffer, &staging_ptr);
	assert(result == OPAL_SUCCESS);

	TriangleData triangle_data = {
		// vertices
		-1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, 0.0f, 0.0f, 0.0f,
		 0.0f, 1.0f, 0.0f, 0.0f,

		// indices
		0, 1, 2,
	};

	memcpy(staging_ptr, &triangle_data, sizeof(TriangleData));

	result = opalUnmapBuffer(device, staging_buffer);
	assert(result == OPAL_SUCCESS);

	// transfer
	Opal_CommandBuffer staging_command_buffer {OPAL_NULL_HANDLE};
	result = opalCreateCommandBuffer(device, &staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommands(staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_BufferView staging_buffer_view = { staging_buffer, 0 };
	Opal_BufferView triangle_buffer_view = { triangle_buffer, 0 };

	result = opalCmdCopyBufferToBuffer(staging_command_buffer, staging_buffer_view, triangle_buffer_view, sizeof(TriangleData));
	assert(result == OPAL_SUCCESS);

	result = opalEndCommands(staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalSubmitCommands(device, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDeviceWaitIdle(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyCommandBuffer(device, staging_command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, staging_buffer);
	assert(result == OPAL_SUCCESS);

	// shaders
	Opal_ShaderDesc vertex_shader_desc = {
		OPAL_SHADER_SOURCE_TYPE_SPIRV_BINARY,
		nullptr, // TODO:
		0, // TODO:
	};

	Opal_ShaderDesc fragment_shader_desc = {
		OPAL_SHADER_SOURCE_TYPE_SPIRV_BINARY,
		nullptr, // TODO:
		0, // TODO:
	};

	result = opalCreateShader(device, &vertex_shader_desc, &vertex_shader);
	assert(result == OPAL_SUCCESS);

	result = opalCreateShader(device, &fragment_shader_desc, &fragment_shader);
	assert(result == OPAL_SUCCESS);

	// bindings
	Opal_BindsetLayoutBinding bindset_layout_binding = { 0, OPAL_BINDING_TYPE_BUFFER, OPAL_SHADER_STAGE_ALL_GRAPHICS };

	result = opalCreateBindSetLayout(device, 1, &bindset_layout_binding, &bindset_layout);
	assert(result == OPAL_SUCCESS);

	Opal_BindsetBinding bindset_binding = {};
	bindset_binding.binding = 0;
	bindset_binding.buffer = {triangle_buffer, 0};

	result = opalCreateBindSet(device, bindset_layout, 1, &bindset_binding, &bindset);
	assert(result == OPAL_SUCCESS);

	// pipeline
	result = opalCreatePipelineLayout(device, 1, &bindset_layout, &pipeline_layout);
	assert(result == OPAL_SUCCESS);

	Opal_VertexAttribute vertex_attribute = {
		OPAL_FORMAT_RGBA32_SFLOAT,
		0,
	};

	Opal_VertexStream vertex_stream = {
		sizeof(float) * 4,
		1, &vertex_attribute,
		OPAL_VERTEX_INPUT_RATE_VERTEX
	};

	Opal_BlendState blend_state = {};

	Opal_GraphicsPipelineDesc pipeline_desc = {};
	pipeline_desc.pipeline_layout = pipeline_layout;

	pipeline_desc.vertex_shader = vertex_shader;
	pipeline_desc.fragment_shader = fragment_shader;

	pipeline_desc.num_vertex_streams = 1;
	pipeline_desc.vertex_streams = &vertex_stream;
	pipeline_desc.primitive_type = OPAL_PRIMITIVE_TYPE_TRIANGLE;

	pipeline_desc.cull_mode = OPAL_CULL_MODE_BACK;
	pipeline_desc.front_face = OPAL_FRONT_FACE_COUNTER_CLOCKWISE;
	pipeline_desc.rasterization_samples = OPAL_SAMPLES_1;

	pipeline_desc.num_blend_states = 1;
	pipeline_desc.blend_states = &blend_state;

	result = opalCreateGraphicsPipeline(device, &pipeline_desc, &pipeline);
	assert(result == OPAL_SUCCESS);

	// command buffer
	result = opalCreateCommandBuffer(device, &command_buffer);
	assert(result == OPAL_SUCCESS);
}

void Application::shutdown()
{
	Opal_Result result = opalDestroyBuffer(device, triangle_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyShader(device, vertex_shader);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyShader(device, fragment_shader);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBindSetLayout(device, bindset_layout);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBindSet(device, bindset);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyPipelineLayout(device, pipeline_layout);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyGraphicsPipeline(device, pipeline);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyCommandBuffer(device, command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyDevice(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyInstance(instance);
	assert(result == OPAL_SUCCESS);
}

void Application::update(float dt)
{

}

void Application::render(Opal_SwapChain swap_chain)
{
	Opal_TextureView swap_chain_texture_view {OPAL_NULL_HANDLE};

	Opal_Result result = opalAcquire(swap_chain, &swap_chain_texture_view);
	assert(result == OPAL_SUCCESS);

	result = opalBeginCommands(command_buffer);
	assert(result == OPAL_SUCCESS);

	Opal_FramebufferAttachment attachments = {
		swap_chain_texture_view,
		OPAL_LOAD_OP_CLEAR,
		OPAL_STORE_OP_STORE,
		{1.0f, 0.0f, 0.0f, 1.0f}
	};

	result = opalCmdBeginGraphicsPass(command_buffer, 1, &attachments);
	assert(result == OPAL_SUCCESS);

	result = opalCmdSetGraphicsPipeline(command_buffer, pipeline);
	assert(result == OPAL_SUCCESS);

	result = opalCmdSetBindsets(command_buffer, 1, &bindset);
	assert(result == OPAL_SUCCESS);

	Opal_BufferView vertex_buffer = {triangle_buffer, 0};
	Opal_BufferView index_buffer = {triangle_buffer, offsetof(TriangleData, indices)};

	result = opalCmdSetVertexBuffers(command_buffer, 1, &vertex_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalCmdSetIndexBuffer(command_buffer, index_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalCmdDrawIndexedInstanced(command_buffer, 3, 0, 1, 0);
	assert(result == OPAL_SUCCESS);

	result = opalCmdEndGraphicsPass(command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalEndCommands(command_buffer);
	assert(result == OPAL_SUCCESS);

	result = opalSubmitCommands(device, command_buffer);
	assert(result == OPAL_SUCCESS);
}

void Application::present(Opal_SwapChain swap_chain)
{
	Opal_Result result = opalPresent(swap_chain);
	assert(result == OPAL_SUCCESS);
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

	wchar_t titlew[4096];

	int sizew = MultiByteToWideChar(CP_UTF8, 0, title, size, NULL, 0);
	assert(sizew < 4096);

	MultiByteToWideChar(CP_UTF8, 0, title, size, titlew, sizew);
	titlew[sizew] = 0;

	HWND handle = CreateWindowExW(
		0,
		wc.lpszClassName,
		titlew,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
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
	static char title[] = "Opal Sample (04_triangle) Привет! ÁÉ¢¿耷靼";

	bool running = true;

	Application app;
	app.init();

	HWND handle = createWindow(title);
	ShowWindow(handle, SW_SHOW);

	Opal_SwapChain swap_chain {OPAL_NULL_HANDLE};
	Opal_Result result = opalCreateSwapChain(app.getDevice(), handle, &swap_chain);
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
		app.render(swap_chain);
		app.present(swap_chain);

		QueryPerformanceCounter(&end_time);
		dt = static_cast<float>((end_time.QuadPart - begin_time.QuadPart) * denominator);
	}

	opalDestroySwapChain(app.getDevice(), swap_chain);
	app.shutdown();

	destroyWindow(handle);
}

#endif

/*
 */
int main()
{
	mainloop();
	return 0;
}
