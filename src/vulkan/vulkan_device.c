#include "vulkan_internal.h"

/*
 */
Opal_Result vulkan_deviceGetInfo(Device *this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	Vulkan_Device *ptr = (Vulkan_Device *)this;
	return vulkan_fillDeviceInfo(ptr->physical_device, info);
}

Opal_Result vulkan_deviceDestroy(Device *this)
{
	assert(this);

	Vulkan_Device *ptr = (Vulkan_Device *)this;
	vkDestroyDevice(ptr->device, NULL);

	return OPAL_SUCCESS;
}
