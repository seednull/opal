#include <opal.h>
#include <cassert>
#include <iostream>

int main()
{
	Opal_Instance instance {OPAL_NULL_HANDLE};

	Opal_InstanceDesc instance_desc = {
		"01_enumerate_devices",
		0,
		"Opal",
		0
	};

	Opal_Result result = opalCreateInstance(OPAL_API_NULL, &instance_desc, &instance);
	assert(result == OPAL_SUCCESS);

	int device_count = 0;
	result = opalEnumerateDevices(instance, &device_count, nullptr);
	assert(result == OPAL_SUCCESS);

	Opal_DeviceInfo infos[256];
	result = opalEnumerateDevices(instance, &device_count, &infos[0]);
	assert(result == OPAL_SUCCESS);

	for (int i = 0; i < device_count; ++i)
	{
		Opal_DeviceInfo &info = infos[i];
		std::cout << " ------ [" << i << "] ------\n";
		std::cout << "Device name: " << info.name << "\n";
		std::cout << "Driver version: " << info.driver_version << "\n";
		std::cout << "Vendor ID: " << info.vendor_id << "\n";
		std::cout << "Device ID: " << info.device_id << "\n";
		std::cout << "Features:\n";
		std::cout << "\tTessellation Shader: " << (info.tessellation_shader != 0) << "\n";
		std::cout << "\tGeometry Shader: " << (info.geometry_shader != 0) << "\n";
		std::cout << "\tCompute Shader: " << (info.compute_shader != 0) << "\n";
		std::cout << "\tMesh Task Pipeline: " << (info.mesh_task_pipeline != 0) << "\n";
		std::cout << "\tRaytrace Pipeline: " << (info.raytrace_pipeline != 0) << "\n";
		std::cout << "\tETC2 Texture Compression: " << (info.texture_compression_etc2 != 0) << "\n";
		std::cout << "\tASTC Texture Compression: " << (info.texture_compression_astc != 0) << "\n";
		std::cout << "\tBC Texture Compression: " << (info.texture_compression_bc != 0) << "\n";
		std::cout << "Limits:\n";
		std::cout << "\tMax buffer alignment: " << info.max_buffer_alignment << "\n";
		std::cout << "\n";
	}

	Opal_Device device {OPAL_NULL_HANDLE};
	result = opalCreateDefaultDevice(instance, OPAL_DEFAULT_DEVICE_HINT_DEFAULT, &device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyDevice(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyInstance(instance);
	assert(result == OPAL_SUCCESS);

	return 0;
}
