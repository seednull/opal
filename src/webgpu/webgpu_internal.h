#pragma once

#include "opal_internal.h"

#include <webgpu/webgpu.h>
#include <emscripten.h>

typedef struct WebGpu_Instance_t
{
	Instance vtbl;
	WGPUInstance instance;
} WebGpu_Instance;

typedef struct WebGpu_Device_t
{
	Device vtbl;
	WGPUAdapter adapter;
	WGPUDevice device;
} WebGpu_Device;

extern Opal_Result webgpu_fillDeviceInfo(WGPUAdapter adapter, Opal_DeviceInfo *info);

extern Opal_Result webgpu_instanceEnumerateDevices(Instance *this, int *device_count, Opal_DeviceInfo *infos);
extern Opal_Result webgpu_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device);
extern Opal_Result webgpu_instanceCreateDevice(Instance *this, int index, Opal_Device *device);
extern Opal_Result webgpu_instanceDestroy(Instance *this);

extern Opal_Result webgpu_deviceGetInfo(Device *this, Opal_DeviceInfo *info);
extern Opal_Result webgpu_deviceDestroy(Device *this);
