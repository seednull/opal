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
	void buildLayout();
	void buildContent();
	void buildDescriptors();
	void buildPipelines();

	void destroyLayout();
	void destroyContent();
	void destroyDescriptors();
	void destroyPipelines();

private:
	struct EmitterData
	{
		uint32_t counters[4];
		float random[4];
	};

	struct AppData
	{
		float width {0.0f};
		float height {0.0f};
		float width_inv {0.0f};
		float height_inv {0.0f};
		float time {0.0f};
		float dt {0.0f};
	};

	struct CameraData
	{
		float view[16];
		float projection[16];
	};

	static constexpr float DEG2RAD = 0.0174532925f;
	static constexpr float MIN_PARTICLE_LIFETIME = 1.0f;
	static constexpr float MAX_PARTICLE_LIFETIME = 10.0f;
	static constexpr float MIN_PARTICLE_IMASS = 1.0f;
	static constexpr float MAX_PARTICLE_IMASS = 0.01f;
	static constexpr uint32_t NUM_PARTICLES = 10000000;
	static constexpr uint32_t WAIT_TIMEOUT_MS = 1000;

private:
	Opal_Instance instance {OPAL_NULL_HANDLE};
	Opal_Device device {OPAL_NULL_HANDLE};
	Opal_Queue queue {OPAL_NULL_HANDLE};

	Opal_Surface surface {OPAL_NULL_HANDLE};
	Opal_Swapchain swapchain {OPAL_NULL_HANDLE};
	Opal_Semaphore semaphore {OPAL_NULL_HANDLE};
	Opal_CommandAllocator command_allocator {OPAL_NULL_HANDLE};
	Opal_CommandBuffer command_buffer {OPAL_NULL_HANDLE};
	Opal_Fence fence {OPAL_NULL_HANDLE};
	Opal_DescriptorHeap descriptor_heap {OPAL_NULL_HANDLE};

	Opal_DescriptorSetLayout compute_descriptor_set_layout {OPAL_NULL_HANDLE};
	Opal_DescriptorSet compute_descriptor_set {OPAL_NULL_HANDLE};
	Opal_PipelineLayout compute_pipeline_layout {OPAL_NULL_HANDLE};

	Opal_Buffer render_application {OPAL_NULL_HANDLE};
	Opal_Buffer render_camera {OPAL_NULL_HANDLE};

	Opal_Buffer mesh {OPAL_NULL_HANDLE};

	Opal_Buffer particle_positions {OPAL_NULL_HANDLE};
	Opal_Buffer particle_velocities {OPAL_NULL_HANDLE};
	Opal_Buffer particle_parameters {OPAL_NULL_HANDLE};
	Opal_Buffer particle_indices {OPAL_NULL_HANDLE};
	Opal_Buffer particle_counters {OPAL_NULL_HANDLE};

	Opal_DescriptorSetLayout render_descriptor_set_layout {OPAL_NULL_HANDLE};
	Opal_DescriptorSet render_descriptor_set {OPAL_NULL_HANDLE};
	Opal_PipelineLayout render_pipeline_layout {OPAL_NULL_HANDLE};

	Opal_Shader emit_shader {OPAL_NULL_HANDLE};
	Opal_ComputePipeline emit_pipeline {OPAL_NULL_HANDLE};

	Opal_Shader simulate_shader {OPAL_NULL_HANDLE};
	Opal_ComputePipeline simulate_pipeline {OPAL_NULL_HANDLE};

	Opal_Shader render_vertex_shader {OPAL_NULL_HANDLE};
	Opal_Shader render_fragment_shader {OPAL_NULL_HANDLE};
	Opal_GraphicsPipeline render_pipeline {OPAL_NULL_HANDLE};

	uint64_t current_frame {0};
	float current_time {0.0f};

	uint32_t width {0};
	uint32_t height {0};
};
