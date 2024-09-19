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
	std::cout << "\tMain queue count: " << info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] << "\n";
	std::cout << "\tCompute queue count: " << info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_COMPUTE] << "\n";
	std::cout << "\tCopy queue count: " << info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_COPY] << "\n";
	std::cout << "\tTessellation Shader: " << (info->features.tessellation_shader != 0) << "\n";
	std::cout << "\tGeometry Shader: " << (info->features.geometry_shader != 0) << "\n";
	std::cout << "\tCompute Pipeline: " << (info->features.compute_pipeline != 0) << "\n";
	std::cout << "\tMeshlet Pipeline: " << (info->features.meshlet_pipeline != 0) << "\n";
	std::cout << "\tRaytrace Pipeline: " << (info->features.raytrace_pipeline != 0) << "\n";
	std::cout << "\tETC2 Texture Compression: " << (info->features.texture_compression_etc2 != 0) << "\n";
	std::cout << "\tASTC Texture Compression: " << (info->features.texture_compression_astc != 0) << "\n";
	std::cout << "\tBC Texture Compression: " << (info->features.texture_compression_bc != 0) << "\n";
	std::cout << "Limits:\n";
	std::cout << "\tMax Texture Dimension 1D: " << info->limits.maxTextureDimension1D << "\n";
	std::cout << "\tMax Texture Dimension 2D: " << info->limits.maxTextureDimension2D << "\n";
	std::cout << "\tMax Texture Dimension 3D: " << info->limits.maxTextureDimension3D << "\n";
	std::cout << "\tMax Texture Array Layers: " << info->limits.maxTextureArrayLayers << "\n";
	std::cout << "\tMax Buffer Size: " << info->limits.maxBufferSize << "\n";
	std::cout << "\tMin Uniform Buffer Offset Alignment: " << info->limits.minUniformBufferOffsetAlignment << "\n";
	std::cout << "\tMin Storage Buffer Offset Alignment: " << info->limits.minStorageBufferOffsetAlignment << "\n";
	std::cout << "\tMax Bindsets: " << info->limits.maxBindsets << "\n";
	std::cout << "\tMax Uniform Buffer Binding Size: " << info->limits.maxUniformBufferBindingSize << "\n";
	std::cout << "\tMax Storage Buffer Binding Size: " << info->limits.maxStorageBufferBindingSize << "\n";
	std::cout << "\tMax Vertex Buffers: " << info->limits.maxVertexBuffers << "\n";
	std::cout << "\tMax Vertex Attributes: " << info->limits.maxVertexAttributes << "\n";
	std::cout << "\tMax Vertex Buffer Stride: " << info->limits.maxVertexBufferStride << "\n";
	std::cout << "\tMax Color Attachments: " << info->limits.maxColorAttachments << "\n";
	std::cout << "\tMax Compute Shared Memory Size: " << info->limits.maxComputeSharedMemorySize << "\n";
	std::cout << "\tMax Compute Workgroup Count X: " << info->limits.maxComputeWorkgroupCountX << "\n";
	std::cout << "\tMax Compute Workgroup Count Y: " << info->limits.maxComputeWorkgroupCountY << "\n";
	std::cout << "\tMax Compute Workgroup Count Z: " << info->limits.maxComputeWorkgroupCountZ << "\n";
	std::cout << "\tMax Compute Workgroup Invocations: " << info->limits.maxComputeWorkgroupInvocations << "\n";
	std::cout << "\tMax Compute Workgroup Local Size X: " << info->limits.maxComputeWorkgroupLocalSizeX << "\n";
	std::cout << "\tMax Compute Workgroup Local Size Y: " << info->limits.maxComputeWorkgroupLocalSizeY << "\n";
	std::cout << "\tMax Compute Workgroup Local Size Z: " << info->limits.maxComputeWorkgroupLocalSizeZ << "\n";
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
