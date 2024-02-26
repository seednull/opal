#include "webgpu_internal.h"

/*
 */
Opal_Result webgpu_deviceGetInfo(Device *this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	WebGpu_Device *ptr = (WebGpu_Device *)this;
	return webgpu_fillDeviceInfo(ptr->adapter, info);
}

/*
 */
Opal_Result webgpu_deviceDestroy(Device *this)
{
	assert(this);

	WebGpu_Device *ptr = (WebGpu_Device *)this;
	wgpuAdapterRelease(ptr->adapter);
	wgpuDeviceRelease(ptr->device);

	return OPAL_SUCCESS;
}
