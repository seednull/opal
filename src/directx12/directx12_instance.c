#include "directx12_internal.h"

/*
 */
Opal_Result directx12_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	assert(desc);
	assert(instance);

	IDXGIFactory1 *factory = NULL;

	// factory
	HRESULT hr = CreateDXGIFactory1(&IID_IDXGIFactory1, &factory);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECX12_ERROR;

	DirectX12_Instance *ptr = (DirectX12_Instance *)malloc(sizeof(DirectX12_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl.enumerateDevices = directx12_instanceEnumerateDevices;
	ptr->vtbl.createDefaultDevice = directx12_instanceCreateDefaultDevice;
	ptr->vtbl.createDevice = directx12_instanceCreateDevice;
	ptr->vtbl.destroy = directx12_instanceDestroy;

	// data
	ptr->factory = factory;

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;
}

/*
 */
Opal_Result directx12_instanceEnumerateDevices(Instance *this, int *device_count, Opal_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);

	DirectX12_Instance *ptr = (DirectX12_Instance *)this;
	IDXGIFactory1 *factory = ptr->factory;

	UINT count = 0;
	IDXGIAdapter1 *adapter = NULL;

	while (IDXGIFactory1_EnumAdapters1(factory, count, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		if (infos)
		{
			Opal_Result result = directx12_fillDeviceInfo(adapter, &infos[count]);
			if (result != OPAL_SUCCESS)
				return result;
		}

		HRESULT hr = IDXGIAdapter1_Release(adapter);
		if (!SUCCEEDED(hr))
			return OPAL_DIRECX12_ERROR;

		count++;
	}

	*device_count = count;
	return OPAL_SUCCESS;
}

Opal_Result directx12_instanceCreateDefaultDevice(Instance *this, Opal_DefaultDeviceHint hint, Opal_Device *device)
{
	assert(this);
	assert(device);

	DirectX12_Instance *instance_ptr = (DirectX12_Instance *)this;
	IDXGIFactory1 *factory = instance_ptr->factory;

	// TODO: use hints
	IDXGIAdapter1 *d3d_adapter = NULL;
	HRESULT hr = IDXGIFactory1_EnumAdapters1(factory, 0, &d3d_adapter);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECX12_ERROR;

	ID3D12Device *d3d_device = NULL;
	hr = D3D12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
	if (!SUCCEEDED(hr))
	{
		IDXGIAdapter1_Release(d3d_adapter);
		return OPAL_DIRECX12_ERROR;
	}

	DirectX12_Device *ptr = (DirectX12_Device *)malloc(sizeof(DirectX12_Device));

	// vtable
	ptr->vtbl.getInfo = directx12_deviceGetInfo;
	ptr->vtbl.destroy = directx12_deviceDestroy;

	// data
	ptr->adapter = d3d_adapter;
	ptr->device = d3d_device;

	*device = (Opal_Device)ptr;
	return OPAL_SUCCESS;
}

Opal_Result directx12_instanceCreateDevice(Instance *this, int index, Opal_Device *device)
{
	assert(this);
	assert(device);
	assert(index == 0);

	DirectX12_Instance *instance_ptr = (DirectX12_Instance *)this;
	IDXGIFactory1 *factory = instance_ptr->factory;

	IDXGIAdapter1 *d3d_adapter = NULL;
	HRESULT hr = IDXGIFactory1_EnumAdapters1(factory, index, &d3d_adapter);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECX12_ERROR;

	ID3D12Device *d3d_device = NULL;
	hr = D3D12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
	if (!SUCCEEDED(hr))
	{
		IDXGIAdapter1_Release(d3d_adapter);
		return OPAL_DIRECX12_ERROR;
	}

	DirectX12_Device *ptr = (DirectX12_Device *)malloc(sizeof(DirectX12_Device));

	// vtable
	ptr->vtbl.getInfo = directx12_deviceGetInfo;
	ptr->vtbl.destroy = directx12_deviceDestroy;

	// data
	ptr->adapter = d3d_adapter;
	ptr->device = d3d_device;

	*device = (Opal_Device)ptr;

	return OPAL_SUCCESS;
}

Opal_Result directx12_instanceDestroy(Instance *this)
{
	assert(this);

	DirectX12_Instance *ptr = (DirectX12_Instance *)this;
	IDXGIFactory1_Release(ptr->factory);

	return OPAL_SUCCESS;
}
