#include "webgpu_internal.h"

/*
 */
Opal_Result webgpu_fillDeviceInfo(WGPUAdapter adapter, Opal_DeviceInfo *info)
{
	assert(adapter);
	assert(info);

	WGPUAdapterProperties properties = {0};
	wgpuAdapterGetProperties(adapter, &properties);

	memset(info, 0, sizeof(Opal_DeviceInfo));

	if (properties.name)
		strncpy(info->name, properties.name, 256);

	info->driver_version = 0; // FIXME: is it possible to get uint32 / uint64 driver version for WGPU adapter?
	info->device_id = properties.deviceID;
	info->vendor_id = properties.vendorID;

	switch (properties.adapterType)
	{
		case WGPUAdapterType_DiscreteGPU: info->gpu_type = OPAL_GPU_TYPE_DISCRETE; break;
		case WGPUAdapterType_IntegratedGPU: info->gpu_type = OPAL_GPU_TYPE_INTEGRATED; break;
		case WGPUAdapterType_CPU: info->gpu_type = OPAL_GPU_TYPE_CPU; break;
		default: info->gpu_type = OPAL_GPU_TYPE_UNKNOWN; break;
	}

	info->compute_shader = 1;
	info->texture_compression_etc2 = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionETC2) != 0;
	info->texture_compression_astc = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionASTC) != 0;
	info->texture_compression_bc = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionBC) != 0;
	
	uint32_t offset = 256;  // FIXME: not sure if this is good default
	// FIXME: use wgpuAdapterGetLimits when it'll be implemented

	info->max_buffer_alignment = offset;

	return OPAL_SUCCESS;
}
