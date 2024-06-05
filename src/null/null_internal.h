#pragma once

#include "opal_internal.h"

typedef struct Null_Instance_t
{
	Opal_InstanceTable *vtbl;
	char *application_name;
	uint32_t application_version;
	char *engine_name;
	uint32_t engine_version;
} Null_Instance;

typedef struct Null_Device_t
{
	Opal_DeviceTable *vtbl;
	Opal_DeviceInfo info;
} Null_Device;

Opal_Result null_deviceInitialize(Null_Device *device_ptr, Null_Instance *instance_ptr);
Opal_Result null_fillDeviceInfo(Opal_DeviceInfo *info);

Opal_Result null_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
