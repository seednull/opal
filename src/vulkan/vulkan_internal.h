#pragma once

#include "opal_internal.h"

#include <volk.h>

typedef struct Vulkan_Instance_t
{
	Instance vtbl;
	VkInstance instance;
} Vulkan_Instance;

typedef struct Vulkan_Device_t
{
	Device vtbl;
	VkPhysicalDevice physical_device;
	VkDevice device;
} Vulkan_Device;

extern Opal_Result vulkan_createDevice(VkPhysicalDevice physical_device, VkDevice *device);
extern Opal_Result vulkan_fillDeviceInfo(VkPhysicalDevice device, Opal_DeviceInfo *info);

extern Opal_Result vulkan_instanceEnumerateDevices(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos);
extern Opal_Result vulkan_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device);
extern Opal_Result vulkan_instanceCreateDevice(Instance *this, uint32_t index, Opal_Device *device);
extern Opal_Result vulkan_instanceDestroy(Instance *this);

extern Opal_Result vulkan_deviceGetInfo(Device *this, Opal_DeviceInfo *info);
extern Opal_Result vulkan_deviceDestroy(Device *this);
