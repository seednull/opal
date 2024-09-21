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
		result |= WGPUBufferUsage_CopySrc;

	if (flags & OPAL_BUFFER_USAGE_TRANSFER_DST)
		result |= WGPUBufferUsage_CopyDst;

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

	if (flags == OPAL_BUFFER_USAGE_TRANSFER_SRC && (memory_type == OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD || memory_type == OPAL_ALLOCATION_MEMORY_TYPE_STREAM))
		result |= WGPUBufferUsage_MapWrite;

	if (flags == OPAL_BUFFER_USAGE_TRANSFER_DST && (memory_type == OPAL_ALLOCATION_MEMORY_TYPE_READBACK))
		result |= WGPUBufferUsage_MapRead;

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

WGPUTextureFormat webgpu_helperToTextureFormat(Opal_TextureFormat format)
{
	static WGPUTextureFormat wgpu_formats[] =
	{
		WGPUTextureFormat_Undefined,

		// 8-bit formats
		WGPUTextureFormat_R8Unorm, WGPUTextureFormat_R8Snorm, WGPUTextureFormat_R8Uint, WGPUTextureFormat_R8Sint,
		WGPUTextureFormat_RG8Unorm, WGPUTextureFormat_RG8Snorm, WGPUTextureFormat_RG8Uint, WGPUTextureFormat_RG8Sint,
		WGPUTextureFormat_RGBA8Unorm, WGPUTextureFormat_RGBA8Snorm, WGPUTextureFormat_RGBA8Uint, WGPUTextureFormat_RGBA8Sint,

		// 16-bit formats
		WGPUTextureFormat_R16Uint, WGPUTextureFormat_R16Sint, WGPUTextureFormat_R16Float,
		WGPUTextureFormat_RG16Uint, WGPUTextureFormat_RG16Sint, WGPUTextureFormat_RG16Float,
		WGPUTextureFormat_RGBA16Uint, WGPUTextureFormat_RGBA16Sint, WGPUTextureFormat_RGBA16Float,

		// 32-bit formats
		WGPUTextureFormat_R32Uint, WGPUTextureFormat_R32Sint, WGPUTextureFormat_R32Float,
		WGPUTextureFormat_RG32Uint, WGPUTextureFormat_RG32Sint, WGPUTextureFormat_RG32Float,
		WGPUTextureFormat_RGBA32Uint, WGPUTextureFormat_RGBA32Sint, WGPUTextureFormat_RGBA32Float,

		// special 4-channel formats
		WGPUTextureFormat_BGRA8Unorm,
		WGPUTextureFormat_BGRA8UnormSrgb,
		WGPUTextureFormat_RGBA8UnormSrgb,

		// hdr 32-bit formats
		WGPUTextureFormat_RG11B10Ufloat,
		WGPUTextureFormat_RGB9E5Ufloat,

		// bc formats
		WGPUTextureFormat_Undefined,
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

WGPUTextureAspect webgpu_helperToTextureAspect(Opal_TextureFormat format)
{
	if (format >= OPAL_TEXTURE_FORMAT_COLOR_BEGIN && format <= OPAL_TEXTURE_FORMAT_COLOR_END)
		return WGPUTextureAspect_All;

	if (format >= OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_BEGIN && format <= OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_END)
	{
		if (format == OPAL_TEXTURE_FORMAT_D16_UNORM || format == OPAL_TEXTURE_FORMAT_D32_SFLOAT)
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

WGPUVertexStepMode webgpu_helperToVertexStepMode(Opal_VertexInputRate rate)
{
	static WGPUVertexStepMode wgpu_vertex_step_modes[] =
	{
		WGPUVertexStepMode_Vertex,
		WGPUVertexStepMode_Instance,
	};

	return wgpu_vertex_step_modes[rate];
}

WGPUVertexFormat webgpu_helperToVertexFormat(Opal_VertexFormat format)
{
	static WGPUVertexFormat wgpu_formats[] =
	{
		WGPUVertexFormat_Undefined,

		// 8-bit formats
		WGPUVertexFormat_Unorm8x2, WGPUVertexFormat_Snorm8x2, WGPUVertexFormat_Uint8x2, WGPUVertexFormat_Sint8x2,
		WGPUVertexFormat_Unorm8x4, WGPUVertexFormat_Snorm8x4, WGPUVertexFormat_Uint8x4, WGPUVertexFormat_Sint8x4,

		// 16-bit formats
		WGPUVertexFormat_Unorm16x2, WGPUVertexFormat_Snorm16x2, WGPUVertexFormat_Uint16x2, WGPUVertexFormat_Sint16x2, WGPUVertexFormat_Float16x2,
		WGPUVertexFormat_Unorm16x4, WGPUVertexFormat_Snorm16x4, WGPUVertexFormat_Uint16x4, WGPUVertexFormat_Sint16x4, WGPUVertexFormat_Float16x4,

		// 32-bit formats
		WGPUVertexFormat_Uint32, WGPUVertexFormat_Sint32, WGPUVertexFormat_Float32,
		WGPUVertexFormat_Uint32x2, WGPUVertexFormat_Sint32x2, WGPUVertexFormat_Float32x2,
		WGPUVertexFormat_Uint32x3, WGPUVertexFormat_Sint32x3, WGPUVertexFormat_Float32x3,
		WGPUVertexFormat_Uint32x4, WGPUVertexFormat_Sint32x4, WGPUVertexFormat_Float32x4,

		// special formats
		WGPUVertexFormat_Unorm10_10_10_2,
	};

	return wgpu_formats[format];
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

WGPUStencilOperation webgpu_helperToStencilOperation(Opal_StencilOp op)
{
	static WGPUStencilOperation wgpu_stencil_operations[] =
	{
		WGPUStencilOperation_Keep,
		WGPUStencilOperation_Zero,
		WGPUStencilOperation_Replace,
		WGPUStencilOperation_Invert,
		WGPUStencilOperation_IncrementClamp,
		WGPUStencilOperation_DecrementClamp,
		WGPUStencilOperation_IncrementWrap,
		WGPUStencilOperation_DecrementWrap,
	};

	return wgpu_stencil_operations[op];
}

WGPUBlendOperation webgpu_helperToBlendOperation(Opal_BlendOp op)
{
	static WGPUBlendOperation wgpu_blend_operations[] =
	{
		WGPUBlendOperation_Add,
		WGPUBlendOperation_Subtract,
		WGPUBlendOperation_ReverseSubtract,
		WGPUBlendOperation_Min,
		WGPUBlendOperation_Max,
	};

	return wgpu_blend_operations[op];
}

WGPUBlendFactor webgpu_helperToBlendFactor(Opal_BlendFactor factor)
{
	static WGPUBlendFactor wgpu_blend_factors[] =
	{
		WGPUBlendFactor_Zero,
		WGPUBlendFactor_One,
		WGPUBlendFactor_Src,
		WGPUBlendFactor_OneMinusSrc,
		WGPUBlendFactor_Dst,
		WGPUBlendFactor_OneMinusDst,
		WGPUBlendFactor_SrcAlpha,
		WGPUBlendFactor_OneMinusSrcAlpha,
		WGPUBlendFactor_DstAlpha,
		WGPUBlendFactor_OneMinusDstAlpha,
	};

	return wgpu_blend_factors[factor];
}

WGPUPrimitiveTopology webgpu_helperToPrimitiveTopology(Opal_PrimitiveType type)
{
	static WGPUPrimitiveTopology wgpu_primitive_topologies[] =
	{
		WGPUPrimitiveTopology_PointList,
		WGPUPrimitiveTopology_LineList,
		WGPUPrimitiveTopology_LineStrip,
		WGPUPrimitiveTopology_TriangleList,
		WGPUPrimitiveTopology_TriangleStrip,
		WGPUPrimitiveTopology_Undefined,
		WGPUPrimitiveTopology_Undefined,
	};

	return wgpu_primitive_topologies[type];
}

WGPUFrontFace webgpu_helperToFrontFace(Opal_FrontFace face)
{
	static WGPUFrontFace wgpu_front_faces[] =
	{
		WGPUFrontFace_CCW,
		WGPUFrontFace_CW,
	};

	return wgpu_front_faces[face];
}

WGPUCullMode webgpu_helperToCullMode(Opal_CullMode mode)
{
	static WGPUCullMode wgpu_cull_modes[] =
	{
		WGPUCullMode_None,
		WGPUCullMode_Front,
		WGPUCullMode_Back,
	};

	return wgpu_cull_modes[mode];
}

WGPUPresentMode webgpu_helperToPresentMode(Opal_PresentMode mode)
{
	static WGPUPresentMode wgpu_present_modes[] =
	{
		WGPUPresentMode_Immediate,
		WGPUPresentMode_Fifo,
		WGPUPresentMode_Mailbox,
	};

	return wgpu_present_modes[mode];
}

WGPUShaderStageFlags webgpu_helperToShaderStage(Opal_ShaderStage stage)
{
	WGPUShaderStageFlags result = 0;
	
	if (stage & OPAL_SHADER_STAGE_VERTEX)
		result |= WGPUShaderStage_Vertex;

	if (stage & OPAL_SHADER_STAGE_FRAGMENT)
		result |= WGPUShaderStage_Fragment;

	if (stage & OPAL_SHADER_STAGE_COMPUTE)
		result |= WGPUShaderStage_Compute;

	return result;
}

WGPUBufferBindingType webgpu_helperToBindingBufferType(Opal_BindingType type)
{
	static WGPUBufferBindingType wgpu_buffer_binding_types[] =
	{
		WGPUBufferBindingType_Uniform,
		WGPUBufferBindingType_Uniform,
		WGPUBufferBindingType_Storage,
		WGPUBufferBindingType_Storage,
		WGPUBufferBindingType_ReadOnlyStorage,
		WGPUBufferBindingType_ReadOnlyStorage,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
		WGPUBufferBindingType_Undefined,
	};

	return wgpu_buffer_binding_types[type];
}

WGPUTextureSampleType webgpu_helperToBindingSampleType(Opal_TextureFormat format)
{
	static WGPUTextureSampleType wgpu_texture_sample_types[] =
	{
		WGPUTextureSampleType_Undefined,

		// 8-bit formats
		WGPUTextureSampleType_Float, WGPUTextureSampleType_Float, WGPUTextureSampleType_Uint, WGPUTextureSampleType_Sint,
		WGPUTextureSampleType_Float, WGPUTextureSampleType_Float, WGPUTextureSampleType_Uint, WGPUTextureSampleType_Sint,
		WGPUTextureSampleType_Float, WGPUTextureSampleType_Float, WGPUTextureSampleType_Uint, WGPUTextureSampleType_Sint,

		// 16-bit formats
		WGPUTextureSampleType_Uint, WGPUTextureSampleType_Sint, WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Uint, WGPUTextureSampleType_Sint, WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Uint, WGPUTextureSampleType_Sint, WGPUTextureSampleType_Float,

		// 32-bit formats
		WGPUTextureSampleType_Uint, WGPUTextureSampleType_Sint, WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Uint, WGPUTextureSampleType_Sint, WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Uint, WGPUTextureSampleType_Sint, WGPUTextureSampleType_Float,

		// special 4-channel formats
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,

		// hdr 32-bit formats
		WGPUTextureSampleType_UnfilterableFloat,
		WGPUTextureSampleType_UnfilterableFloat,

		// bc formats
		WGPUTextureSampleType_Undefined,
		WGPUTextureSampleType_Undefined,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,

		// etc formats
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,

		// astc formats
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,
		WGPUTextureSampleType_Float,

		// depth_stencil formats
		WGPUTextureSampleType_Depth,
		WGPUTextureSampleType_Depth,
		WGPUTextureSampleType_Undefined,
		WGPUTextureSampleType_Depth,
		WGPUTextureSampleType_Depth,
	};

	return wgpu_texture_sample_types[format];
}

WGPUTextureViewDimension webgpu_helperToBindingViewDimension(Opal_BindingType type)
{
	static WGPUTextureViewDimension wgpu_texture_view_dimensions[] =
	{
		WGPUTextureViewDimension_Undefined,
		WGPUTextureViewDimension_Undefined,
		WGPUTextureViewDimension_Undefined,
		WGPUTextureViewDimension_Undefined,
		WGPUTextureViewDimension_Undefined,
		WGPUTextureViewDimension_Undefined,
		WGPUTextureViewDimension_1D,
		WGPUTextureViewDimension_2D,
		WGPUTextureViewDimension_2DArray,
		WGPUTextureViewDimension_Cube,
		WGPUTextureViewDimension_CubeArray,
		WGPUTextureViewDimension_3D,
		WGPUTextureViewDimension_2D,
		WGPUTextureViewDimension_1D,
		WGPUTextureViewDimension_2D,
		WGPUTextureViewDimension_2DArray,
		WGPUTextureViewDimension_3D,
		WGPUTextureViewDimension_Undefined,
		WGPUTextureViewDimension_Undefined,
		WGPUTextureViewDimension_Undefined,
	};

	return wgpu_texture_view_dimensions[type];
}

WGPULoadOp webgpu_helperToLoadOp(Opal_LoadOp op)
{
	static WGPULoadOp wgpu_load_ops[] =
	{
		WGPULoadOp_Clear,
		WGPULoadOp_Clear,
		WGPULoadOp_Load,
	};

	return wgpu_load_ops[op];
}

WGPUStoreOp webgpu_helperToStoreOp(Opal_StoreOp op)
{
	static WGPUStoreOp wgpu_store_ops[] =
	{
		WGPUStoreOp_Discard,
		WGPUStoreOp_Store,
	};

	return wgpu_store_ops[op];
}

WGPUIndexFormat webgpu_helperToIndexFormat(Opal_IndexFormat format)
{
	static WGPUIndexFormat wgpu_index_formats[] =
	{
		WGPUIndexFormat_Undefined,
		WGPUIndexFormat_Uint16,
		WGPUIndexFormat_Uint32,
	};

	return wgpu_index_formats[format];
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

	// fill basic info
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

	// fill features
	info->features.compute_pipeline = 1;
	info->features.texture_compression_etc2 = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionETC2) != 0;
	info->features.texture_compression_astc = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionASTC) != 0;
	info->features.texture_compression_bc = wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionBC) != 0;
	info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 1;

	// fill limits
	info->limits.maxTextureDimension1D = adapter_limits.limits.maxTextureDimension1D;
	info->limits.maxTextureDimension2D = adapter_limits.limits.maxTextureDimension2D;
	info->limits.maxTextureDimension3D = adapter_limits.limits.maxTextureDimension3D;
	info->limits.maxTextureArrayLayers = adapter_limits.limits.maxTextureArrayLayers;

	info->limits.maxBufferSize = adapter_limits.limits.maxBufferSize;
	info->limits.minUniformBufferOffsetAlignment = adapter_limits.limits.minUniformBufferOffsetAlignment;
	info->limits.minStorageBufferOffsetAlignment = adapter_limits.limits.minStorageBufferOffsetAlignment;

	info->limits.maxBindsets = adapter_limits.limits.maxBindGroups;
	info->limits.maxUniformBufferBindingSize = adapter_limits.limits.maxUniformBufferBindingSize;
	info->limits.maxStorageBufferBindingSize = adapter_limits.limits.maxStorageBufferBindingSize;

	info->limits.maxVertexBuffers = adapter_limits.limits.maxVertexBuffers;
	info->limits.maxVertexAttributes = adapter_limits.limits.maxVertexAttributes;
	info->limits.maxVertexBufferStride = adapter_limits.limits.maxVertexBufferArrayStride;
	info->limits.maxColorAttachments = adapter_limits.limits.maxColorAttachments;

	info->limits.maxComputeSharedMemorySize = adapter_limits.limits.maxComputeWorkgroupStorageSize;

	info->limits.maxComputeWorkgroupCountX = adapter_limits.limits.maxComputeWorkgroupsPerDimension;
	info->limits.maxComputeWorkgroupCountY = adapter_limits.limits.maxComputeWorkgroupsPerDimension;
	info->limits.maxComputeWorkgroupCountZ = adapter_limits.limits.maxComputeWorkgroupsPerDimension;

	info->limits.maxComputeWorkgroupInvocations = adapter_limits.limits.maxComputeInvocationsPerWorkgroup;

	info->limits.maxComputeWorkgroupLocalSizeX = adapter_limits.limits.maxComputeWorkgroupSizeX;
	info->limits.maxComputeWorkgroupLocalSizeY = adapter_limits.limits.maxComputeWorkgroupSizeY;
	info->limits.maxComputeWorkgroupLocalSizeZ = adapter_limits.limits.maxComputeWorkgroupSizeZ;

	return OPAL_SUCCESS;
}
