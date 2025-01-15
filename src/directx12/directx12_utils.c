#include "directx12_internal.h"

#include <assert.h>
#include <string.h>

/*
 */
D3D12_RESOURCE_STATES directx12_helperToResourceState(Opal_ResourceState state)
{
	D3D12_RESOURCE_STATES result = 0;

	if (state & OPAL_RESOURCE_STATE_VERTEX_AND_UNIFORM_BUFFER)
		result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

	if (state & OPAL_RESOURCE_STATE_INDEX_BUFFER)
		result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;

	if (state & OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT)
		result |= D3D12_RESOURCE_STATE_RENDER_TARGET;

	if (state & OPAL_RESOURCE_STATE_UNORDERED_ACCESS)
		result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	if (state & OPAL_RESOURCE_STATE_DEPTH_STENCIL_WRITE)
		result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;

	if (state & OPAL_RESOURCE_STATE_DEPTH_STENCIL_READ)
		result |= D3D12_RESOURCE_STATE_DEPTH_READ;

	if (state & OPAL_RESOURCE_STATE_NON_FRAGMENT_SHADER_RESOURCE)
		result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	if (state & OPAL_RESOURCE_STATE_FRAGMENT_SHADER_RESOURCE)
		result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	if (state & OPAL_RESOURCE_STATE_COPY_DEST)
		result |= D3D12_RESOURCE_STATE_COPY_DEST;

	if (state & OPAL_RESOURCE_STATE_COPY_SOURCE)
		result |= D3D12_RESOURCE_STATE_COPY_SOURCE;

	if (state & OPAL_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
		result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	if (state & OPAL_RESOURCE_STATE_PRESENT)
		result |= D3D12_RESOURCE_STATE_PRESENT;

	return result;
}

D3D12_RESOURCE_STATES directx12_helperToInitialBufferResourceState(Opal_AllocationMemoryType type, Opal_BufferUsageFlags flags)
{
	if (type == OPAL_ALLOCATION_MEMORY_TYPE_STREAM || type == OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD)
		return D3D12_RESOURCE_STATE_GENERIC_READ;

	if (type == OPAL_ALLOCATION_MEMORY_TYPE_READBACK)
		return D3D12_RESOURCE_STATE_COPY_DEST;

	D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
		| D3D12_RESOURCE_STATE_INDEX_BUFFER
		| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
		| D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		| D3D12_RESOURCE_STATE_COPY_SOURCE;

	if (flags & OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE)
		result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	// TODO: remove this check once there's clear API design for resource states & barriers
	// if (usage & OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT)
	// 	result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	// TODO: remove this check once there's clear API design for resource states & barriers
	// if (usage & OPAL_BUFFER_USAGE_SHADER_BINDING_TABLE)
	// 	result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	return result;
}

D3D12_RESOURCE_FLAGS directx12_helperToTextureFlags(Opal_TextureUsageFlags flags, Opal_TextureFormat format)
{
	D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;

	if (flags & OPAL_TEXTURE_USAGE_UNORDERED_ACCESS)
		result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	if (flags & OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT)
	{
		if (format >= OPAL_TEXTURE_FORMAT_COLOR_BEGIN && format <= OPAL_TEXTURE_FORMAT_COLOR_END)
			result |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		if (format >= OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_BEGIN && format <= OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_END)
			result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}

	return result;
}

D3D12_RESOURCE_DIMENSION directx12_helperToTextureDimension(Opal_TextureType type)
{
	static D3D12_RESOURCE_DIMENSION d3d12_types[] =
	{
		D3D12_RESOURCE_DIMENSION_TEXTURE1D,
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		D3D12_RESOURCE_DIMENSION_TEXTURE3D,
	};

	return d3d12_types[type];
}

DirectX12_ResourceType directx12_helperToTextureResourceType(Opal_TextureUsageFlags flags, Opal_Samples samples)
{
	if (flags & OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT)
		return (samples != OPAL_SAMPLES_1) ? DIRECTX12_RESOURCE_TYPE_MSAA_DS_RT_TEXTURE : DIRECTX12_RESOURCE_TYPE_DS_RT_TEXTURE;

	return (samples != OPAL_SAMPLES_1) ? DIRECTX12_RESOURCE_TYPE_MSAA_NON_DS_RT_TEXTURE : DIRECTX12_RESOURCE_TYPE_NON_DS_RT_TEXTURE;
}

D3D12_FILTER directx12_helperToSamplerFilter(Opal_SamplerFilterMode min, Opal_SamplerFilterMode mag, Opal_SamplerFilterMode mip)
{
	static D3D12_FILTER d3d12_filters[] =
	{
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT,
		D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
	};

	return d3d12_filters[min + mag * 2 + mip * 4];
}
D3D12_TEXTURE_ADDRESS_MODE directx12_helperToSamplerAddressMode(Opal_SamplerAddressMode mode)
{
	static D3D12_TEXTURE_ADDRESS_MODE d3d12_address_modes[] =
	{
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
	};

	return d3d12_address_modes[mode];
}

D3D12_DESCRIPTOR_HEAP_TYPE directx12_helperToDescriptorHeapType(Opal_DescriptorHeapType type)
{
	static D3D12_DESCRIPTOR_HEAP_TYPE d3d12_types[] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	};

	return d3d12_types[type];
}

D3D12_STENCIL_OP directx12_helperToStencilOp(Opal_StencilOp op)
{
	static D3D12_STENCIL_OP d3d12_ops[] =
	{
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_ZERO,
		D3D12_STENCIL_OP_REPLACE,
		D3D12_STENCIL_OP_INVERT,
		D3D12_STENCIL_OP_INCR_SAT,
		D3D12_STENCIL_OP_DECR_SAT,
		D3D12_STENCIL_OP_INCR,
		D3D12_STENCIL_OP_DECR,
	};

	return d3d12_ops[op];
}

D3D12_COMPARISON_FUNC directx12_helperToComparisonFunc(Opal_CompareOp op)
{
	static D3D12_COMPARISON_FUNC d3d12_ops[] =
	{
		D3D12_COMPARISON_FUNC_NEVER,
		D3D12_COMPARISON_FUNC_LESS,
		D3D12_COMPARISON_FUNC_EQUAL,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_COMPARISON_FUNC_GREATER,
		D3D12_COMPARISON_FUNC_NOT_EQUAL,
		D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		D3D12_COMPARISON_FUNC_ALWAYS,
	};

	return d3d12_ops[op];
}

D3D12_BLEND_OP directx12_helperToBlendOp(Opal_BlendOp op)
{
	static D3D12_BLEND_OP d3d12_ops[] =
	{
		D3D12_BLEND_OP_ADD,
		D3D12_BLEND_OP_SUBTRACT,
		D3D12_BLEND_OP_REV_SUBTRACT,
		D3D12_BLEND_OP_MIN,
		D3D12_BLEND_OP_MAX,
	};

	return d3d12_ops[op];
}

D3D12_BLEND directx12_helperToBlendFactor(Opal_BlendFactor factor)
{
	static D3D12_BLEND d3d12_factors[] =
	{
		D3D12_BLEND_ZERO,
		D3D12_BLEND_ONE,
		D3D12_BLEND_SRC_COLOR,
		D3D12_BLEND_INV_SRC_COLOR,
		D3D12_BLEND_DEST_COLOR,
		D3D12_BLEND_INV_DEST_COLOR,
		D3D12_BLEND_SRC_ALPHA,
		D3D12_BLEND_INV_SRC_ALPHA,
		D3D12_BLEND_DEST_ALPHA,
		D3D12_BLEND_INV_DEST_ALPHA,
	};

	return d3d12_factors[factor];
}

D3D12_CULL_MODE directx12_helperToCullMode(Opal_CullMode mode)
{
	static D3D12_CULL_MODE d3d12_modes[] =
	{
		D3D12_CULL_MODE_NONE,
		D3D12_CULL_MODE_FRONT,
		D3D12_CULL_MODE_BACK,
	};

	return d3d12_modes[mode];
}

UINT directx12_helperToSampleCount(Opal_Samples samples)
{
	static UINT d3d12_samples[] =
	{
		1,
		2,
		4,
		8,
		16,
		32,
		64,
	};

	return d3d12_samples[samples];
}

UINT directx12_helperToInstanceDataStepRate(Opal_VertexInputRate rate)
{
	static UINT d3d12_rates[] =
	{
		0,
		1,
	};

	return d3d12_rates[rate];
}

D3D12_INPUT_CLASSIFICATION directx12_helperToInputSlotClass(Opal_VertexInputRate rate)
{
	static D3D12_INPUT_CLASSIFICATION d3d12_classes[] =
	{
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA,
	};

	return d3d12_classes[rate];
}

D3D12_INDEX_BUFFER_STRIP_CUT_VALUE directx12_helperToStripCutValue(Opal_IndexFormat format)
{
	static D3D12_INDEX_BUFFER_STRIP_CUT_VALUE d3d12_values[] =
	{
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF,
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF,
	};

	return d3d12_values[format];
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE directx12_helperToPrimitiveTopologyType(Opal_PrimitiveType type)
{
	static D3D12_PRIMITIVE_TOPOLOGY_TYPE d3d12_types[] =
	{
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,
	};

	return d3d12_types[type];
}

D3D12_PRIMITIVE_TOPOLOGY directx12_helperToPrimitiveTopology(Opal_PrimitiveType type)
{
	static D3D12_PRIMITIVE_TOPOLOGY d3d12_types[] =
	{
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
		D3D_PRIMITIVE_TOPOLOGY_LINELIST,
		D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST,
		D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,
	};

	return d3d12_types[type];
}

D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE directx12_helperToBeginningAccessType(Opal_LoadOp op)
{
	static D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE d3d12_ops[] =
	{
		D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD,
		D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR,
		D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE,
	};

	return d3d12_ops[op];
}

D3D12_RENDER_PASS_ENDING_ACCESS_TYPE directx12_helperToEndingAccessType(Opal_StoreOp op)
{
	static D3D12_RENDER_PASS_ENDING_ACCESS_TYPE d3d12_ops[] =
	{
		D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD,
		D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE,
	};

	return d3d12_ops[op];
}

DXGI_FORMAT directx12_helperToDXGIVertexFormat(Opal_VertexFormat format)
{
	static DXGI_FORMAT dxgi_formats[] =
	{
		DXGI_FORMAT_UNKNOWN,

		DXGI_FORMAT_R8G8_UNORM,
		DXGI_FORMAT_R8G8_SNORM,
		DXGI_FORMAT_R8G8_UINT,
		DXGI_FORMAT_R8G8_SINT,

		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_SNORM,
		DXGI_FORMAT_R8G8B8A8_UINT,
		DXGI_FORMAT_R8G8B8A8_SINT,

		DXGI_FORMAT_R16G16_UNORM,
		DXGI_FORMAT_R16G16_SNORM,
		DXGI_FORMAT_R16G16_UINT,
		DXGI_FORMAT_R16G16_SINT,
		DXGI_FORMAT_R16G16_FLOAT,

		DXGI_FORMAT_R16G16B16A16_UNORM,
		DXGI_FORMAT_R16G16B16A16_SNORM,
		DXGI_FORMAT_R16G16B16A16_UINT,
		DXGI_FORMAT_R16G16B16A16_SINT,
		DXGI_FORMAT_R16G16B16A16_FLOAT,

		DXGI_FORMAT_R32_UINT,
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R32_FLOAT,

		DXGI_FORMAT_R32G32_UINT,
		DXGI_FORMAT_R32G32_SINT,
		DXGI_FORMAT_R32G32_FLOAT,

		DXGI_FORMAT_R32G32B32_UINT,
		DXGI_FORMAT_R32G32B32_SINT,
		DXGI_FORMAT_R32G32B32_FLOAT,

		DXGI_FORMAT_R32G32B32A32_UINT,
		DXGI_FORMAT_R32G32B32A32_SINT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,

		DXGI_FORMAT_R10G10B10A2_UNORM,
	};

	return dxgi_formats[format];
}

DXGI_FORMAT directx12_helperToDXGITextureFormat(Opal_TextureFormat format)
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

DXGI_FORMAT directx12_helperToDXGIIndexFormat(Opal_IndexFormat format)
{
	static DXGI_FORMAT dxgi_formats[] =
	{
		DXGI_FORMAT_UNKNOWN,

		DXGI_FORMAT_R16_UINT,
		DXGI_FORMAT_R32_UINT,
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
	// info->limits.max_buffer_size = ?;
	info->limits.min_uniform_buffer_offset_alignment = 0xFFFF;
	info->limits.min_storage_buffer_offset_alignment = 0xFFFF;
	info->limits.max_descriptor_sets = 32;
	info->limits.max_uniform_buffer_binding_size = 0xFFFFFFFF;
	info->limits.max_storage_buffer_binding_size = 0xFFFFFFFF;
	info->limits.max_vertex_buffers = 32;
	info->limits.max_vertex_attributes = 64;
	info->limits.max_vertex_buffer_stride = 0x00003FFF;
	info->limits.max_color_attachments = 8;
	// info->limits.max_compute_shared_memory_size = ?;
	info->limits.max_compute_workgroup_count_x = 65535;
	info->limits.max_compute_workgroup_count_y = 65535;
	info->limits.max_compute_workgroup_count_z = 65535;
	info->limits.max_compute_workgroup_invocations = 1024;
	info->limits.max_compute_workgroup_local_size_x = 1024;
	info->limits.max_compute_workgroup_local_size_y = 1024;
	info->limits.max_compute_workgroup_local_size_z = 64;

	return OPAL_SUCCESS;
}
