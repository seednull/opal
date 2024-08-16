#include "directx12_internal.h"

#include <assert.h>
#include <stdlib.h>

/*
 */
typedef HRESULT (WINAPI* PFN_DXGI_CREATE_FACTORY)(REFIID, _COM_Outptr_ void **);

static PFN_D3D12_CREATE_DEVICE opal_d3d12CreateDevice = NULL;
static PFN_DXGI_CREATE_FACTORY opal_dxgiCreateFactory1 = NULL;

static Opal_Result directx12_initialize()
{
	static int once = 0;
	static HMODULE d3d12_module = NULL;
	static HMODULE dxgi_module = NULL;

	if (!once)
	{
		d3d12_module = LoadLibraryA("d3d12.dll");
		if (!d3d12_module)
			return OPAL_DIRECX12_ERROR;

		dxgi_module = LoadLibraryA("dxgi.dll");
		if (!dxgi_module)
			return OPAL_DIRECX12_ERROR;

		opal_d3d12CreateDevice = (PFN_D3D12_CREATE_DEVICE)(void(*)(void))GetProcAddress(d3d12_module, "D3D12CreateDevice");
		opal_dxgiCreateFactory1 = (PFN_DXGI_CREATE_FACTORY)(void(*)(void))GetProcAddress(dxgi_module, "CreateDXGIFactory");

		once = 1;
	}

	return OPAL_SUCCESS;
}

/*
 */
static Opal_Result directx12_instanceEnumerateDevices(Opal_Instance this, uint32_t *device_count, Opal_DeviceInfo *infos)
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
			ID3D12Device *d3d_device = NULL;
			HRESULT hr = opal_d3d12CreateDevice((IUnknown *)adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
			if (!SUCCEEDED(hr))
			{
				IDXGIAdapter1_Release(d3d_device);
				return OPAL_DIRECX12_ERROR;
			}

			Opal_Result result = directx12_fillDeviceInfo(adapter, d3d_device, &infos[count]);

			ID3D12Device_Release(d3d_device);

			if (result != OPAL_SUCCESS)
			{
				IDXGIAdapter1_Release(d3d_device);
				return result;
			}
		}

		IDXGIAdapter1_Release(adapter);
		count++;
	}

	*device_count = count;
	return OPAL_SUCCESS;
}

static Opal_Result directx12_instanceCreateSurface(Opal_Instance this, void *handle, Opal_Surface *surface)
{
	assert(this);
	assert(handle);
	assert(surface);

	OPAL_UNUSED(this);
	OPAL_UNUSED(handle);
	OPAL_UNUSED(surface);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_instanceCreateDefaultDevice(Opal_Instance this, Opal_DeviceHint hint, Opal_Device *device)
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
			HRESULT hr = opal_d3d12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
			if (!SUCCEEDED(hr))
			{
				IDXGIAdapter1_Release(d3d_adapter);
				return OPAL_DIRECX12_ERROR;
			}

			Opal_Result result = directx12_fillDeviceInfo(d3d_adapter, d3d_device, &info);
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

	hr = opal_d3d12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
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

static Opal_Result directx12_instanceCreateDevice(Opal_Instance this, uint32_t index, Opal_Device *device)
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
	hr = opal_d3d12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
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

static Opal_Result directx12_instanceDestroySurface(Opal_Instance this, Opal_Surface surface)
{
	assert(this);
	assert(surface);

	OPAL_UNUSED(this);
	OPAL_UNUSED(surface);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result directx12_instanceDestroy(Opal_Instance this)
{
	assert(this);

	DirectX12_Instance *ptr = (DirectX12_Instance *)this;
	IDXGIFactory1_Release(ptr->factory);

	free(ptr);
	return OPAL_SUCCESS;
}

/*
 */
static Opal_InstanceTable instance_vtbl =
{
	directx12_instanceEnumerateDevices,

	directx12_instanceCreateSurface,
	directx12_instanceCreateDevice,
	directx12_instanceCreateDefaultDevice,

	directx12_instanceDestroySurface,
	directx12_instanceDestroy,
};

/*
 */
Opal_Result directx12_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	assert(desc);
	assert(instance);

	OPAL_UNUSED(desc);

	Opal_Result opal_result = directx12_initialize();
	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	IDXGIFactory1 *factory = NULL;

	// factory
	HRESULT hr = opal_dxgiCreateFactory1(&IID_IDXGIFactory1, &factory);
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
