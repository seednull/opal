#include "directx12_internal.h"
#include <assert.h>

/*
 */
static Opal_InstanceTable instance_vtbl =
{
	directx12_instanceDestroy,
	directx12_instanceEnumerateDevices,
	directx12_instanceCreateDevice,
	directx12_instanceCreateDefaultDevice,
};

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
	ptr->vtbl = &instance_vtbl;

	// data
	ptr->factory = factory;

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;
}

/*
 */
Opal_Result directx12_instanceEnumerateDevices(Opal_Instance this, uint32_t *device_count, Opal_DeviceInfo *infos)
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

Opal_Result directx12_instanceCreateDefaultDevice(Opal_Instance this, Opal_DeviceHint hint, Opal_Device *device)
{
	assert(this);
	assert(device);

	DirectX12_Instance *instance_ptr = (DirectX12_Instance *)this;
	IDXGIFactory1 *factory = instance_ptr->factory;

	IDXGIAdapter1 *d3d_adapter = NULL;
	ID3D12Device *d3d_device = NULL;

	UINT best_index = 0;

	if (hint != OPAL_DEVICE_HINT_DEFAULT)
	{
		UINT count = 0;

		uint32_t best_score = 0;
		uint32_t current_score = 0;

		Opal_DeviceInfo info = {0};

		while (IDXGIFactory1_EnumAdapters1(factory, count, &d3d_adapter) != DXGI_ERROR_NOT_FOUND)
		{
			HRESULT hr = D3D12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
			if (!SUCCEEDED(hr))
			{
				IDXGIAdapter1_Release(d3d_adapter);
				return OPAL_DIRECX12_ERROR;
			}

			Opal_Result result = directx12_fillDeviceInfoWithDevice(d3d_adapter, d3d_device, &info);
			if (result != OPAL_SUCCESS)
			{
				IDXGIAdapter1_Release(d3d_adapter);
				ID3D12Device_Release(d3d_device);
				return result;
			}

			current_score = opal_evaluateDevice(&info, hint);

			if (best_score < current_score)
			{
				best_score = current_score;
				best_index = count;
			}

			IDXGIAdapter1_Release(d3d_adapter);
			ID3D12Device_Release(d3d_device);

			count++;
		}
	}

	HRESULT hr = IDXGIFactory1_EnumAdapters1(factory, best_index, &d3d_adapter);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECX12_ERROR;

	hr = D3D12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
	if (!SUCCEEDED(hr))
	{
		IDXGIAdapter1_Release(d3d_adapter);
		return OPAL_DIRECX12_ERROR;
	}

	DirectX12_Device *device_ptr = (DirectX12_Device *)malloc(sizeof(DirectX12_Device));
	assert(device_ptr);

	Opal_Result result = directx12_deviceInitialize(device_ptr, instance_ptr, d3d_adapter, d3d_device);
	if (result != OPAL_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Opal_Device)device_ptr);
		return result;
	}

	*device = (Opal_Device)device_ptr;
	return result;
}

Opal_Result directx12_instanceCreateDevice(Opal_Instance this, uint32_t index, Opal_Device *device)
{
	assert(this);
	assert(device);
	assert(index == 0);

	DirectX12_Instance *instance_ptr = (DirectX12_Instance *)this;
	IDXGIFactory1 *factory = instance_ptr->factory;

	IDXGIAdapter1 *d3d_adapter = NULL;
	HRESULT hr = IDXGIFactory1_EnumAdapters1(factory, index, &d3d_adapter);
	if (!SUCCEEDED(hr))
		return OPAL_INVALID_DEVICE_INDEX;

	ID3D12Device *d3d_device = NULL;
	hr = D3D12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
	if (!SUCCEEDED(hr))
	{
		IDXGIAdapter1_Release(d3d_adapter);
		return OPAL_DIRECX12_ERROR;
	}

	DirectX12_Device *device_ptr = (DirectX12_Device *)malloc(sizeof(DirectX12_Device));
	assert(device_ptr);

	Opal_Result result = directx12_deviceInitialize(device_ptr, instance_ptr, d3d_adapter, d3d_device);
	if (result != OPAL_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Opal_Device)device_ptr);
		return result;
	}

	*device = (Opal_Device)device_ptr;
	return OPAL_SUCCESS;
}

Opal_Result directx12_instanceDestroy(Opal_Instance this)
{
	assert(this);

	DirectX12_Instance *ptr = (DirectX12_Instance *)this;
	IDXGIFactory1_Release(ptr->factory);

	free(ptr);
	return OPAL_SUCCESS;
}
