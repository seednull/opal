#include "null_internal.h"

/*
 */
Opal_Result null_deviceGetInfo(Device *this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	Null_Device *ptr = (Null_Device *)this;

	memcpy(info, &ptr->info, sizeof(Opal_DeviceInfo));
	return OPAL_SUCCESS;
}

/*
 */
Opal_Result null_deviceDestroy(Device *this)
{
	assert(this);

	Null_Device *ptr = (Null_Device *)this;
	// Nothing to destroy here

	return OPAL_SUCCESS;
}
