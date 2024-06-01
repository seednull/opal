#include "null_internal.h"

/*
 */
static void null_fillDefaultDeviceInfo(Opal_DeviceInfo *info)
{
	static const char *device_name = "Null Device";

	assert(info);

	memset(info, 0, sizeof(Opal_DeviceInfo));
	memcpy(info->name, device_name, sizeof(char) * 12);

	info->device_type = OPAL_DEVICE_TYPE_UNKNOWN;
	info->queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 1;
	info->max_buffer_alignment = 256;
}

/*
 */
static Null_Device *null_createDefaultDevice()
{
	Null_Device *ptr = (Null_Device *)malloc(sizeof(Null_Device));
	assert(ptr);

	// vtable
	ptr->vtbl.getInfo = null_deviceGetInfo;
	ptr->vtbl.getQueue = null_deviceGetQueue;
	ptr->vtbl.destroy = null_deviceDestroy;
	ptr->vtbl.createBuffer = null_deviceCreateBuffer;
	ptr->vtbl.createTexture = null_deviceCreateTexture;
	ptr->vtbl.createTextureView = null_deviceCreateTextureView;
	ptr->vtbl.mapBuffer = null_deviceMapBuffer;
	ptr->vtbl.unmapBuffer = null_deviceUnmapBuffer;
	ptr->vtbl.destroyBuffer = null_deviceDestroyBuffer;
	ptr->vtbl.destroyTexture = null_deviceDestroyTexture;
	ptr->vtbl.destroyTextureView = null_deviceDestroyTextureView;

	// data
	null_fillDefaultDeviceInfo(&ptr->info);

	return ptr;
}

/*
 */
Opal_Result null_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	assert(desc);
	assert(instance);

	Null_Instance *ptr = (Null_Instance *)malloc(sizeof(Null_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl.enumerateDevices = null_instanceEnumerateDevices;
	ptr->vtbl.createDefaultDevice = null_instanceCreateDefaultDevice;
	ptr->vtbl.createDevice = null_instanceCreateDevice;
	ptr->vtbl.destroy = null_instanceDestroy;

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
Opal_Result null_instanceEnumerateDevices(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);

	*device_count = 1;

	if (infos)
		null_fillDefaultDeviceInfo(&infos[0]);

	return OPAL_SUCCESS;
}

Opal_Result null_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device)
{
	assert(this);
	assert(device);

	*device = (Opal_Device)null_createDefaultDevice();
	return OPAL_SUCCESS;
}

Opal_Result null_instanceCreateDevice(Instance *this, uint32_t index, Opal_Device *device)
{
	assert(this);
	assert(device);

	if (index != 0)
		return OPAL_INVALID_DEVICE_INDEX;

	*device = (Opal_Device)null_createDefaultDevice();
	return OPAL_SUCCESS;
}

Opal_Result null_instanceDestroy(Instance *this)
{
	assert(this);

	Null_Instance *ptr = (Null_Instance *)this;

	free(ptr->application_name);
	free(ptr->engine_name);

	return OPAL_SUCCESS;
}
