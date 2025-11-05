#pragma once

#include <opal.h>

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

	Opal_Surface surface {OPAL_NULL_HANDLE};
	Opal_Swapchain swapchain {OPAL_NULL_HANDLE};
	Opal_Semaphore semaphore {OPAL_NULL_HANDLE};
	Opal_CommandAllocator command_allocators[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE};
	Opal_CommandBuffer command_buffers[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};

	uint64_t current_frame {0};
	uint64_t wait_frame {0};

	uint32_t width {0};
	uint32_t height {0};
};
