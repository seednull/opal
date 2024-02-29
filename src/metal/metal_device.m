#include "metal_internal.h"

/*
 */
Opal_Result metal_deviceGetInfo(Device *this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	Metal_Device *ptr = (Metal_Device *)this;
	return metal_fillDeviceInfo(ptr->device, info);
}

/*
 */
Opal_Result metal_deviceDestroy(Device *this)
{
	assert(this);

	Metal_Device *ptr = (Metal_Device *)this;
	// [ptr->device release]; // FIXME: do we need to manually release it here?

	return OPAL_SUCCESS;
}

