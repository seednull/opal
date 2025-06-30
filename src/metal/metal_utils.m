#include "metal_internal.h"

static const uint32_t apple_vendor_id = 0x106b;
static const uint32_t display_vga_class = 0x30000;

/*
 */
MTLTextureType metal_helperToTextureType(Opal_TextureType type, Opal_Samples samples)
{
	if (type == OPAL_TEXTURE_TYPE_2D && samples != OPAL_SAMPLES_1)
		return MTLTextureType2DMultisampleArray;

	static MTLTextureType metal_types[] =
	{
		MTLTextureType1D,
		MTLTextureType2DArray, // for texture view compatibility
		MTLTextureType3D,
	};

	return metal_types[type];
}

MTLTextureType metal_helperToTextureViewType(Opal_TextureViewType type)
{
	static MTLTextureType metal_types[] =
	{
		MTLTextureType1D,
		MTLTextureType2D,
		MTLTextureType2DArray,
		MTLTextureTypeCube,
		MTLTextureTypeCubeArray,
		MTLTextureType3D,
	};

	return metal_types[type];
}

MTLPixelFormat metal_helperToPixelFormat(Opal_TextureFormat format)
{
	static MTLPixelFormat metal_formats[] =
	{
		MTLPixelFormatInvalid,

		MTLPixelFormatR8Unorm,
		MTLPixelFormatR8Snorm,
		MTLPixelFormatR8Uint,
		MTLPixelFormatR8Sint,

		MTLPixelFormatRG8Unorm,
		MTLPixelFormatRG8Snorm,
		MTLPixelFormatRG8Uint,
		MTLPixelFormatRG8Sint,

		MTLPixelFormatRGBA8Unorm,
		MTLPixelFormatRGBA8Snorm,
		MTLPixelFormatRGBA8Uint,
		MTLPixelFormatRGBA8Sint,

		MTLPixelFormatR16Uint,
		MTLPixelFormatR16Sint,
		MTLPixelFormatR16Float,

		MTLPixelFormatRG16Uint,
		MTLPixelFormatRG16Sint,
		MTLPixelFormatRG16Float,

		MTLPixelFormatRGBA16Uint,
		MTLPixelFormatRGBA16Sint,
		MTLPixelFormatRGBA16Float,

		MTLPixelFormatR32Uint,
		MTLPixelFormatR32Sint,
		MTLPixelFormatR32Float,

		MTLPixelFormatRG32Uint,
		MTLPixelFormatRG32Sint,
		MTLPixelFormatRG32Float,

		MTLPixelFormatRGBA32Uint,
		MTLPixelFormatRGBA32Sint,
		MTLPixelFormatRGBA32Float,

		MTLPixelFormatBGRA8Unorm,
		MTLPixelFormatBGRA8Unorm_sRGB,
		MTLPixelFormatRGBA8Unorm_sRGB,

		MTLPixelFormatRG11B10Float,
		MTLPixelFormatRGB9E5Float,

		MTLPixelFormatBC1_RGBA,
		MTLPixelFormatBC1_RGBA_sRGB,
		MTLPixelFormatBC1_RGBA,
		MTLPixelFormatBC1_RGBA_sRGB,
		MTLPixelFormatBC2_RGBA,
		MTLPixelFormatBC2_RGBA_sRGB,
		MTLPixelFormatBC3_RGBA,
		MTLPixelFormatBC3_RGBA_sRGB,
		MTLPixelFormatBC4_RUnorm,
		MTLPixelFormatBC4_RSnorm,
		MTLPixelFormatBC5_RGUnorm,
		MTLPixelFormatBC5_RGSnorm,
		MTLPixelFormatBC6H_RGBUfloat,
		MTLPixelFormatBC6H_RGBFloat,
		MTLPixelFormatBC7_RGBAUnorm,
		MTLPixelFormatBC7_RGBAUnorm_sRGB,

		MTLPixelFormatETC2_RGB8,
		MTLPixelFormatETC2_RGB8_sRGB,
		MTLPixelFormatETC2_RGB8A1,
		MTLPixelFormatETC2_RGB8A1_sRGB,
		MTLPixelFormatEAC_RGBA8,
		MTLPixelFormatEAC_RGBA8_sRGB,
		MTLPixelFormatEAC_R11Unorm,
		MTLPixelFormatEAC_R11Snorm,
		MTLPixelFormatEAC_RG11Unorm,
		MTLPixelFormatEAC_RG11Snorm,

		MTLPixelFormatASTC_4x4_LDR,
		MTLPixelFormatASTC_4x4_sRGB,
		MTLPixelFormatASTC_5x4_LDR,
		MTLPixelFormatASTC_5x4_sRGB,
		MTLPixelFormatASTC_5x5_LDR,
		MTLPixelFormatASTC_5x5_sRGB,
		MTLPixelFormatASTC_6x5_LDR,
		MTLPixelFormatASTC_6x5_sRGB,
		MTLPixelFormatASTC_6x6_LDR,
		MTLPixelFormatASTC_6x6_sRGB,
		MTLPixelFormatASTC_8x5_LDR,
		MTLPixelFormatASTC_8x5_sRGB,
		MTLPixelFormatASTC_8x6_LDR,
		MTLPixelFormatASTC_8x6_sRGB,
		MTLPixelFormatASTC_8x8_LDR,
		MTLPixelFormatASTC_8x8_sRGB,
		MTLPixelFormatASTC_10x5_LDR,
		MTLPixelFormatASTC_10x5_sRGB,
		MTLPixelFormatASTC_10x6_LDR,
		MTLPixelFormatASTC_10x6_sRGB,
		MTLPixelFormatASTC_10x8_LDR,
		MTLPixelFormatASTC_10x8_sRGB,
		MTLPixelFormatASTC_10x10_LDR,
		MTLPixelFormatASTC_10x10_sRGB,
		MTLPixelFormatASTC_12x10_LDR,
		MTLPixelFormatASTC_12x10_sRGB,
		MTLPixelFormatASTC_12x12_LDR,
		MTLPixelFormatASTC_12x12_sRGB,

		MTLPixelFormatDepth16Unorm,
		MTLPixelFormatDepth32Float,
		MTLPixelFormatInvalid,
		MTLPixelFormatDepth24Unorm_Stencil8,
		MTLPixelFormatDepth32Float_Stencil8,
	};

	return metal_formats[format];
}

NSUInteger metal_helperToSampleCount(Opal_Samples samples)
{
	static NSUInteger metal_types[] =
	{
		1, 2, 4, 8, 16, 32, 64,
	};

	return metal_types[samples];
}

MTLTextureUsage metal_helperToTextureUsage(Opal_TextureUsageFlags flags)
{
	MTLTextureUsage result = 0;
	
	if (flags & OPAL_TEXTURE_USAGE_SHADER_SAMPLED)
		result |= MTLTextureUsageShaderRead;

	if (flags & OPAL_TEXTURE_USAGE_UNORDERED_ACCESS)
		result |= MTLTextureUsageShaderWrite;

	if (flags & OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT)
		result |= MTLTextureUsageRenderTarget;

	return result;
}

MTLSamplerAddressMode metal_helperToSamplerAddressMode(Opal_SamplerAddressMode mode)
{
	static MTLSamplerAddressMode metal_modes[] =
	{
		MTLSamplerAddressModeRepeat,
		MTLSamplerAddressModeMirrorRepeat,
		MTLSamplerAddressModeClampToEdge,
	};

	return metal_modes[mode];
}

MTLSamplerMinMagFilter metal_helperToSamplerMinMagFilter(Opal_SamplerFilterMode mode)
{
	static MTLSamplerMinMagFilter metal_filters[] =
	{
		MTLSamplerMinMagFilterNearest,
		MTLSamplerMinMagFilterLinear,
	};

	return metal_filters[mode];
}

MTLSamplerMipFilter metal_helperToSamplerMipFilter(Opal_SamplerFilterMode mode)
{
	static MTLSamplerMipFilter metal_filters[] =
	{
		MTLSamplerMipFilterNearest,
		MTLSamplerMipFilterLinear,
	};

	return metal_filters[mode];
}

MTLCompareFunction metal_helperToCompareFunction(Opal_CompareOp op)
{
	static MTLCompareFunction metal_functions[] =
	{
		MTLCompareFunctionNever,
		MTLCompareFunctionLess,
		MTLCompareFunctionEqual,
		MTLCompareFunctionLessEqual,
		MTLCompareFunctionGreater,
		MTLCompareFunctionNotEqual,
		MTLCompareFunctionGreaterEqual,
		MTLCompareFunctionAlways,
	};

	return metal_functions[op];
}

MTLVertexFormat metal_helperToVertexFormat(Opal_VertexFormat format)
{
	static MTLVertexFormat metal_formats[] =
	{
		MTLVertexFormatInvalid,

		MTLVertexFormatUChar2Normalized,
		MTLVertexFormatChar2Normalized,
		MTLVertexFormatUChar2,
		MTLVertexFormatChar2,

		MTLVertexFormatUChar4Normalized,
		MTLVertexFormatChar4Normalized,
		MTLVertexFormatUChar4,
		MTLVertexFormatChar4,

		MTLVertexFormatUShort2Normalized,
		MTLVertexFormatShort2Normalized,
		MTLVertexFormatUShort2,
		MTLVertexFormatShort2,
		MTLVertexFormatHalf2,

		MTLVertexFormatUShort4Normalized,
		MTLVertexFormatShort4Normalized,
		MTLVertexFormatUShort4,
		MTLVertexFormatShort4,
		MTLVertexFormatHalf4,

		MTLVertexFormatUInt,
		MTLVertexFormatInt,
		MTLVertexFormatFloat,

		MTLVertexFormatUInt2,
		MTLVertexFormatInt2,
		MTLVertexFormatFloat2,

		MTLVertexFormatUInt3,
		MTLVertexFormatInt3,
		MTLVertexFormatFloat3,

		MTLVertexFormatUInt4,
		MTLVertexFormatInt4,
		MTLVertexFormatFloat4,

		MTLVertexFormatUInt1010102Normalized,
	};

	return metal_formats[format];
}

MTLVertexStepFunction metal_helperToVertexStepFunction(Opal_VertexInputRate rate)
{
	static MTLVertexStepFunction metal_step_functions[] =
	{
		MTLVertexStepFunctionPerVertex,
		MTLVertexStepFunctionPerInstance,
	};

	return metal_step_functions[rate];
}

MTLBlendFactor metal_helperToBlendFactor(Opal_BlendFactor factor)
{
	static MTLBlendFactor metal_blend_factors[] =
	{
		MTLBlendFactorZero,
		MTLBlendFactorOne,
		MTLBlendFactorSourceColor,
		MTLBlendFactorOneMinusSourceColor,
		MTLBlendFactorDestinationColor,
		MTLBlendFactorOneMinusDestinationColor,
		MTLBlendFactorSourceAlpha,
		MTLBlendFactorOneMinusSourceAlpha,
		MTLBlendFactorDestinationAlpha,
		MTLBlendFactorOneMinusDestinationAlpha,
	};

	return metal_blend_factors[factor];
}

MTLBlendOperation metal_helperToBlendOperation(Opal_BlendOp op)
{
	static MTLBlendOperation metal_blend_operations[] =
	{
		MTLBlendOperationAdd,
		MTLBlendOperationSubtract,
		MTLBlendOperationReverseSubtract,
		MTLBlendOperationMin,
		MTLBlendOperationMax,
	};

	return metal_blend_operations[op];
}

MTLPrimitiveTopologyClass metal_helperToPrimitiveTopologyClass(Opal_PrimitiveType type)
{
	assert(type != OPAL_PRIMITIVE_TYPE_TRIANGLE_PATCH);
	assert(type != OPAL_PRIMITIVE_TYPE_QUAD_PATCH);

	static MTLPrimitiveTopologyClass metal_primitive_topology_classes[] =
	{
		MTLPrimitiveTopologyClassPoint,
		MTLPrimitiveTopologyClassLine,
		MTLPrimitiveTopologyClassLine,
		MTLPrimitiveTopologyClassTriangle,
		MTLPrimitiveTopologyClassTriangle,
		MTLPrimitiveTopologyClassUnspecified,
		MTLPrimitiveTopologyClassUnspecified,
	};

	return metal_primitive_topology_classes[type];
}

MTLPrimitiveType metal_helperToPrimitiveType(Opal_PrimitiveType type)
{
	assert(type != OPAL_PRIMITIVE_TYPE_TRIANGLE_PATCH);
	assert(type != OPAL_PRIMITIVE_TYPE_QUAD_PATCH);

	static MTLPrimitiveType metal_primitive_types[] =
	{
		MTLPrimitiveTypePoint,
		MTLPrimitiveTypeLine,
		MTLPrimitiveTypeLineStrip,
		MTLPrimitiveTypeTriangle,
		MTLPrimitiveTypeTriangleStrip,
		NSUIntegerMax,
		NSUIntegerMax,
	};

	return metal_primitive_types[type];
}

MTLCullMode metal_helperToCullMode(Opal_CullMode mode)
{
	static MTLCullMode metal_cull_modes[] =
	{
		MTLCullModeNone,
		MTLCullModeFront,
		MTLCullModeBack,
	};

	return metal_cull_modes[mode];
}

MTLWinding metal_helperToWinding(Opal_FrontFace face)
{
	static MTLWinding metal_windings[] =
	{
		MTLWindingCounterClockwise,
		MTLWindingClockwise,
	};

	return metal_windings[face];
}

MTLIndexType metal_helperToIndexType(Opal_IndexFormat format)
{
	assert(format != OPAL_INDEX_FORMAT_UNDEFINED);

	static MTLIndexType metal_index_types[] =
	{
		NSUIntegerMax,
		MTLIndexTypeUInt16,
		MTLIndexTypeUInt32,
	};

	return metal_index_types[format];
}

CFStringRef metal_helperToColorspaceName(Opal_ColorSpace space)
{
	switch (space)
	{
		case OPAL_COLOR_SPACE_SRGB: return kCGColorSpaceSRGB;
		default: return nil;
	}
}

/*
 */
static uint32_t metal_getEntryProperty(io_registry_entry_t entry, CFStringRef name)
{
	CFTypeRef property = IORegistryEntrySearchCFProperty(entry, kIOServicePlane, name, kCFAllocatorDefault, kIORegistryIterateRecursively | kIORegistryIterateParents);
	if (!property)
		return 0;

	uint32_t value = 0;
	uint32_t *ptr = (uint32_t *)CFDataGetBytePtr((CFDataRef)property);
	if (ptr)
		value = *ptr;

	CFRelease(property);

	return value;
}

static bool metal_tryFillByRegistryID(uint64_t registry_id, Opal_DeviceInfo *info)
{
	io_registry_entry_t entry = IOServiceGetMatchingService(MACH_PORT_NULL, IORegistryEntryIDMatching(registry_id));
	if (!entry)
		return false;

	io_registry_entry_t parent;
	if (IORegistryEntryGetParentEntry(entry, kIOServicePlane, &parent) != kIOReturnSuccess)
	{
		IOObjectRelease(entry);
		return false;
	}

	info->vendor_id = metal_getEntryProperty(entry, CFSTR("vendor-id"));
	info->device_id = metal_getEntryProperty(entry, CFSTR("device-id"));

	IOObjectRelease(parent);
	IOObjectRelease(entry);

	return true;
}

static bool metal_tryFillBySearchingAll(Opal_DeviceInfo *info)
{
	io_iterator_t entry_iterator;
	if (IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching("IOCIDevice"), &entry_iterator) != kIOReturnSuccess)
		return false;

	while (true)
	{
		io_registry_entry_t entry = IOIteratorNext(entry_iterator);
		if (!entry)
			break;

		uint32_t class = metal_getEntryProperty(entry, CFSTR("class-code"));
		if (class != display_vga_class)
			continue;

		info->vendor_id = metal_getEntryProperty(entry, CFSTR("vendor-id"));
		info->device_id = metal_getEntryProperty(entry, CFSTR("device-id"));
		break;
	}

	IOObjectRelease(entry_iterator);
	return false;
}

uint32_t metal_getAppleDeviceID(MTLGPUFamily family)
{
	if (family == 0)
		return 0;

	NSOperatingSystemVersion os = [[NSProcessInfo processInfo] operatingSystemVersion];
	uint8_t os_major = (uint8_t)os.majorVersion;
	uint8_t os_minor = (uint8_t)os.minorVersion;

	return (os_major << 24) + (os_minor << 16) + (uint16_t)family;
}

MTLGPUFamily metal_getAppleDeviceFamily(id<MTLDevice> metal_device)
{
	assert(metal_device);

	static MTLGPUFamily supported_families[] =
	{
		MTLGPUFamilyMac2,
		MTLGPUFamilyApple1,
		MTLGPUFamilyApple2,
		MTLGPUFamilyApple3,
		MTLGPUFamilyApple4,
		MTLGPUFamilyApple5,
		MTLGPUFamilyApple6,
		MTLGPUFamilyApple7,
		MTLGPUFamilyApple8,
		MTLGPUFamilyApple9,
	};

	static uint32_t num_families = sizeof(supported_families) / sizeof(MTLGPUFamily);

	MTLGPUFamily family = 0;

	for (uint32_t i = 0; i < num_families; ++i)
		if ([metal_device supportsFamily: supported_families[i]])
			family = supported_families[i];

	return family;
}

/*
 */
Opal_Result metal_helperFillDeviceEnginesInfo(Metal_DeviceEnginesInfo *info)
{
	assert(info);

	info->queue_counts[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 16; // NOTE: intentional artificial limit in order to keep the API consistent
	info->queue_counts[OPAL_DEVICE_ENGINE_TYPE_COMPUTE] = 8; // NOTE: intentional artificial limit in order to keep the API consistent
	info->queue_counts[OPAL_DEVICE_ENGINE_TYPE_COPY] = 2; // NOTE: intentional artificial limit in order to keep the API consistent

	return OPAL_SUCCESS;
}

Opal_Result metal_helperFillDeviceInfo(id<MTLDevice> metal_device, Opal_DeviceInfo *info)
{
	assert(metal_device);
	assert(info);

	memset(info, 0, sizeof(Opal_DeviceInfo));

	bool is_common2_or_greater = [metal_device supportsFamily: MTLGPUFamilyCommon2];
	bool is_metal3 = [metal_device supportsFamily: MTLGPUFamilyMetal3];

	MTLGPUFamily family = metal_getAppleDeviceFamily(metal_device);

	bool is_apple1_or_greater = family >= MTLGPUFamilyApple1;
	bool is_apple2_or_greater = family >= MTLGPUFamilyApple2;
	bool is_apple3_or_greater = family >= MTLGPUFamilyApple3;
	bool is_apple7_or_greater = family >= MTLGPUFamilyApple7;
	bool is_mac2 = family == MTLGPUFamilyMac2;

	strncpy(info->name, metal_device.name.UTF8String, 256);

	info->api = OPAL_API_METAL;
	info->driver_version = 0; // FIXME: fill with hardcoded Opal version

	if (family)
	{
		info->vendor_id = apple_vendor_id;
		info->device_id = metal_getAppleDeviceID(family);
	}
	else
	{
		if (!metal_tryFillByRegistryID(metal_device.registryID, info))
			metal_tryFillBySearchingAll(info);
	}

	info->device_type = OPAL_DEVICE_TYPE_DISCRETE;

	if (metal_device.isLowPower)
		info->device_type = OPAL_DEVICE_TYPE_INTEGRATED;

	Metal_DeviceEnginesInfo device_engines_info = {0};
	metal_helperFillDeviceEnginesInfo(&device_engines_info);

	memcpy(info->features.queue_count, &device_engines_info.queue_counts, sizeof(uint32_t) * OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	// TODO: enable tessellation shader back once it's implemented in the backend
	// info->features.tessellation_shader = is_apple3_or_greater || is_mac2 || is_common2_or_greater;
	info->features.compute_pipeline = 1;
	info->features.meshlet_pipeline = is_apple7_or_greater || is_mac2 || is_metal3;
	info->features.raytrace_pipeline = metal_device.supportsRaytracing;
	info->features.texture_compression_etc2 = is_apple2_or_greater && !is_mac2;
	info->features.texture_compression_astc = is_apple2_or_greater && !is_mac2;
	info->features.texture_compression_bc = metal_device.supportsBCTextureCompression;

	// TODO: fill limits properly
	info->limits.max_texture_dimension_1d = 0;
	info->limits.max_texture_dimension_1d = 0;
	info->limits.max_texture_dimension_2d = 0;
	info->limits.max_texture_dimension_3d = 0;
	info->limits.max_texture_array_layers = 0;
	info->limits.max_buffer_size = 0;
	info->limits.min_uniform_buffer_offset_alignment = 0;
	info->limits.min_storage_buffer_offset_alignment = 0;
	info->limits.max_descriptor_sets = 0;
	info->limits.max_uniform_buffer_binding_size = 0;
	info->limits.max_storage_buffer_binding_size = 0;
	info->limits.max_vertex_buffers = 0;
	info->limits.max_vertex_attributes = 0;
	info->limits.max_vertex_buffer_stride = 0;
	info->limits.max_color_attachments = 0;
	info->limits.max_compute_shared_memory_size = 0;
	info->limits.max_compute_workgroup_count_x = 0;
	info->limits.max_compute_workgroup_count_y = 0;
	info->limits.max_compute_workgroup_count_z = 0;
	info->limits.max_compute_workgroup_invocations = 0;
	info->limits.max_compute_workgroup_local_size_x = 0;
	info->limits.max_compute_workgroup_local_size_y = 0;
	info->limits.max_compute_workgroup_local_size_z = 0;
	info->limits.max_raytrace_recursion_depth = 0;
	info->limits.max_raytrace_hit_attribute_size = 0;

	return OPAL_SUCCESS;
}
