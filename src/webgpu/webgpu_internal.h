#pragma once

#include "opal_internal.h"

#include <webgpu/webgpu.h>
#include <emscripten.h>

typedef struct WebGPU_Instance_t
{
	Opal_InstanceTable *vtbl;
	WGPUInstance instance;
} WebGPU_Instance;

typedef struct WebGPU_Device_t
{
	Opal_DeviceTable *vtbl;
	WGPUAdapter adapter;
	WGPUDevice device;
} WebGPU_Device;

Opal_Result webgpu_deviceInitialize(WebGPU_Device *device_ptr, WebGPU_Instance *instance_ptr, WGPUAdapter adapter, WGPUDevice device);
Opal_Result webgpu_fillDeviceInfo(WGPUAdapter adapter, Opal_DeviceInfo *info);

Opal_Result webgpu_instanceEnumerateDevices(Opal_Instance this, uint32_t *device_count, Opal_DeviceInfo *infos);
Opal_Result webgpu_instanceCreateDefaultDevice(Opal_Instance this, Opal_DeviceHint hint, Opal_Device *device);
Opal_Result webgpu_instanceCreateDevice(Opal_Instance this, uint32_t index, Opal_Device *device);
Opal_Result webgpu_instanceDestroy(Opal_Instance this);

Opal_Result webgpu_deviceGetInfo(Opal_Device this, Opal_DeviceInfo *info);
Opal_Result webgpu_deviceGetQueue(Opal_Device this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue);
Opal_Result webgpu_deviceDestroy(Opal_Device this);

Opal_Result webgpu_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
Opal_Result webgpu_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture);
Opal_Result webgpu_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

Opal_Result webgpu_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr);
Opal_Result webgpu_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer);

Opal_Result webgpu_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer);
Opal_Result webgpu_deviceDestroyTexture(Opal_Device this, Opal_Texture texture);
Opal_Result webgpu_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view);
