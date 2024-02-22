#pragma once

#include "opal_internal.h"

#define COBJMACROS
#include <dxgi.h>
#include <d3d12.h>

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

extern Opal_Result directx12_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
extern Opal_Result directx12_instanceEnumerateDevices(Instance *this, int *device_count, Opal_DeviceInfo *infos);
extern Opal_Result directx12_instanceCreateDefaultDevice(Instance *this, Opal_DefaultDeviceHint hint, Opal_Device *device);
extern Opal_Result directx12_instanceCreateDevice(Instance *this, int index, Opal_Device *device);
extern Opal_Result directx12_instanceDestroy(Instance *this);

extern Opal_Result directx12_deviceGetInfo(Device *this, Opal_DeviceInfo *info);
extern Opal_Result directx12_deviceDestroy(Device *this);
