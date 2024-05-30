#include <benchmark/benchmark.h>
#include <opal.h>

template<int T>
class AllocatorBench : public benchmark::Fixture
{
public:
	void SetUp(benchmark::State &state)
	{
		static Opal_InstanceDesc instance_desc = {
			"allocator benchmark",
			"Opal",
			0,
			0,
			OPAL_DEFAULT_HEAP_SIZE,
			OPAL_DEFAULT_HEAP_ALLOCATIONS,
			OPAL_DEFAULT_HEAPS,
			(Opal_InstanceCreationFlags)T
		};

		Opal_Result result = opalCreateInstance(OPAL_API_VULKAN, &instance_desc, &instance);
		assert(result == OPAL_SUCCESS);

		result = opalCreateDefaultDevice(instance, OPAL_DEVICE_HINT_DEFAULT, &device);
		assert(result == OPAL_SUCCESS);

		textures = (Opal_Texture *)malloc(sizeof(Opal_Texture) * 65536);
		buffers = (Opal_Buffer *)malloc(sizeof(Opal_Buffer) * 65536);

		memset(textures, OPAL_NULL_HANDLE, sizeof(Opal_Texture) * 65536);
		memset(buffers, OPAL_NULL_HANDLE, sizeof(Opal_Buffer) * 65536);
	}

	void TearDown(benchmark::State& state)
	{
		Opal_Result result = opalDestroyDevice(device);
		assert(result == OPAL_SUCCESS);

		result = opalDestroyInstance(instance);
		assert(result == OPAL_SUCCESS);

		free(textures);
		free(buffers);
	}

	void LinearAlloc(benchmark::State &state)
	{
		static Opal_TextureDesc texture_desc =
		{
			OPAL_TEXTURE_TYPE_2D,
			OPAL_FORMAT_RGBA8_UNORM,
			128,
			128,
			1,
			1,
			1,
			OPAL_SAMPLES_1,
			(Opal_TextureUsageFlags)OPAL_TEXTURE_USAGE_SHADER_SAMPLED,
			OPAL_ALLOCATION_HINT_AUTO,
		};

		for (auto _ : state)
		{
			for (int64_t i = 0; i < state.range(0); ++i)
			{
				Opal_Result result = opalCreateTexture(device, &texture_desc, &textures[i]);
				assert(result == OPAL_SUCCESS);
			}

			for (int64_t i = 0; i < state.range(0); ++i)
			{
				Opal_Result result = opalDestroyTexture(device, textures[i]);
				assert(result == OPAL_SUCCESS);
			}
		}
	}

	void InterleavedAlloc(benchmark::State &state)
	{
		static Opal_TextureDesc texture_desc =
		{
			OPAL_TEXTURE_TYPE_2D,
			OPAL_FORMAT_RGBA8_UNORM,
			128,
			128,
			1,
			1,
			1,
			OPAL_SAMPLES_1,
			(Opal_TextureUsageFlags)OPAL_TEXTURE_USAGE_SHADER_SAMPLED,
			OPAL_ALLOCATION_HINT_AUTO,
		};

		static Opal_BufferDesc buffer_desc =
		{
			(Opal_BufferUsageFlags)(OPAL_BUFFER_USAGE_UNIFORM),
			512,
			OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL,
			OPAL_ALLOCATION_HINT_AUTO,
		};

		for (auto _ : state)
		{
			for (int64_t i = 0; i < state.range(0); ++i)
			{
				Opal_Result result = opalCreateTexture(device, &texture_desc, &textures[i]);
				assert(result == OPAL_SUCCESS);

				result = opalCreateBuffer(device, &buffer_desc, &buffers[i]);
				assert(result == OPAL_SUCCESS);
			}

			for (int64_t i = 0; i < state.range(0); ++i)
			{
				Opal_Result result = opalDestroyTexture(device, textures[i]);
				assert(result == OPAL_SUCCESS);

				result = opalDestroyBuffer(device, buffers[i]);
				assert(result == OPAL_SUCCESS);
			}
		}
	}

protected:
	Opal_Instance instance {OPAL_NULL_HANDLE};
	Opal_Device device {OPAL_NULL_HANDLE};

	Opal_Texture *textures;
	Opal_Buffer *buffers;
};

BENCHMARK_TEMPLATE_DEFINE_F(AllocatorBench, LinearAllocVanilla, 0)(benchmark::State& state) { LinearAlloc(state); }
BENCHMARK_TEMPLATE_DEFINE_F(AllocatorBench, LinearAllocVma, 1)(benchmark::State& state) { LinearAlloc(state); }

BENCHMARK_TEMPLATE_DEFINE_F(AllocatorBench, InterleavedAllocVanilla, 0)(benchmark::State& state) { InterleavedAlloc(state); }
BENCHMARK_TEMPLATE_DEFINE_F(AllocatorBench, InterleavedAllocVma, 1)(benchmark::State& state) { InterleavedAlloc(state); }

BENCHMARK_REGISTER_F(AllocatorBench, LinearAllocVanilla)
	->Name("LinearAllocVanilla")
	->RangeMultiplier(4)->Range(1, 65536);

BENCHMARK_REGISTER_F(AllocatorBench, LinearAllocVma)
	->Name("LinearAllocVma")
	->RangeMultiplier(4)->Range(1, 65536);

BENCHMARK_REGISTER_F(AllocatorBench, InterleavedAllocVanilla)
	->Name("InterleavedAllocVanilla")
	->RangeMultiplier(4)->Range(1, 65536);

BENCHMARK_REGISTER_F(AllocatorBench, InterleavedAllocVma)
	->Name("InterleavedAllocVma")
	->RangeMultiplier(4)->Range(1, 65536);

BENCHMARK_MAIN();
