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

extern Opal_Result null_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
extern Opal_Result null_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture);
extern Opal_Result null_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

extern Opal_Result null_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr);
extern Opal_Result null_deviceUnmapBuffer(Device *this, Opal_Buffer buffer);

extern Opal_Result null_deviceDestroyBuffer(Device *this, Opal_Buffer buffer);
extern Opal_Result null_deviceDestroyTexture(Device *this, Opal_Texture texture);
extern Opal_Result null_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view);
