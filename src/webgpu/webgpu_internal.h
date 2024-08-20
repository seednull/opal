#pragma once

#include "opal_internal.h"

#include <webgpu/webgpu.h>

#include "common/pool.h"

typedef struct WebGPU_Instance_t
{
	Opal_InstanceTable *vtbl;
	WGPUInstance instance;
	Opal_Pool surfaces;
} WebGPU_Instance;

typedef struct WebGPU_Device_t
{
	Opal_DeviceTable *vtbl;
	WGPUAdapter adapter;
	WGPUDevice device;
} WebGPU_Device;

typedef struct WebGPU_Surface_t
{
	WGPUSurface surface;
} WebGPU_Surface;

Opal_Result webgpu_deviceInitialize(WebGPU_Device *device_ptr, WebGPU_Instance *instance_ptr, WGPUAdapter adapter, WGPUDevice device);
Opal_Result webgpu_fillDeviceInfo(WGPUAdapter adapter, Opal_DeviceInfo *info);
