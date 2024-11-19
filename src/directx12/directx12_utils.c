#include "directx12_internal.h"

#include <assert.h>
#include <string.h>

/*
 */
D3D12_RESOURCE_STATES directx12_helperToInitialBufferResourceState(Opal_AllocationMemoryType type, Opal_BufferUsageFlags usage)
{
	if (type == OPAL_ALLOCATION_MEMORY_TYPE_STREAM || type == OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD)
		return D3D12_RESOURCE_STATE_GENERIC_READ;

	if (type == OPAL_ALLOCATION_MEMORY_TYPE_READBACK)
		return D3D12_RESOURCE_STATE_COPY_DEST;

	D3D12_RESOURCE_STATES result = 0;

	if (usage & OPAL_BUFFER_USAGE_TRANSFER_SRC)
		result |= D3D12_RESOURCE_STATE_COPY_SOURCE;

	if (usage & OPAL_BUFFER_USAGE_TRANSFER_DST)
		result |= D3D12_RESOURCE_STATE_COPY_DEST;

	if (usage & OPAL_BUFFER_USAGE_VERTEX)
		result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

	if (usage & OPAL_BUFFER_USAGE_INDEX)
		result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;

	if (usage & OPAL_BUFFER_USAGE_UNIFORM)
		result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

	if (usage & OPAL_BUFFER_USAGE_STORAGE)
		result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	if (usage & OPAL_BUFFER_USAGE_INDIRECT)
		result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

	if (usage & OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE)
		result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	if (usage & OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT)
		result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	if (usage & OPAL_BUFFER_USAGE_SHADER_BINDING_TABLE)
		result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	return result;
}

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
	info->limits.max_texture_dimension_1d = 16384;
	info->limits.max_texture_dimension_2d = 16384;
	info->limits.max_texture_dimension_3d = 2048;
	info->limits.max_texture_array_layers = 2048;
	// info->limits.max_buffer_size = ;
	info->limits.min_uniform_buffer_offset_alignment = 0xFFFF;
	info->limits.min_storage_buffer_offset_alignment = 0xFFFF;
	info->limits.max_bindsets = 32;
	info->limits.max_uniform_buffer_binding_size = 0xFFFFFFFF;
	info->limits.max_storage_buffer_binding_size = 0xFFFFFFFF;
	info->limits.max_vertex_buffers = 32;
	info->limits.max_vertex_attributes = 64;
	info->limits.max_vertex_buffer_stride = 0x00003FFF;
	info->limits.max_color_attachments = 8;
	// info->limits.max_compute_shared_memory_size = 0xFFFF;
	info->limits.max_compute_workgroup_count_x = 65535;
	info->limits.max_compute_workgroup_count_y = 65535;
	info->limits.max_compute_workgroup_count_z = 65535;
	info->limits.max_compute_workgroup_invocations = 1024;
	info->limits.max_compute_workgroup_local_size_x = 1024;
	info->limits.max_compute_workgroup_local_size_y = 1024;
	info->limits.max_compute_workgroup_local_size_z = 64;

	return OPAL_SUCCESS;
}
