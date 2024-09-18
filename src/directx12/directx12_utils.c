#include "directx12_internal.h"

#include <assert.h>
#include <string.h>

/*
 */
Opal_Result directx12_fillDeviceInfo(IDXGIAdapter1 *adapter, ID3D12Device *device, Opal_DeviceInfo *info)
{
	assert(adapter);
	assert(device);
	assert(info);

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
	info->queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 16; // NOTE: intentional artificial limit in order to keep the API consistent
	info->queue_count[OPAL_DEVICE_ENGINE_TYPE_COMPUTE] = 8; // NOTE: intentional artificial limit in order to keep the API consistent
	info->queue_count[OPAL_DEVICE_ENGINE_TYPE_COPY] = 2; // NOTE: intentional artificial limit in order to keep the API consistent

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

	D3D12_FEATURE_DATA_D3D12_OPTIONS9 meshlet_options = {0};
	hr = ID3D12Device_CheckFeatureSupport(device, D3D12_FEATURE_D3D12_OPTIONS9, &meshlet_options, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS9));
	if (SUCCEEDED(hr))
		info->meshlet_pipeline = (meshlet_options.MeshShaderPipelineStatsSupported == TRUE);

	return OPAL_SUCCESS;
}

Opal_Result directx12_fillDeviceLimits(IDXGIAdapter1 *adapter, ID3D12Device *device, Opal_DeviceLimits *limits)
{
	assert(adapter);
	assert(device);
	assert(limits);

	memset(limits, 0, sizeof(Opal_DeviceLimits));

	limits->maxTextureDimension1D = 16384;
	limits->maxTextureDimension2D = 16384;
	limits->maxTextureDimension3D = 2048;
	limits->maxTextureArrayLayers = 2048;
	// limits->maxBufferSize = ;
	limits->minUniformBufferOffsetAlignment = 0xFFFF;
	limits->minStorageBufferOffsetAlignment = 0xFFFF;
	limits->maxBindsets = 32;
	limits->maxUniformBufferBindingSize = 0xFFFFFFFF;
	limits->maxStorageBufferBindingSize = 0xFFFFFFFF;
	limits->maxVertexBuffers = 32;
	limits->maxVertexAttributes = 64;
	limits->maxVertexBufferStride = 0x00003FFF;
	limits->maxColorAttachments = 8;
	// limits->maxComputeSharedMemorySize = 0xFFFF;
	limits->maxComputeWorkgroupCountX = 65535;
	limits->maxComputeWorkgroupCountY = 65535;
	limits->maxComputeWorkgroupCountZ = 65535;
	limits->maxComputeWorkgroupInvocations = 1024;
	limits->maxComputeWorkgroupLocalSizeX = 1024;
	limits->maxComputeWorkgroupLocalSizeY = 1024;
	limits->maxComputeWorkgroupLocalSizeZ = 64;

	return OPAL_SUCCESS;
}
