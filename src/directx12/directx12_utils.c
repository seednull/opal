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

	// fill basic info
	info->device_type = OPAL_DEVICE_TYPE_DISCRETE;
	info->driver_version = umd.QuadPart;
	info->vendor_id = desc.VendorId;
	info->device_id = desc.DeviceId;

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

	// fill features
	info->features.tessellation_shader = 1;
	info->features.geometry_shader = 1;
	info->features.compute_pipeline = 1;
	info->features.texture_compression_bc = 1;
	info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 16; // NOTE: intentional artificial limit in order to keep the API consistent
	info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_COMPUTE] = 8; // NOTE: intentional artificial limit in order to keep the API consistent
	info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_COPY] = 2; // NOTE: intentional artificial limit in order to keep the API consistent

	D3D12_FEATURE_DATA_D3D12_OPTIONS5 raytracing_options = {0};
	hr = ID3D12Device_CheckFeatureSupport(device, D3D12_FEATURE_D3D12_OPTIONS5, &raytracing_options, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
	if (SUCCEEDED(hr))
		info->features.raytrace_pipeline = (raytracing_options.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED);

	D3D12_FEATURE_DATA_D3D12_OPTIONS9 meshlet_options = {0};
	hr = ID3D12Device_CheckFeatureSupport(device, D3D12_FEATURE_D3D12_OPTIONS9, &meshlet_options, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS9));
	if (SUCCEEDED(hr))
		info->features.meshlet_pipeline = (meshlet_options.MeshShaderPipelineStatsSupported == TRUE);

	// fill limits
	info->limits.maxTextureDimension1D = 16384;
	info->limits.maxTextureDimension2D = 16384;
	info->limits.maxTextureDimension3D = 2048;
	info->limits.maxTextureArrayLayers = 2048;
	// info->limits.maxBufferSize = ;
	info->limits.minUniformBufferOffsetAlignment = 0xFFFF;
	info->limits.minStorageBufferOffsetAlignment = 0xFFFF;
	info->limits.maxBindsets = 32;
	info->limits.maxUniformBufferBindingSize = 0xFFFFFFFF;
	info->limits.maxStorageBufferBindingSize = 0xFFFFFFFF;
	info->limits.maxVertexBuffers = 32;
	info->limits.maxVertexAttributes = 64;
	info->limits.maxVertexBufferStride = 0x00003FFF;
	info->limits.maxColorAttachments = 8;
	// info->limits.maxComputeSharedMemorySize = 0xFFFF;
	info->limits.maxComputeWorkgroupCountX = 65535;
	info->limits.maxComputeWorkgroupCountY = 65535;
	info->limits.maxComputeWorkgroupCountZ = 65535;
	info->limits.maxComputeWorkgroupInvocations = 1024;
	info->limits.maxComputeWorkgroupLocalSizeX = 1024;
	info->limits.maxComputeWorkgroupLocalSizeY = 1024;
	info->limits.maxComputeWorkgroupLocalSizeZ = 64;

	return OPAL_SUCCESS;
}
