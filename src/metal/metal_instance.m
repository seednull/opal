#include "metal_internal.h"

/*
 */
static Metal_Device *opal_createDevice(id<MTLDevice> device)
{
	assert(device);

	Metal_Device *ptr = (Metal_Device *)malloc(sizeof(Metal_Device));
	assert(ptr);

	// vtable
	ptr->vtbl.getInfo = metal_deviceGetInfo;
	ptr->vtbl.destroy = metal_deviceDestroy;
	ptr->vtbl.createBuffer = metal_deviceCreateBuffer;
	ptr->vtbl.createTexture = metal_deviceCreateTexture;
	ptr->vtbl.createTextureView = metal_deviceCreateTextureView;
	ptr->vtbl.mapBuffer = metal_deviceMapBuffer;
	ptr->vtbl.unmapBuffer = metal_deviceUnmapBuffer;
	ptr->vtbl.destroyBuffer = metal_deviceDestroyBuffer;
	ptr->vtbl.destroyTexture = metal_deviceDestroyTexture;
	ptr->vtbl.destroyTextureView = metal_deviceDestroyTextureView;

	// data
	ptr->device = device;

	return ptr;
}

/*
 */
Opal_Result metal_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	assert(desc);
	assert(instance);

	Metal_Instance *ptr = (Metal_Instance *)malloc(sizeof(Metal_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl.enumerateDevices = metal_instanceEnumerateDevices;
	ptr->vtbl.createDefaultDevice = metal_instanceCreateDefaultDevice;
	ptr->vtbl.createDevice = metal_instanceCreateDevice;
	ptr->vtbl.destroy = metal_instanceDestroy;

	// info
	ptr->application_name = strdup(desc->application_name);
	ptr->application_version = desc->application_version;
	ptr->engine_name = strdup(desc->engine_name);
	ptr->engine_version = desc->engine_version;

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;
}

/*
 */
Opal_Result metal_instanceEnumerateDevices(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);

	NSArray<id<MTLDevice>> *devices = MTLCopyAllDevices();
	if (!devices)
		return OPAL_METAL_ERROR;

	if (infos)
	{
		for (uint32_t i = 0; i < devices.count; ++i)
		{
			id<MTLDevice> device = devices[i];

			Opal_Result result = metal_fillDeviceInfo(device, &infos[i]);
			if (result != OPAL_SUCCESS)
			{
				[devices release];
				return result;
			}
		}
	}

	*device_count = devices.count;

	[devices release];
	return OPAL_SUCCESS;
}

Opal_Result metal_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device)
{
	assert(this);
	assert(device);

	NSArray<id<MTLDevice>> *metal_devices = MTLCopyAllDevices();
	if (!metal_devices)
		return OPAL_METAL_ERROR;

	uint32_t best_score = 0;
	id<MTLDevice> best_metal_device = 0;

	for (uint32_t i = 0; i < metal_devices.count; ++i)
	{
		id<MTLDevice> metal_device = metal_devices[i];
		Opal_DeviceInfo info = {0};

		Opal_Result result = metal_fillDeviceInfo(metal_device, &info);
		if (result != OPAL_SUCCESS)
		{
			[metal_devices release];
			return result;
		}

		uint32_t score = opal_evaluateDevice(&info, hint);
		if (best_score < score)
		{
			best_score = score;
			best_metal_device = metal_device;
		}
	}
	[metal_devices release];

	*device = (Opal_Device)opal_createDevice(best_metal_device);
	return OPAL_SUCCESS;
}

Opal_Result metal_instanceCreateDevice(Instance *this, uint32_t index, Opal_Device *device)
{
	assert(this);
	assert(device);

	NSArray<id<MTLDevice>> *metal_devices = MTLCopyAllDevices();
	if (!metal_devices)
		return OPAL_METAL_ERROR;

	if (index >= metal_devices.count)
	{
		[metal_devices release];
		return OPAL_INVALID_DEVICE_INDEX;
	}

	id<MTLDevice> metal_device = metal_devices[index];
	[metal_devices release];

	*device = (Opal_Device)opal_createDevice(metal_device);
	return OPAL_SUCCESS;
}

Opal_Result metal_instanceDestroy(Instance *this)
{
	assert(this);

	Metal_Instance *ptr = (Metal_Instance *)this;

	free(ptr->application_name);
	free(ptr->engine_name);

	return OPAL_SUCCESS;
}
