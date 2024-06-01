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
		case WGPUAdapterType_DiscreteGPU: info->device_type = OPAL_DEVICE_TYPE_DISCRETE; break;
		case WGPUAdapterType_IntegratedGPU: info->device_type = OPAL_DEVICE_TYPE_INTEGRATED; break;
		case WGPUAdapterType_CPU: info->device_type = OPAL_DEVICE_TYPE_CPU; break;
		default: info->device_type = OPAL_DEVICE_TYPE_UNKNOWN; break;
	}

	info->compute_shader = 1;
	info->texture_compression_etc2 = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionETC2) != 0;
	info->texture_compression_astc = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionASTC) != 0;
	info->texture_compression_bc = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionBC) != 0;
	info->max_buffer_alignment = 256;  // FIXME: not sure if this is good default
	info->queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 1;

	// TODO: use wgpuAdapterGetLimits when it'll be implemented

	return OPAL_SUCCESS;
}
