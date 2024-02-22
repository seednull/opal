#include "null_internal.h"

static Opal_DeviceInfo defaultInfo =
{
	"Null Device",
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	256,
};

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
Opal_Result null_instanceEnumerateDevices(Instance *this, int *device_count, Opal_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);

	*device_count = 1;

	if (infos)
	{
		memcpy(&infos[0], &defaultInfo, sizeof(Opal_DeviceInfo));
	}

	return OPAL_SUCCESS;
}

Opal_Result null_instanceCreateDefaultDevice(Instance *this, Opal_DefaultDeviceHint hint, Opal_Device *device)
{
	assert(this);
	assert(device);

	Null_Device *ptr = (Null_Device *)malloc(sizeof(Null_Device));

	// vtable
	ptr->vtbl.getInfo = null_deviceGetInfo;
	ptr->vtbl.destroy = null_deviceDestroy;

	// data
	memcpy(&ptr->info, &defaultInfo, sizeof(Opal_DeviceInfo));

	*device = (Opal_Device)ptr;
	return OPAL_SUCCESS;
}

Opal_Result null_instanceCreateDevice(Instance *this, int index, Opal_Device *device)
{
	assert(this);
	assert(device);
	assert(index == 0);

	Null_Device *ptr = (Null_Device *)malloc(sizeof(Null_Device));

	// vtable
	ptr->vtbl.getInfo = null_deviceGetInfo;
	ptr->vtbl.destroy = null_deviceDestroy;

	// data
	memcpy(&ptr->info, &defaultInfo, sizeof(Opal_DeviceInfo));

	*device = (Opal_Device)ptr;

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
