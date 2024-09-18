#include <opal.h>
#include <cassert>
#include <iostream>

static const char *device_types[] =
{
	"Discrete",
	"Integrated",
	"CPU",
	"External",
	"Unknown",
};

static const char *hint_types[] =
{
	"Device",
	"High Performance Device",
	"Low Power Device",
};

void printDeviceInfo(const Opal_DeviceInfo *info)
{
	std::cout << "Device name: " << info->name << "\n";
	std::cout << "Device type: " << device_types[info->device_type] << "\n";
	std::cout << "Driver version: " << info->driver_version << "\n";
	std::cout << "Vendor ID: " << info->vendor_id << "\n";
	std::cout << "Device ID: " << info->device_id << "\n";
	std::cout << "Features:\n";
	std::cout << "\tTessellation Shader: " << (info->tessellation_shader != 0) << "\n";
	std::cout << "\tGeometry Shader: " << (info->geometry_shader != 0) << "\n";
	std::cout << "\tCompute Pipeline: " << (info->compute_pipeline != 0) << "\n";
	std::cout << "\tMeshlet Pipeline: " << (info->meshlet_pipeline != 0) << "\n";
	std::cout << "\tRaytrace Pipeline: " << (info->raytrace_pipeline != 0) << "\n";
	std::cout << "\tETC2 Texture Compression: " << (info->texture_compression_etc2 != 0) << "\n";
	std::cout << "\tASTC Texture Compression: " << (info->texture_compression_astc != 0) << "\n";
	std::cout << "\tBC Texture Compression: " << (info->texture_compression_bc != 0) << "\n";
	std::cout << "\tMain queue count: " << info->queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] << "\n";
	std::cout << "\tCompute queue count: " << info->queue_count[OPAL_DEVICE_ENGINE_TYPE_COMPUTE] << "\n";
	std::cout << "\tCopy queue count: " << info->queue_count[OPAL_DEVICE_ENGINE_TYPE_COPY] << "\n";
	std::cout << "\n";
}

void testEnumerateDevices(Opal_Instance instance)
{
	uint32_t device_count = 0;
	Opal_Result result = opalEnumerateDevices(instance, &device_count, nullptr);
	if (result != OPAL_SUCCESS)
		return;

	Opal_DeviceInfo infos[16];
	result = opalEnumerateDevices(instance, &device_count, &infos[0]);
	assert(result == OPAL_SUCCESS);

	for (uint32_t i = 0; i < device_count; ++i)
	{
		std::cout << " ------ [Enumerate " << i << "] ------\n";
		printDeviceInfo(&infos[i]);
	}
}

void testCreateDevice(Opal_Instance instance, uint32_t index)
{
	Opal_Device device = OPAL_NULL_HANDLE;
	Opal_Result result = opalCreateDevice(instance, index, &device);
	if (result != OPAL_SUCCESS)
		return;

	Opal_DeviceInfo info = {};
	result = opalGetDeviceInfo(device, &info);
	assert(result == OPAL_SUCCESS);
	std::cout << " ------ [CreateByIndex " << index << "] ------\n";
	printDeviceInfo(&info);

	result = opalDestroyDevice(device);
	assert(result == OPAL_SUCCESS);
}

void testCreateDefaultDevice(Opal_Instance instance, Opal_DeviceHint hint)
{
	Opal_Device device = OPAL_NULL_HANDLE;
	Opal_Result result = opalCreateDefaultDevice(instance, hint, &device);
	if (result != OPAL_SUCCESS)
		return;

	Opal_DeviceInfo info = {};
	result = opalGetDeviceInfo(device, &info);
	assert(result == OPAL_SUCCESS);
	std::cout << " ------ [CreateDefault " << hint_types[hint] << "] ------\n";
	printDeviceInfo(&info);

	result = opalDestroyDevice(device);
	assert(result == OPAL_SUCCESS);
}

int main()
{
	Opal_Instance instance = OPAL_NULL_HANDLE;

	Opal_InstanceDesc instance_desc =
	{
		"01_enumerate_devices",
		"Opal",
		0,
		0,
		OPAL_DEFAULT_HEAP_SIZE,
		OPAL_DEFAULT_HEAP_ALLOCATIONS,
		OPAL_DEFAULT_HEAPS,
	};

	Opal_Result result = opalCreateInstance(OPAL_API_AUTO, &instance_desc, &instance);
	assert(result == OPAL_SUCCESS);

	testEnumerateDevices(instance);
	testCreateDevice(instance, 0);
	testCreateDefaultDevice(instance, OPAL_DEVICE_HINT_DEFAULT);
	testCreateDefaultDevice(instance, OPAL_DEVICE_HINT_PREFER_HIGH_PERFORMANCE);
	testCreateDefaultDevice(instance, OPAL_DEVICE_HINT_PREFER_LOW_POWER);

	result = opalDestroyInstance(instance);
	assert(result == OPAL_SUCCESS);

	return 0;
}
