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

	// TODO: remove this check once there's clear API design for resource states & barriers
	// if (usage & OPAL_BUFFER_USAGE_TRANSFER_DST)
		// result |= D3D12_RESOURCE_STATE_COPY_DEST;

	if (usage & OPAL_BUFFER_USAGE_VERTEX)
		result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

	if (usage & OPAL_BUFFER_USAGE_INDEX)
		result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;

	if (usage & OPAL_BUFFER_USAGE_UNIFORM)
		result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

	// TODO: remove this check once there's clear API design for resource states & barriers
	// if (usage & OPAL_BUFFER_USAGE_STORAGE)
		// result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	if (usage & OPAL_BUFFER_USAGE_INDIRECT)
		result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

	if (usage & OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE)
		result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	// TODO: remove this check once there's clear API design for resource states & barriers
	// if (usage & OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT)
	// 	result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	// TODO: remove this check once there's clear API design for resource states & barriers
	// if (usage & OPAL_BUFFER_USAGE_SHADER_BINDING_TABLE)
	// 	result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	return result;
}

DXGI_FORMAT directx12_helperToDXGIFormat(Opal_TextureFormat format)
{
	static DXGI_FORMAT dxgi_formats[] =
	{
		DXGI_FORMAT_UNKNOWN,

		DXGI_FORMAT_R8_UNORM,
		DXGI_FORMAT_R8_SNORM,
		DXGI_FORMAT_R8_UINT,
		DXGI_FORMAT_R8_SINT,

		DXGI_FORMAT_R8G8_UNORM,
		DXGI_FORMAT_R8G8_SNORM,
		DXGI_FORMAT_R8G8_UINT,
		DXGI_FORMAT_R8G8_SINT,

		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_SNORM,
		DXGI_FORMAT_R8G8B8A8_UINT,
		DXGI_FORMAT_R8G8B8A8_SINT,

		DXGI_FORMAT_R16_UINT,
		DXGI_FORMAT_R16_SINT,
		DXGI_FORMAT_R16_FLOAT,

		DXGI_FORMAT_R16G16_UINT,
		DXGI_FORMAT_R16G16_SINT,
		DXGI_FORMAT_R16G16_FLOAT,

		DXGI_FORMAT_R16G16B16A16_UINT,
		DXGI_FORMAT_R16G16B16A16_SINT,
		DXGI_FORMAT_R16G16B16A16_FLOAT,

		DXGI_FORMAT_R32_UINT,
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R32_FLOAT,

		DXGI_FORMAT_R32G32_UINT,
		DXGI_FORMAT_R32G32_SINT,
		DXGI_FORMAT_R32G32_FLOAT,

		DXGI_FORMAT_R32G32B32A32_UINT,
		DXGI_FORMAT_R32G32B32A32_SINT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,

		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,

		DXGI_FORMAT_R11G11B10_FLOAT,
		DXGI_FORMAT_R9G9B9E5_SHAREDEXP,

		DXGI_FORMAT_BC1_UNORM,
		DXGI_FORMAT_BC1_UNORM_SRGB,

		DXGI_FORMAT_BC1_UNORM,
		DXGI_FORMAT_BC1_UNORM_SRGB,
		DXGI_FORMAT_BC1_UNORM,
		DXGI_FORMAT_BC1_UNORM_SRGB,
		DXGI_FORMAT_BC2_UNORM,
		DXGI_FORMAT_BC2_UNORM_SRGB,
		DXGI_FORMAT_BC3_UNORM,
		DXGI_FORMAT_BC3_UNORM_SRGB,
		DXGI_FORMAT_BC4_UNORM,
		DXGI_FORMAT_BC4_SNORM,
		DXGI_FORMAT_BC5_UNORM,
		DXGI_FORMAT_BC5_SNORM,
		DXGI_FORMAT_BC6H_UF16,
		DXGI_FORMAT_BC6H_SF16,
		DXGI_FORMAT_BC7_UNORM,
		DXGI_FORMAT_BC7_UNORM_SRGB,

		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,

		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,

		DXGI_FORMAT_D16_UNORM,
		DXGI_FORMAT_D32_FLOAT,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
	};

	return dxgi_formats[format];
}

DXGI_USAGE directx12_helperToDXGIUsage(Opal_TextureUsageFlags usage)
{
	DXGI_USAGE result = DXGI_CPU_ACCESS_NONE;

	if (usage & OPAL_TEXTURE_USAGE_SHADER_SAMPLED)
		result |= DXGI_USAGE_SHADER_INPUT;
	if (usage & OPAL_TEXTURE_USAGE_UNORDERED_ACCESS)
		result |= DXGI_USAGE_UNORDERED_ACCESS;
	if (usage & OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT)
		result |= DXGI_USAGE_RENDER_TARGET_OUTPUT;

	return result;
}

DXGI_COLOR_SPACE_TYPE directx12_helperToDXGIColorSpace(Opal_ColorSpace color_space)
{
	static DXGI_COLOR_SPACE_TYPE dxgi_color_spaces[] =
	{
		DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709,
	};

	return dxgi_color_spaces[color_space];
}

Opal_Result directx12_helperFillDeviceEnginesInfo(DirectX12_DeviceEnginesInfo *info)
{
	assert(info);

	info->queue_counts[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 16; // NOTE: intentional artificial limit in order to keep the API consistent
	info->queue_counts[OPAL_DEVICE_ENGINE_TYPE_COMPUTE] = 8; // NOTE: intentional artificial limit in order to keep the API consistent
	info->queue_counts[OPAL_DEVICE_ENGINE_TYPE_COPY] = 2; // NOTE: intentional artificial limit in order to keep the API consistent

	return OPAL_SUCCESS;
}

/*
 */
Opal_Result directx12_helperFillDeviceInfo(IDXGIAdapter1 *adapter, ID3D12Device *device, Opal_DeviceInfo *info)
{
	assert(adapter);
	assert(device);
	assert(info);

	LARGE_INTEGER umd = {0};
	HRESULT hr = IDXGIAdapter1_CheckInterfaceSupport(adapter, &IID_IDXGIDevice, &umd);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

	DXGI_ADAPTER_DESC1 desc = {0};
	hr = IDXGIAdapter1_GetDesc1(adapter, &desc);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECTX12_ERROR;

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
	DirectX12_DeviceEnginesInfo device_engines_info = {0};
	directx12_helperFillDeviceEnginesInfo(&device_engines_info);

	memcpy(info->features.queue_count, &device_engines_info.queue_counts, sizeof(uint32_t) * OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	info->features.tessellation_shader = 1;
	info->features.geometry_shader = 1;
	info->features.compute_pipeline = 1;
	info->features.texture_compression_bc = 1;

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
