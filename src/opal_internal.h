#pragma once

#include <opal.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct Instance_t Instance;
typedef struct Device_t Device;

typedef Opal_Result (*PFN_instanceEnumerateDevices)(Instance *this, int *device_count, Opal_DeviceInfo *infos);
typedef Opal_Result (*PFN_instanceCreateDefaultDevice)(Instance *this, Opal_DeviceHint hint, Opal_Device *device);
typedef Opal_Result (*PFN_instanceCreateDevice)(Instance *this, int index, Opal_Device *device);
typedef Opal_Result (*PFN_instanceDestroy)(Instance *this);

typedef Opal_Result (*PFN_deviceGetInfo)(Device *this, Opal_DeviceInfo *info);
typedef Opal_Result (*PFN_deviceDestroy)(Device *this);

typedef struct Instance_t
{
	PFN_instanceEnumerateDevices enumerateDevices;
	PFN_instanceCreateDefaultDevice createDefaultDevice;
	PFN_instanceCreateDevice createDevice;
	PFN_instanceDestroy destroy;
} Instance;

typedef struct Device_t
{
	PFN_deviceGetInfo getInfo;
	PFN_deviceDestroy destroy;
} Device;

extern Opal_Result vulkan_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
extern Opal_Result directx12_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
extern Opal_Result metal_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
extern Opal_Result webgpu_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
extern Opal_Result null_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);

extern uint32_t opalEvaluateDevice(const Opal_DeviceInfo *info, Opal_DeviceHint hint);
