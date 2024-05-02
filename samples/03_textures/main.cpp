#include <opal.h>
#include <cassert>
#include <iostream>

void testTextures(Opal_Device device)
{
	Opal_Texture textures[2] {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};

	Opal_TextureDesc desc =
	{
		OPAL_TEXTURE_TYPE_2D,
		OPAL_TEXTURE_FORMAT_RGBA8_UNORM,
		128,
		128,
		1,
		1,
		1,
		OPAL_TEXTURE_SAMPLES_1,
		(Opal_TextureUsageFlags)(OPAL_TEXTURE_USAGE_SHADER_SAMPLED | OPAL_TEXTURE_USAGE_RENDER_ATTACHMENT),
		OPAL_ALLOCATION_HINT_AUTO,
	};

	Opal_Result result = opalCreateTexture(device, &desc, &textures[0]);
	assert(result == OPAL_SUCCESS);

	result = opalCreateTexture(device, &desc, &textures[1]);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyTexture(device, textures[0]);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyTexture(device, textures[1]);
	assert(result == OPAL_SUCCESS);
}

int main()
{
	Opal_Instance instance {OPAL_NULL_HANDLE};

	Opal_InstanceDesc instance_desc = {
		"03_textures",
		"Opal",
		0,
		0,
		OPAL_DEFAULT_HEAP_SIZE,
		OPAL_DEFAULT_HEAP_ALLOCATIONS,
		OPAL_DEFAULT_HEAPS,
	};

	Opal_Result result = opalCreateInstance(OPAL_API_VULKAN, &instance_desc, &instance);
	assert(result == OPAL_SUCCESS);

	Opal_Device device {OPAL_NULL_HANDLE};
	result = opalCreateDefaultDevice(instance, OPAL_DEVICE_HINT_DEFAULT, &device);
	assert(result == OPAL_SUCCESS);

	testTextures(device);

	result = opalDestroyDevice(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyInstance(instance);
	assert(result == OPAL_SUCCESS);

	return 0;
}
