#pragma once

#include "opal_internal.h"

typedef struct Null_Instance_t
{
	Instance vtbl;

	char *application_name;
	uint32_t application_version;
	char *engine_name;
	uint32_t engine_version;
} Null_Instance;

typedef struct Null_Device_t
{
	Device vtbl;
	Opal_DeviceInfo info;
} Null_Device;

extern Opal_Result null_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
extern Opal_Result null_instanceEnumerateDevices(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos);
extern Opal_Result null_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device);
extern Opal_Result null_instanceCreateDevice(Instance *this, uint32_t index, Opal_Device *device);
extern Opal_Result null_instanceDestroy(Instance *this);

extern Opal_Result null_deviceGetInfo(Device *this, Opal_DeviceInfo *info);
extern Opal_Result null_deviceDestroy(Device *this);
