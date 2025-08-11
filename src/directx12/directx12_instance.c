#include "directx12_internal.h"

#include <assert.h>
#include <stdlib.h>

/*
 */
PFN_D3D12_CREATE_DEVICE opal_d3d12CreateDevice = NULL;
PFN_D3D12_SERIALIZE_ROOT_SIGNATURE opal_d3d12SerializeRootSignature = NULL;
PFN_D3D12_GET_DEBUG_INTERFACE opal_d3d12GetDebugInterface = NULL;
PFN_DXGI_CREATE_FACTORY opal_dxgiCreateFactory1 = NULL;

static Opal_Result directx12_initialize()
{
	static int once = 0;
	static HMODULE d3d12_module = NULL;
	static HMODULE dxgi_module = NULL;

	if (!once)
	{
		d3d12_module = LoadLibraryA("d3d12.dll");
		if (!d3d12_module)
			return OPAL_DIRECTX12_ERROR;

		dxgi_module = LoadLibraryA("dxgi.dll");
		if (!dxgi_module)
			return OPAL_DIRECTX12_ERROR;

		opal_dxgiCreateFactory1 = (PFN_DXGI_CREATE_FACTORY)GetProcAddress(dxgi_module, "CreateDXGIFactory1");
		opal_d3d12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(d3d12_module, "D3D12CreateDevice");
		opal_d3d12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(d3d12_module, "D3D12GetDebugInterface");
		opal_d3d12SerializeRootSignature = (PFN_D3D12_SERIALIZE_ROOT_SIGNATURE)GetProcAddress(d3d12_module, "D3D12SerializeRootSignature");

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
	IDXGIFactory2 *factory = ptr->factory;

	UINT count = 0;
	IDXGIAdapter1 *d3d_adapter = NULL;

	while (IDXGIFactory2_EnumAdapters1(factory, count, &d3d_adapter) != DXGI_ERROR_NOT_FOUND)
	{
		if (infos)
		{
			ID3D12Device *d3d_device = NULL;
			HRESULT hr = opal_d3d12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
			if (!SUCCEEDED(hr))
			{
				IDXGIAdapter1_Release(d3d_device);
				return OPAL_DIRECTX12_ERROR;
			}

			Opal_Result result = directx12_helperFillDeviceInfo(d3d_adapter, d3d_device, &infos[count]);

			ID3D12Device_Release(d3d_device);

			if (result != OPAL_SUCCESS)
			{
				IDXGIAdapter1_Release(d3d_adapter);
				return result;
			}
		}

		IDXGIAdapter1_Release(d3d_adapter);
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

	DirectX12_Instance *instance_ptr = (DirectX12_Instance *)this;

	DirectX12_Surface result = {0};
	result.handle = (HWND)handle;

	*surface = (Opal_Surface)opal_poolAddElement(&instance_ptr->surfaces, &result);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_instanceCreateDefaultDevice(Opal_Instance this, Opal_DeviceHint hint, Opal_Device *device)
{
	assert(this);
	assert(device);

	DirectX12_Instance *instance_ptr = (DirectX12_Instance *)this;
	IDXGIFactory2 *factory = instance_ptr->factory;

	IDXGIAdapter1 *d3d_adapter = NULL;
	ID3D12Device *d3d_device = NULL;

	UINT best_index = 0;

	if (hint != OPAL_DEVICE_HINT_DEFAULT)
	{
		UINT count = 0;

		uint32_t best_score = 0;
		uint32_t current_score = 0;

		Opal_DeviceInfo info = {0};

		while (IDXGIFactory2_EnumAdapters1(factory, count, &d3d_adapter) != DXGI_ERROR_NOT_FOUND)
		{
			HRESULT hr = opal_d3d12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
			if (!SUCCEEDED(hr))
			{
				IDXGIAdapter1_Release(d3d_adapter);
				return OPAL_DIRECTX12_ERROR;
			}

			Opal_Result result = directx12_helperFillDeviceInfo(d3d_adapter, d3d_device, &info);
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

	HRESULT hr = IDXGIFactory2_EnumAdapters1(factory, best_index, &d3d_adapter);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	hr = opal_d3d12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
	if (!SUCCEEDED(hr))
	{
		IDXGIAdapter1_Release(d3d_adapter);
		return OPAL_DIRECTX12_ERROR;
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
	IDXGIFactory2 *factory = instance_ptr->factory;

	IDXGIAdapter1 *d3d_adapter = NULL;
	HRESULT hr = IDXGIFactory2_EnumAdapters1(factory, index, &d3d_adapter);
	if (!SUCCEEDED(hr))
		return OPAL_INVALID_DEVICE_INDEX;

	ID3D12Device *d3d_device = NULL;
	hr = opal_d3d12CreateDevice((IUnknown *)d3d_adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &d3d_device);
	if (!SUCCEEDED(hr))
	{
		IDXGIAdapter1_Release(d3d_adapter);
		return OPAL_DIRECTX12_ERROR;
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

	Opal_PoolHandle handle = (Opal_PoolHandle)surface;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	DirectX12_Instance *instance_ptr = (DirectX12_Instance *)this;

	opal_poolRemoveElement(&instance_ptr->surfaces, handle);
	return OPAL_SUCCESS;
}

static Opal_Result directx12_instanceDestroy(Opal_Instance this)
{
	assert(this);

	DirectX12_Instance *ptr = (DirectX12_Instance *)this;

	opal_poolShutdown(&ptr->surfaces);

	if (ptr->debug)
		ID3D12Debug1_Release(ptr->debug);

	IDXGIFactory2_Release(ptr->factory);

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

	IDXGIFactory2 *factory = NULL;

	// factory
	HRESULT hr = opal_dxgiCreateFactory1(&IID_IDXGIFactory2, &factory);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	// debug interface
	ID3D12Debug1 *d3d12_debug1 = NULL;
	if (desc->flags & OPAL_INSTANCE_CREATION_FLAGS_USE_DEBUG_LAYERS)
	{
		ID3D12Debug *d3d12_debug = NULL;
		hr = opal_d3d12GetDebugInterface(&IID_ID3D12Debug, &d3d12_debug);
		if (!SUCCEEDED(hr))
			return OPAL_DIRECTX12_ERROR;

		hr = ID3D12Debug_QueryInterface(d3d12_debug, &IID_ID3D12Debug1, &d3d12_debug1);
		ID3D12Debug_Release(d3d12_debug);

		if (!SUCCEEDED(hr))
			return OPAL_DIRECTX12_ERROR;

		ID3D12Debug1_EnableDebugLayer(d3d12_debug1);
		ID3D12Debug1_SetEnableGPUBasedValidation(d3d12_debug1, TRUE);
	}

	DirectX12_Instance *ptr = (DirectX12_Instance *)malloc(sizeof(DirectX12_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl = &instance_vtbl;

	// data
	ptr->factory = factory;
	ptr->debug = d3d12_debug1;
	ptr->heap_size = desc->heap_size;
	ptr->max_heap_allocations = desc->max_heap_allocations;
	ptr->max_heaps = desc->max_heaps;

	// pools
	opal_poolInitialize(&ptr->surfaces, sizeof(DirectX12_Surface), 32);

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;
}
