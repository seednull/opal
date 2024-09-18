#pragma once

#include "opal_internal.h"

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <dxgi.h>
#include <d3d12.h>

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

Opal_Result directx12_fillDeviceInfo(IDXGIAdapter1 *adapter, ID3D12Device *device, Opal_DeviceInfo *info);
Opal_Result directx12_fillDeviceLimits(IDXGIAdapter1 *adapter, ID3D12Device *device, Opal_DeviceLimits *limits);

Opal_Result directx12_deviceInitialize(DirectX12_Device *device_ptr, DirectX12_Instance *instance_ptr, IDXGIAdapter1 *adapter, ID3D12Device *device);
