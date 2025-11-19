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
	void buildPipelineLayout();
	void buildPipeline();
	void buildSBT();
	void buildBLAS();
	void buildTLAS();

private:
	struct Camera
	{
		float pos[4];
		float dir[4];
	};

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

	Opal_DeviceInfo device_info {};

	Opal_AccelerationStructure blas {OPAL_NULL_HANDLE};
	Opal_AccelerationStructure tlas {OPAL_NULL_HANDLE};

	Opal_ShaderBindingTable sbt {OPAL_NULL_HANDLE};

	Opal_Buffer camera_buffer {OPAL_NULL_HANDLE};
	Camera *camera_ptr {nullptr};

	Opal_PipelineLayout pipeline_layout {OPAL_NULL_HANDLE};
#ifndef OPAL_PLATFORM_MACOS
	Opal_RaytracePipeline raytrace_pipeline {OPAL_NULL_HANDLE};
#else
	Opal_ComputePipeline compute_pipeline {OPAL_NULL_HANDLE};
#endif

	Opal_DescriptorHeap descriptor_heap {OPAL_NULL_HANDLE};
	Opal_DescriptorSetLayout descriptor_set_layout {OPAL_NULL_HANDLE};
	
	Opal_Surface surface {OPAL_NULL_HANDLE};
	Opal_Swapchain swapchain {OPAL_NULL_HANDLE};
	Opal_Semaphore semaphore {OPAL_NULL_HANDLE};
	
	Opal_DescriptorSet descriptor_sets[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};
	Opal_CommandAllocator command_allocators[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE};
	Opal_CommandBuffer command_buffers[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};
	Opal_Texture frame_textures[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};
	Opal_TextureView frame_texture_views[IN_FLIGHT_FRAMES] {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};
	bool frame_initialized[IN_FLIGHT_FRAMES] {false, false};

	uint64_t current_frame {0};
	uint64_t wait_frame {0};

	uint32_t width {0};
	uint32_t height {0};
};
