#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cassert>

#include "app.h"

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
	const char *title = "Opal Sample (05_rt_triangle)";
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
