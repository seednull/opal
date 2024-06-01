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

Opal_Result webgpu_fillDeviceInfo(WGPUAdapter adapter, Opal_DeviceInfo *info);

Opal_Result webgpu_instanceEnumerateDevices(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos);
Opal_Result webgpu_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device);
Opal_Result webgpu_instanceCreateDevice(Instance *this, uint32_t index, Opal_Device *device);
Opal_Result webgpu_instanceDestroy(Instance *this);

Opal_Result webgpu_deviceGetInfo(Device *this, Opal_DeviceInfo *info);
Opal_Result webgpu_deviceGetQueue(Device *this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue);
Opal_Result webgpu_deviceDestroy(Device *this);

Opal_Result webgpu_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
Opal_Result webgpu_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture);
Opal_Result webgpu_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

Opal_Result webgpu_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr);
Opal_Result webgpu_deviceUnmapBuffer(Device *this, Opal_Buffer buffer);

Opal_Result webgpu_deviceDestroyBuffer(Device *this, Opal_Buffer buffer);
Opal_Result webgpu_deviceDestroyTexture(Device *this, Opal_Texture texture);
Opal_Result webgpu_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view);
