#pragma once

#include "opal_internal.h"

#import <Metal/Metal.h>

typedef struct Metal_Instance_t
{
	Instance vtbl;
	char *application_name;
	uint32_t application_version;
	char *engine_name;
	uint32_t engine_version;
} Metal_Instance;

typedef struct Metal_Device_t
{
	Device vtbl;
	id<MTLDevice> device;
} Metal_Device;

Opal_Result metal_fillDeviceInfo(id<MTLDevice> device, Opal_DeviceInfo *info);

Opal_Result metal_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
Opal_Result metal_instanceEnumerateDevices(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos);
Opal_Result metal_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device);
Opal_Result metal_instanceCreateDevice(Instance *this, uint32_t index, Opal_Device *device);
Opal_Result metal_instanceDestroy(Instance *this);

Opal_Result metal_deviceGetInfo(Device *this, Opal_DeviceInfo *info);
Opal_Result metal_deviceGetQueue(Device *this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue);
Opal_Result metal_deviceDestroy(Device *this);

Opal_Result metal_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
Opal_Result metal_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture);
Opal_Result metal_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

Opal_Result metal_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr);
Opal_Result metal_deviceUnmapBuffer(Device *this, Opal_Buffer buffer);

Opal_Result metal_deviceDestroyBuffer(Device *this, Opal_Buffer buffer);
Opal_Result metal_deviceDestroyTexture(Device *this, Opal_Texture texture);
Opal_Result metal_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view);
