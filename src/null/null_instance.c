#include "null_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Opal_Result null_instanceEnumerateDevices(Opal_Instance this, uint32_t *device_count, Opal_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);

	OPAL_UNUSED(this);

	*device_count = 1;

	if (infos)
		null_fillDeviceInfo(&infos[0]);

	return OPAL_SUCCESS;
}

static Opal_Result null_instanceCreateSurface(Opal_Instance this, void *handle, Opal_Surface *surface)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(handle);
	OPAL_UNUSED(surface);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_instanceCreateDefaultDevice(Opal_Instance this, Opal_DeviceHint hint, Opal_Device *device)
{
	assert(this);
	assert(device);

	OPAL_UNUSED(hint);

	Null_Instance *instance_ptr = (Null_Instance *)this;
	Null_Device *device_ptr = (Null_Device *)malloc(sizeof(Null_Device));
	assert(device_ptr);

	Opal_Result result = null_deviceInitialize(device_ptr, instance_ptr);
	if (result != OPAL_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Opal_Device)device_ptr);
		return result;
	}

	*device = (Opal_Device)device_ptr;
	return OPAL_SUCCESS;
}

static Opal_Result null_instanceCreateDevice(Opal_Instance this, uint32_t index, Opal_Device *device)
{
	assert(this);
	assert(device);

	if (index != 0)
		return OPAL_INVALID_DEVICE_INDEX;

	Null_Instance *instance_ptr = (Null_Instance *)this;
	Null_Device *device_ptr = (Null_Device *)malloc(sizeof(Null_Device));
	assert(device_ptr);

	Opal_Result result = null_deviceInitialize(device_ptr, instance_ptr);
	if (result != OPAL_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Opal_Device)device_ptr);
		return result;
	}

	*device = (Opal_Device)device_ptr;
	return OPAL_SUCCESS;
}

static Opal_Result null_instanceDestroySurface(Opal_Instance this, Opal_Surface surface)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result null_instanceDestroy(Opal_Instance this)
{
	assert(this);

	Null_Instance *ptr = (Null_Instance *)this;

	free(ptr->application_name);
	free(ptr->engine_name);

	free(ptr);
	return OPAL_SUCCESS;
}

/*
 */
static Opal_InstanceTable instance_vtbl =
{
	null_instanceEnumerateDevices,

	null_instanceCreateSurface,
	null_instanceCreateDevice,
	null_instanceCreateDefaultDevice,

	null_instanceDestroySurface,
	null_instanceDestroy,
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
	ptr->vtbl = &instance_vtbl;

	// info
	ptr->application_name = strdup(desc->application_name);
	ptr->application_version = desc->application_version;
	ptr->engine_name = strdup(desc->engine_name);
	ptr->engine_version = desc->engine_version;

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;
}
