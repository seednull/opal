#pragma once

#include "opal_internal.h"

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <dxgi.h>
#include <directx/d3d12.h>

typedef struct DirectX12_Instance_t
{
	Instance vtbl;
	IDXGIFactory1 *factory;
} DirectX12_Instance;

typedef struct DirectX12_Device_t
{
	Device vtbl;
	IDXGIAdapter1 *adapter;
	ID3D12Device *device;
} DirectX12_Device;

extern Opal_Result directx12_fillDeviceInfo(IDXGIAdapter1 *adapter, Opal_DeviceInfo *info);
extern Opal_Result directx12_fillDeviceInfoWithDevice(IDXGIAdapter1 *adapter, ID3D12Device *device, Opal_DeviceInfo *info);

extern Opal_Result directx12_instanceEnumerateDevices(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos);
extern Opal_Result directx12_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device);
extern Opal_Result directx12_instanceCreateDevice(Instance *this, uint32_t index, Opal_Device *device);
extern Opal_Result directx12_instanceDestroy(Instance *this);

extern Opal_Result directx12_deviceGetInfo(Device *this, Opal_DeviceInfo *info);
extern Opal_Result directx12_deviceDestroy(Device *this);

extern Opal_Result directx12_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
extern Opal_Result directx12_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture);
extern Opal_Result directx12_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

extern Opal_Result directx12_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr);
extern Opal_Result directx12_deviceUnmapBuffer(Device *this, Opal_Buffer buffer);

extern Opal_Result directx12_deviceDestroyBuffer(Device *this, Opal_Buffer buffer);
extern Opal_Result directx12_deviceDestroyTexture(Device *this, Opal_Texture texture);
extern Opal_Result directx12_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view);
