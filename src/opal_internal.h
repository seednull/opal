#pragma once

#include <opal.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct Instance_t Instance;
typedef struct Device_t Device;

typedef Opal_Result (*PFN_instanceEnumerateDevices)(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos);
typedef Opal_Result (*PFN_instanceCreateDefaultDevice)(Instance *this, Opal_DeviceHint hint, Opal_Device *device);
typedef Opal_Result (*PFN_instanceCreateDevice)(Instance *this, uint32_t index, Opal_Device *device);
typedef Opal_Result (*PFN_instanceDestroy)(Instance *this);

typedef Opal_Result (*PFN_deviceGetInfo)(Device *this, Opal_DeviceInfo *info);
typedef Opal_Result (*PFN_deviceDestroy)(Device *this);

typedef Opal_Result (*PFN_deviceCreateBuffer)(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
typedef Opal_Result (*PFN_deviceCreateTexture)(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture);
typedef Opal_Result (*PFN_deviceCreateTextureView)(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

typedef Opal_Result (*PFN_deviceDestroyBuffer)(Device *this, Opal_Buffer buffer);
typedef Opal_Result (*PFN_deviceDestroyTexture)(Device *this, Opal_Texture texture);
typedef Opal_Result (*PFN_deviceDestroyTextureView)(Device *this, Opal_TextureView texture_view);

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
	PFN_deviceCreateBuffer createBuffer;
	PFN_deviceCreateTexture createTexture;
	PFN_deviceCreateTextureView createTextureView;
	PFN_deviceDestroyBuffer destroyBuffer;
	PFN_deviceDestroyTexture destroyTexture;
	PFN_deviceDestroyTextureView destroyTextureView;
} Device;

extern Opal_Result vulkan_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
extern Opal_Result directx12_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
extern Opal_Result metal_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
extern Opal_Result webgpu_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
extern Opal_Result null_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);

extern uint32_t opalEvaluateDevice(const Opal_DeviceInfo *info, Opal_DeviceHint hint);
