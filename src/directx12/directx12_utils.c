#include "directx12_internal.h"

/*
 */
Opal_Result directx12_fillDeviceInfo(IDXGIAdapter1 *adapter, Opal_DeviceInfo *info)
{
	assert(adapter);
	assert(info);

	ID3D12Device *device = NULL;
	HRESULT hr = D3D12CreateDevice((IUnknown *)adapter, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &device);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECX12_ERROR;

	Opal_Result result = directx12_fillDeviceInfoWithDevice(adapter, device, info);
	ID3D12Device_Release(device);

	return result;
}

Opal_Result directx12_fillDeviceInfoWithDevice(IDXGIAdapter1 *adapter, ID3D12Device *device, Opal_DeviceInfo *info)
{
	assert(adapter);
	assert(info);
	assert(device);

	LARGE_INTEGER umd = {0};
	HRESULT hr = IDXGIAdapter1_CheckInterfaceSupport(adapter, &IID_IDXGIDevice, &umd);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECX12_ERROR;

	DXGI_ADAPTER_DESC1 desc = {0};
	hr = IDXGIAdapter1_GetDesc1(adapter, &desc);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECX12_ERROR;

	memset(info, 0, sizeof(Opal_DeviceInfo));

	WideCharToMultiByte(CP_UTF8, 0, &desc.Description[0], 128, info->name, 256, NULL, NULL);
	info->device_type = OPAL_DEVICE_TYPE_DISCRETE;
	info->driver_version = umd.QuadPart;
	info->vendor_id = desc.VendorId;
	info->device_id = desc.DeviceId;
	info->tessellation_shader = 1;
	info->geometry_shader = 1;
	info->compute_pipeline = 1;
	info->texture_compression_bc = 1;
	info->max_buffer_alignment = 0xFFFF;

	if (desc.Flags == DXGI_ADAPTER_FLAG_SOFTWARE)
		info->device_type = OPAL_DEVICE_TYPE_CPU;

	if (info->device_type == OPAL_DEVICE_TYPE_DISCRETE)
	{
		D3D12_FEATURE_DATA_ARCHITECTURE architecture = {0};
		hr = ID3D12Device_CheckFeatureSupport(device, D3D12_FEATURE_ARCHITECTURE, &architecture, sizeof(D3D12_FEATURE_DATA_ARCHITECTURE));
		if (SUCCEEDED(hr))
			if (architecture.UMA == TRUE)
				info->device_type = OPAL_DEVICE_TYPE_INTEGRATED;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS5 raytracing_options = {0};
	hr = ID3D12Device_CheckFeatureSupport(device, D3D12_FEATURE_D3D12_OPTIONS5, &raytracing_options, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
	if (SUCCEEDED(hr))
		info->raytrace_pipeline = (raytracing_options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED);

	D3D12_FEATURE_DATA_D3D12_OPTIONS9 mesh_options = {0};
	hr = ID3D12Device_CheckFeatureSupport(device, D3D12_FEATURE_D3D12_OPTIONS9, &mesh_options, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS9));
	if (SUCCEEDED(hr))
		info->meshlet_pipeline = (mesh_options.MeshShaderPipelineStatsSupported == TRUE);

	return OPAL_SUCCESS;
}
