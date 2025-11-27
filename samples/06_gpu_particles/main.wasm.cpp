#include <emscripten.h>
#include <emscripten/html5.h>
#include <cassert>

#include "app.h"

static int current_width = 800;
static int current_height = 600;
static bool need_resize = false;
static char canvas_handle[] = "#canvas";

static EM_BOOL onCanvasResize(int event_type, const EmscriptenUiEvent *event, void *user_data)
{
	if (event_type != EMSCRIPTEN_EVENT_RESIZE)
		return EM_FALSE;

	current_width = static_cast<uint32_t>(event->windowInnerWidth);
	current_height = static_cast<uint32_t>(event->windowInnerHeight);

	need_resize = true;
	return EM_FALSE;
}

static EM_BOOL onFullscreenChange(int event_type, const EmscriptenFullscreenChangeEvent *event, void *user_data)
{
	if (event->isFullscreen)
	{
		current_width = event->screenWidth;
		current_height = event->screenHeight;
	}

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
	emscripten_get_canvas_element_size(canvas_handle, &current_width, &current_height);

	Application app;
	app.init(canvas_handle, static_cast<uint32_t>(current_width), static_cast<uint32_t>(current_height));

	emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, onFullscreenChange);
	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 0, onCanvasResize);
	emscripten_set_main_loop_arg(frame, &app, 0, 1);

	app.shutdown();
	return 0;
}
