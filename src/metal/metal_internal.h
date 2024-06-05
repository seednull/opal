#pragma once

#include "opal_internal.h"

#import <Metal/Metal.h>

typedef struct Metal_Instance_t
{
	Opal_InstanceTable *vtbl;
	char *application_name;
	uint32_t application_version;
	char *engine_name;
	uint32_t engine_version;
} Metal_Instance;

typedef struct Metal_Device_t
{
	Opal_DeviceTable *vtbl;
	id<MTLDevice> device;
} Metal_Device;

Opal_Result metal_deviceInitialize(Metal_Device *device_ptr, Metal_Instance *instance_ptr, id<MTLDevice> device);
Opal_Result metal_fillDeviceInfo(id<MTLDevice> device, Opal_DeviceInfo *info);
