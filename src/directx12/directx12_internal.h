#pragma once

#include "opal_internal.h"

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <dxgi.h>
#include <directx/d3d12.h>

typedef struct DirectX12_Instance_t
{
	Opal_InstanceTable *vtbl;
	IDXGIFactory1 *factory;
} DirectX12_Instance;

typedef struct DirectX12_Device_t
{
	Opal_DeviceTable *vtbl;
	IDXGIAdapter1 *adapter;
	ID3D12Device *device;
} DirectX12_Device;

Opal_Result directx12_deviceInitialize(DirectX12_Device *device_ptr, DirectX12_Instance *instance_ptr, IDXGIAdapter1 *adapter, ID3D12Device *device);
Opal_Result directx12_fillDeviceInfo(IDXGIAdapter1 *adapter, Opal_DeviceInfo *info);
Opal_Result directx12_fillDeviceInfoWithDevice(IDXGIAdapter1 *adapter, ID3D12Device *device, Opal_DeviceInfo *info);

Opal_Result directx12_instanceEnumerateDevices(Opal_Instance this, uint32_t *device_count, Opal_DeviceInfo *infos);
Opal_Result directx12_instanceCreateDefaultDevice(Opal_Instance this, Opal_DeviceHint hint, Opal_Device *device);
Opal_Result directx12_instanceCreateDevice(Opal_Instance this, uint32_t index, Opal_Device *device);
Opal_Result directx12_instanceDestroy(Opal_Instance this);

Opal_Result directx12_deviceGetInfo(Opal_Device this, Opal_DeviceInfo *info);
Opal_Result directx12_deviceGetQueue(Opal_Device this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue);
Opal_Result directx12_deviceDestroy(Opal_Device this);

Opal_Result directx12_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
Opal_Result directx12_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture);
Opal_Result directx12_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

Opal_Result directx12_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr);
Opal_Result directx12_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer);

Opal_Result directx12_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer);
Opal_Result directx12_deviceDestroyTexture(Opal_Device this, Opal_Texture texture);
Opal_Result directx12_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view);
