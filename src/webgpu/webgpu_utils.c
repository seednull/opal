#include "webgpu_internal.h"

#include <assert.h>
#include <string.h>

/*
 */
WGPUBufferUsageFlags webgpu_helperToBufferUsage(Opal_BufferUsageFlags flags, Opal_AllocationMemoryType memory_type)
{
	assert((flags & OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE) == 0);
	assert((flags & OPAL_BUFFER_USAGE_SHADER_BINDING_TABLE) == 0);

	WGPUBufferUsageFlags result = 0;

	if (flags & OPAL_BUFFER_USAGE_TRANSFER_SRC)
	{
		result |= WGPUBufferUsage_CopySrc;

		if (memory_type != OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL)
			result |= WGPUBufferUsage_MapWrite;
	}

	if (flags & OPAL_BUFFER_USAGE_TRANSFER_DST)
	{
		result |= WGPUBufferUsage_CopyDst;

		if (memory_type != OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL)
			result |= WGPUBufferUsage_MapRead;
	}

	if (flags & OPAL_BUFFER_USAGE_VERTEX)
		result |= WGPUBufferUsage_Vertex;

	if (flags & OPAL_BUFFER_USAGE_INDEX)
		result |= WGPUBufferUsage_Index;

	if (flags & OPAL_BUFFER_USAGE_UNIFORM)
		result |= WGPUBufferUsage_Uniform;

	if (flags & OPAL_BUFFER_USAGE_STORAGE)
		result |= WGPUBufferUsage_Storage;

	if (flags & OPAL_BUFFER_USAGE_INDIRECT)
		result |= WGPUBufferUsage_Indirect;

	return result;
}

WGPUTextureUsageFlags webgpu_helperToTextureUsage(Opal_TextureUsageFlags flags)
{
	WGPUTextureUsageFlags result = 0;

	if (flags & OPAL_TEXTURE_USAGE_TRANSFER_SRC)
		result |= WGPUTextureUsage_CopySrc;

	if (flags & OPAL_TEXTURE_USAGE_TRANSFER_DST)
		result |= WGPUTextureUsage_CopyDst;

	if (flags & OPAL_TEXTURE_USAGE_SHADER_SAMPLED)
		result |= WGPUTextureUsage_TextureBinding;

	if (flags & OPAL_TEXTURE_USAGE_UNORDERED_ACCESS)
		result |= WGPUTextureUsage_StorageBinding;

	if (flags & OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT)
		result |= WGPUTextureUsage_RenderAttachment;

	return result;
}

WGPUTextureDimension webgpu_helperToTextureDimension(Opal_TextureType type)
{
	static WGPUTextureDimension wgpu_texture_dimensions[] =
	{
		WGPUTextureDimension_1D,
		WGPUTextureDimension_2D,
		WGPUTextureDimension_3D,
	};

	return wgpu_texture_dimensions[type];
}

WGPUTextureViewDimension webgpu_helperToTextureViewDimension(Opal_TextureViewType type)
{
	static WGPUTextureViewDimension wgpu_texture_view_dimensions[] =
	{
		WGPUTextureViewDimension_1D,
		WGPUTextureViewDimension_2D,
		WGPUTextureViewDimension_2DArray,
		WGPUTextureViewDimension_Cube,
		WGPUTextureViewDimension_CubeArray,
		WGPUTextureViewDimension_3D,
	};

	return wgpu_texture_view_dimensions[type];
}

WGPUTextureFormat webgpu_helperToTextureFormat(Opal_Format format)
{
	static WGPUTextureFormat wgpu_formats[] =
	{
		WGPUTextureFormat_Undefined,

		// 8-bit formats
		WGPUTextureFormat_R8Unorm, WGPUTextureFormat_R8Snorm, WGPUTextureFormat_R8Uint, WGPUTextureFormat_R8Sint, WGPUTextureFormat_Undefined,
		WGPUTextureFormat_RG8Unorm, WGPUTextureFormat_RG8Snorm, WGPUTextureFormat_RG8Uint, WGPUTextureFormat_RG8Sint, WGPUTextureFormat_Undefined,
		WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined,
		WGPUTextureFormat_RGBA8Unorm, WGPUTextureFormat_RGBA8Snorm, WGPUTextureFormat_RGBA8Uint, WGPUTextureFormat_RGBA8Sint, WGPUTextureFormat_RGBA8UnormSrgb,

		WGPUTextureFormat_BGRA8Unorm, WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_BGRA8UnormSrgb,

		// 16-bit formats
		WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_R16Uint, WGPUTextureFormat_R16Sint, WGPUTextureFormat_R16Float,
		WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_RG16Uint, WGPUTextureFormat_RG16Sint, WGPUTextureFormat_RG16Float,
		WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined,
		WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_RGBA16Uint, WGPUTextureFormat_RGBA16Sint, WGPUTextureFormat_RGBA16Float,

		// 32-bit formats
		WGPUTextureFormat_R32Uint, WGPUTextureFormat_R32Sint, WGPUTextureFormat_R32Float,
		WGPUTextureFormat_RG32Uint, WGPUTextureFormat_RG32Sint, WGPUTextureFormat_RG32Float,
		WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined, WGPUTextureFormat_Undefined,
		WGPUTextureFormat_RGBA32Uint, WGPUTextureFormat_RGBA32Sint, WGPUTextureFormat_RGBA32Float,

		// hdr 32-bit formats
		WGPUTextureFormat_RG11B10Ufloat,
		WGPUTextureFormat_RGB9E5Ufloat,

		// bc formats
		WGPUTextureFormat_Undefined,
		WGPUTextureFormat_BC1RGBAUnorm,
		WGPUTextureFormat_BC1RGBAUnormSrgb,
		WGPUTextureFormat_BC2RGBAUnorm,
		WGPUTextureFormat_BC2RGBAUnormSrgb,
		WGPUTextureFormat_BC3RGBAUnorm,
		WGPUTextureFormat_BC3RGBAUnormSrgb,
		WGPUTextureFormat_BC4RUnorm,
		WGPUTextureFormat_BC4RSnorm,
		WGPUTextureFormat_BC5RGUnorm,
		WGPUTextureFormat_BC5RGSnorm,
		WGPUTextureFormat_BC6HRGBUfloat,
		WGPUTextureFormat_BC6HRGBFloat,
		WGPUTextureFormat_BC7RGBAUnorm,
		WGPUTextureFormat_BC7RGBAUnormSrgb,

		// etc formats
		WGPUTextureFormat_ETC2RGB8Unorm,
		WGPUTextureFormat_ETC2RGB8UnormSrgb,
		WGPUTextureFormat_ETC2RGB8A1Unorm,
		WGPUTextureFormat_ETC2RGB8A1UnormSrgb,
		WGPUTextureFormat_ETC2RGBA8Unorm,
		WGPUTextureFormat_ETC2RGBA8UnormSrgb,
		WGPUTextureFormat_EACR11Unorm,
		WGPUTextureFormat_EACR11Snorm,
		WGPUTextureFormat_EACRG11Unorm,
		WGPUTextureFormat_EACRG11Snorm,

		// astc formats
		WGPUTextureFormat_ASTC4x4Unorm,
		WGPUTextureFormat_ASTC4x4UnormSrgb,
		WGPUTextureFormat_ASTC5x4Unorm,
		WGPUTextureFormat_ASTC5x4UnormSrgb,
		WGPUTextureFormat_ASTC5x5Unorm,
		WGPUTextureFormat_ASTC5x5UnormSrgb,
		WGPUTextureFormat_ASTC6x5Unorm,
		WGPUTextureFormat_ASTC6x5UnormSrgb,
		WGPUTextureFormat_ASTC6x6Unorm,
		WGPUTextureFormat_ASTC6x6UnormSrgb,
		WGPUTextureFormat_ASTC8x5Unorm,
		WGPUTextureFormat_ASTC8x5UnormSrgb,
		WGPUTextureFormat_ASTC8x6Unorm,
		WGPUTextureFormat_ASTC8x6UnormSrgb,
		WGPUTextureFormat_ASTC8x8Unorm,
		WGPUTextureFormat_ASTC8x8UnormSrgb,
		WGPUTextureFormat_ASTC10x5Unorm,
		WGPUTextureFormat_ASTC10x5UnormSrgb,
		WGPUTextureFormat_ASTC10x6Unorm,
		WGPUTextureFormat_ASTC10x6UnormSrgb,
		WGPUTextureFormat_ASTC10x8Unorm,
		WGPUTextureFormat_ASTC10x8UnormSrgb,
		WGPUTextureFormat_ASTC10x10Unorm,
		WGPUTextureFormat_ASTC10x10UnormSrgb,
		WGPUTextureFormat_ASTC12x10Unorm,
		WGPUTextureFormat_ASTC12x10UnormSrgb,
		WGPUTextureFormat_ASTC12x12Unorm,
		WGPUTextureFormat_ASTC12x12UnormSrgb,

		// depth_stencil formats
		WGPUTextureFormat_Depth16Unorm,
		WGPUTextureFormat_Depth32Float,
		WGPUTextureFormat_Undefined,
		WGPUTextureFormat_Depth24PlusStencil8,
		WGPUTextureFormat_Depth32FloatStencil8,
	};

	return wgpu_formats[format];
}

uint32_t webgpu_helperToSampleCount(Opal_Samples samples)
{
	static uint32_t wgpu_samples[] =
	{
		1,
		2,
		4,
		8,
		16,
		32,
		64,
	};

	return wgpu_samples[samples];
}

WGPUTextureAspect webgpu_helperToTextureAspect(Opal_Format format)
{
	if (format >= OPAL_FORMAT_COLOR_BEGIN && format <= OPAL_FORMAT_COLOR_END)
		return WGPUTextureAspect_All;

	if (format >= OPAL_FORMAT_DEPTH_STENCIL_BEGIN && format <= OPAL_FORMAT_DEPTH_STENCIL_END)
	{
		if (format == OPAL_FORMAT_D16_UNORM || format == OPAL_FORMAT_D32_SFLOAT)
			return WGPUTextureAspect_DepthOnly;

		return WGPUTextureAspect_All;
	}

	return WGPUTextureAspect_Undefined;
}

WGPUAddressMode webgpu_helperToAddressMode(Opal_SamplerAddressMode mode)
{
	static WGPUAddressMode wgpu_address_modes[] =
	{
		WGPUAddressMode_Repeat,
		WGPUAddressMode_MirrorRepeat,
		WGPUAddressMode_ClampToEdge,
	};

	return wgpu_address_modes[mode];
}

WGPUFilterMode webgpu_helperToFilterMode(Opal_SamplerFilterMode mode)
{
	static WGPUFilterMode wgpu_filter_modes[] =
	{
		WGPUFilterMode_Nearest,
		WGPUFilterMode_Linear,
	};

	return wgpu_filter_modes[mode];
}

WGPUMipmapFilterMode webgpu_helperToMipmapFilterMode(Opal_SamplerFilterMode mode)
{
	static WGPUMipmapFilterMode wgpu_mipmap_filter_modes[] =
	{
		WGPUMipmapFilterMode_Nearest,
		WGPUMipmapFilterMode_Linear,
	};

	return wgpu_mipmap_filter_modes[mode];
}

WGPUCompareFunction webgpu_helperToCompareFunction(Opal_CompareOp op)
{
	static WGPUCompareFunction wgpu_compare_functions[] =
	{
		WGPUCompareFunction_Never,
		WGPUCompareFunction_Less,
		WGPUCompareFunction_Equal,
		WGPUCompareFunction_LessEqual,
		WGPUCompareFunction_Greater,
		WGPUCompareFunction_NotEqual,
		WGPUCompareFunction_GreaterEqual,
		WGPUCompareFunction_Always,
	};

	return wgpu_compare_functions[op];
}

/*
 */
Opal_Result webgpu_fillDeviceInfo(WGPUAdapter adapter, Opal_DeviceInfo *info)
{
	assert(adapter);
	assert(info);

	WGPUAdapterInfo adapter_info = {0};
	wgpuAdapterGetInfo(adapter, &adapter_info);

	WGPUSupportedLimits adapter_limits = {0};
	wgpuAdapterGetLimits(adapter, &adapter_limits);

	memset(info, 0, sizeof(Opal_DeviceInfo));

	if (adapter_info.description)
		strncpy(info->name, adapter_info.description, 256);

	info->driver_version = 0; // FIXME: is it possible to get uint32 / uint64 driver version for WGPU adapter?
	info->device_id = adapter_info.deviceID;
	info->vendor_id = adapter_info.vendorID;

	switch (adapter_info.adapterType)
	{
		case WGPUAdapterType_DiscreteGPU: info->device_type = OPAL_DEVICE_TYPE_DISCRETE; break;
		case WGPUAdapterType_IntegratedGPU: info->device_type = OPAL_DEVICE_TYPE_INTEGRATED; break;
		case WGPUAdapterType_CPU: info->device_type = OPAL_DEVICE_TYPE_CPU; break;
		default: info->device_type = OPAL_DEVICE_TYPE_UNKNOWN; break;
	}

	info->compute_pipeline = 1;
	info->texture_compression_etc2 = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionETC2) != 0;
	info->texture_compression_astc = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionASTC) != 0;
	info->texture_compression_bc = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionBC) != 0;
	info->max_buffer_alignment = 256; // FIXME: not sure if this is good default
	info->queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 1;

	return OPAL_SUCCESS;
}
