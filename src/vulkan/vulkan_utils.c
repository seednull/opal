#include "vulkan_internal.h"
#include "common/intrinsics.h"

#include <stdlib.h>
#include <string.h>

/*
 */
VkImageCreateFlags vulkan_helperToImageCreateFlags(const Opal_TextureDesc *desc)
{
	VkImageCreateFlags result = 0;

	if (desc->type == OPAL_TEXTURE_TYPE_3D)
		result |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

	if (desc->type == OPAL_TEXTURE_TYPE_2D && desc->width == desc->height && desc->layer_count >= 6)
		result |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	return result;
}

VkImageType vulkan_helperToImageType(Opal_TextureType type)
{
	static VkImageType vk_image_types[] = 
	{
		VK_IMAGE_TYPE_1D,
		VK_IMAGE_TYPE_2D,
		VK_IMAGE_TYPE_3D,
	};

	return vk_image_types[type];
}

VkImageViewType vulkan_helperToImageViewType(Opal_TextureViewType type)
{
	static VkImageViewType vk_image_view_types[] =
	{
		VK_IMAGE_VIEW_TYPE_1D,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_IMAGE_VIEW_TYPE_2D_ARRAY,
		VK_IMAGE_VIEW_TYPE_CUBE,
		VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
		VK_IMAGE_VIEW_TYPE_3D,
	};

	return vk_image_view_types[type];
}

VkImageLayout vulkan_helperToImageLayout(VkDescriptorType type)
{
	static VkImageLayout vk_image_layouts[] =
	{
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_UNDEFINED,
	};

	return vk_image_layouts[type];
}

VkPresentModeKHR vulkan_helperToPresentMode(Opal_PresentMode mode)
{
	static VkImageViewType vk_present_modes[] =
	{
		VK_PRESENT_MODE_IMMEDIATE_KHR,
		VK_PRESENT_MODE_FIFO_KHR,
		VK_PRESENT_MODE_MAILBOX_KHR,
	};

	return vk_present_modes[mode];
}

VkImageAspectFlags vulkan_helperToAspectMask(Opal_Format format)
{
	if (format >= OPAL_FORMAT_COLOR_BEGIN && format <= OPAL_FORMAT_COLOR_END)
		return VK_IMAGE_ASPECT_COLOR_BIT;

	if (format >= OPAL_FORMAT_DEPTHSTENCIL_BEGIN && format <= OPAL_FORMAT_DEPTHSTENCIL_END)
	{
		VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (format >= OPAL_FORMAT_D16_UNORM_S8_UINT)
			aspect_flags |= VK_IMAGE_ASPECT_STENCIL_BIT;

		return aspect_flags;
	}

	return 0;
}

VkFormat vulkan_helperToFormat(Opal_Format format)
{
	static VkFormat vk_formats[] =
	{
		VK_FORMAT_UNDEFINED,

		// 8-bit formats
		VK_FORMAT_R8_UNORM, VK_FORMAT_R8_SNORM, VK_FORMAT_R8_UINT, VK_FORMAT_R8_SINT, VK_FORMAT_R8_SRGB,
		VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R8G8_UINT,VK_FORMAT_R8G8_SINT, VK_FORMAT_R8G8_SRGB,
		VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8_SNORM, VK_FORMAT_R8G8B8_UINT, VK_FORMAT_R8G8B8_SINT, VK_FORMAT_R8G8B8_SRGB,
		VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8G8B8A8_SINT, VK_FORMAT_R8G8B8A8_SRGB,

		VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM, VK_FORMAT_B8G8R8A8_UINT, VK_FORMAT_B8G8R8A8_SINT, VK_FORMAT_B8G8R8A8_SRGB,

		// 16-bit formats
		VK_FORMAT_R16_UNORM, VK_FORMAT_R16_SNORM, VK_FORMAT_R16_UINT, VK_FORMAT_R16_SINT,VK_FORMAT_R16_SFLOAT,
		VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SFLOAT,
		VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16_SINT, VK_FORMAT_R16G16B16_SFLOAT,
		VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SFLOAT,

		// 32-bit formats
		VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT, VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_UINT,
		VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_UINT,
		VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_UINT,
		VK_FORMAT_R32G32B32A32_SINT, VK_FORMAT_R32G32B32A32_SFLOAT,

		// hdr 32-bit formats
		VK_FORMAT_B10G11R11_UFLOAT_PACK32,
		VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,

		// bc formats
		VK_FORMAT_BC1_RGB_SRGB_BLOCK,
		VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
		VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
		VK_FORMAT_BC2_UNORM_BLOCK,
		VK_FORMAT_BC2_SRGB_BLOCK,
		VK_FORMAT_BC3_UNORM_BLOCK,
		VK_FORMAT_BC3_SRGB_BLOCK,
		VK_FORMAT_BC4_UNORM_BLOCK,
		VK_FORMAT_BC4_SNORM_BLOCK,
		VK_FORMAT_BC5_UNORM_BLOCK,
		VK_FORMAT_BC5_SNORM_BLOCK,
		VK_FORMAT_BC6H_UFLOAT_BLOCK,
		VK_FORMAT_BC6H_SFLOAT_BLOCK,
		VK_FORMAT_BC7_UNORM_BLOCK,
		VK_FORMAT_BC7_SRGB_BLOCK,

		// etc formats
		VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
		VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
		VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
		VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
		VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
		VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
		VK_FORMAT_EAC_R11_UNORM_BLOCK,
		VK_FORMAT_EAC_R11_SNORM_BLOCK,
		VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
		VK_FORMAT_EAC_R11G11_SNORM_BLOCK,

		// astc formats
		VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
		VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
		VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
		VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
		VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
		VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
		VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
		VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
		VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
		VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
		VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
		VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
		VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
		VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
		VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
		VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
		VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
		VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
		VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
		VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
		VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
		VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
		VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
		VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
		VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
		VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
		VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
		VK_FORMAT_ASTC_12x12_SRGB_BLOCK,

		// depthstencil formats
		VK_FORMAT_D16_UNORM,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_BC1_RGB_UNORM_BLOCK,
	};

	return vk_formats[format];
}

VkIndexType vulkan_helperToIndexType(Opal_IndexFormat format)
{
	static VkIndexType vk_index_types[] =
	{
		VK_INDEX_TYPE_UINT16,
		VK_INDEX_TYPE_UINT32,
	};

	return vk_index_types[format];
}

VkSampleCountFlagBits vulkan_helperToSamples(Opal_Samples samples)
{
	static VkSampleCountFlagBits vk_sample_count_bits[] =
	{
		VK_SAMPLE_COUNT_1_BIT,
		VK_SAMPLE_COUNT_2_BIT,
		VK_SAMPLE_COUNT_4_BIT,
		VK_SAMPLE_COUNT_8_BIT,
		VK_SAMPLE_COUNT_16_BIT,
		VK_SAMPLE_COUNT_32_BIT,
		VK_SAMPLE_COUNT_64_BIT,
	};

	return vk_sample_count_bits[samples];
}

VkImageUsageFlags vulkan_helperToImageUsage(Opal_TextureUsageFlags flags, Opal_Format format)
{
	VkImageUsageFlags result = 0;

	if (flags & OPAL_TEXTURE_USAGE_TRANSFER_SRC)
		result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	if (flags & OPAL_TEXTURE_USAGE_TRANSFER_DST)
		result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	if (flags & OPAL_TEXTURE_USAGE_SHADER_SAMPLED)
		result |= VK_IMAGE_USAGE_SAMPLED_BIT;

	if (flags & OPAL_TEXTURE_USAGE_UNORDERED_ACCESS)
		result |= VK_IMAGE_USAGE_STORAGE_BIT;

	if (flags & OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT)
	{
		if (format >= OPAL_FORMAT_COLOR_BEGIN && format <= OPAL_FORMAT_COLOR_END)
			result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		
		if (format >= OPAL_FORMAT_DEPTHSTENCIL_BEGIN && format <= OPAL_FORMAT_DEPTHSTENCIL_END)
			result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	return result;
}

VkImageLayout vulkan_helperToInitialImageLayout(Opal_TextureUsageFlags flags, Opal_Format format)
{
	if (flags & OPAL_TEXTURE_USAGE_SHADER_SAMPLED)
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	if (flags & OPAL_TEXTURE_USAGE_UNORDERED_ACCESS)
		return VK_IMAGE_LAYOUT_GENERAL;

	return VK_IMAGE_LAYOUT_UNDEFINED;
}

/*
 */
VkBufferUsageFlags vulkan_helperToBufferUsage(Opal_BufferUsageFlags flags)
{
	VkBufferUsageFlags result = 0;

	if (flags & OPAL_BUFFER_USAGE_TRANSFER_SRC)
		result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	if (flags & OPAL_BUFFER_USAGE_TRANSFER_DST)
		result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	if (flags & OPAL_BUFFER_USAGE_VERTEX)
		result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	if (flags & OPAL_BUFFER_USAGE_INDEX)
		result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	if (flags & OPAL_BUFFER_USAGE_UNIFORM)
		result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	if (flags & OPAL_BUFFER_USAGE_STORAGE)
		result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	if (flags & OPAL_BUFFER_USAGE_INDIRECT)
		result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

	return result;
}

VkFilter vulkan_helperToFilter(Opal_SamplerFilterMode mode)
{
	static VkFilter vk_filters[] =
	{
		VK_FILTER_NEAREST,
		VK_FILTER_LINEAR,
	};

	return vk_filters[mode];
}

VkSamplerMipmapMode vulkan_helperToSamplerMipmapMode(Opal_SamplerFilterMode mode)
{
	static VkSamplerMipmapMode vk_sampler_mipmap_modes[] =
	{
		VK_SAMPLER_MIPMAP_MODE_NEAREST,
		VK_SAMPLER_MIPMAP_MODE_LINEAR,
	};

	return vk_sampler_mipmap_modes[mode];
}

VkSamplerAddressMode vulkan_helperToSamplerAddressMode(Opal_SamplerAddressMode mode)
{
	static VkSamplerAddressMode vk_sampler_address_modes[] =
	{
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	};

	return vk_sampler_address_modes[mode];
}

VkCompareOp vulkan_helperToCompareOp(Opal_CompareOp op)
{
	static VkCompareOp vk_compare_ops[] =
	{
		VK_COMPARE_OP_NEVER,
		VK_COMPARE_OP_LESS,
		VK_COMPARE_OP_EQUAL,
		VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_COMPARE_OP_GREATER,
		VK_COMPARE_OP_NOT_EQUAL,
		VK_COMPARE_OP_GREATER_OR_EQUAL,
		VK_COMPARE_OP_ALWAYS,
	};

	return vk_compare_ops[op];
}

VkDescriptorType vulkan_helperToDescriptorType(Opal_BindingType type)
{
	static VkDescriptorType vk_descriptor_types[] =
	{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	};

	return vk_descriptor_types[type];
}

VkShaderStageFlags vulkan_helperToShaderStageFlags(Opal_ShaderStage stage)
{
	VkShaderStageFlags result = 0;
	
	if (stage & OPAL_SHADER_STAGE_VERTEX)
		result |= VK_SHADER_STAGE_VERTEX_BIT;

	if (stage & OPAL_SHADER_STAGE_TESSELLATION_CONTROL)
		result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

	if (stage & OPAL_SHADER_STAGE_TESSELLATION_EVALUATION)
		result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

	if (stage & OPAL_SHADER_STAGE_GEOMETRY)
		result |= VK_SHADER_STAGE_GEOMETRY_BIT;

	if (stage & OPAL_SHADER_STAGE_FRAGMENT)
		result |= VK_SHADER_STAGE_FRAGMENT_BIT;

	if (stage & OPAL_SHADER_STAGE_COMPUTE)
		result |= VK_SHADER_STAGE_COMPUTE_BIT;

	if (stage & OPAL_SHADER_STAGE_TASK)
		result |= VK_SHADER_STAGE_TASK_BIT_EXT;

	if (stage & OPAL_SHADER_STAGE_MESH)
		result |= VK_SHADER_STAGE_MESH_BIT_EXT;

	if (stage & OPAL_SHADER_STAGE_RAYGEN)
		result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	if (stage & OPAL_SHADER_STAGE_ANY_HIT)
		result |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

	if (stage & OPAL_SHADER_STAGE_CLOSEST_HIT)
		result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	if (stage & OPAL_SHADER_STAGE_MISS)
		result |= VK_SHADER_STAGE_MISS_BIT_KHR;

	if (stage & OPAL_SHADER_STAGE_INTERSECTION)
		result |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;

	return result;
}

VkVertexInputRate vulkan_helperToVertexInputRate(Opal_VertexInputRate rate)
{
	static VkVertexInputRate vk_vertex_input_rates[] =
	{
		VK_VERTEX_INPUT_RATE_VERTEX,
		VK_VERTEX_INPUT_RATE_INSTANCE,
	};

	return vk_vertex_input_rates[rate];
}

VkPrimitiveTopology vulkan_helperToPrimitiveTopology(Opal_PrimitiveType type)
{
	static VkPrimitiveTopology vk_primitive_topologies[] =
	{
		VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
		VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
	};

	return vk_primitive_topologies[type];
}

uint32_t vulkan_helperToPatchControlPoints(Opal_PrimitiveType type)
{
	static uint32_t vk_patch_control_points[] =
	{
		0,
		0,
		0,
		3,
		4,
	};

	return vk_patch_control_points[type];
}

VkCullModeFlags vulkan_helperToCullMode(Opal_CullMode mode)
{
	static VkCullModeFlags vk_cull_mode_flags[] =
	{
		VK_CULL_MODE_NONE,
		VK_CULL_MODE_FRONT_BIT,
		VK_CULL_MODE_BACK_BIT,
		VK_CULL_MODE_FRONT_AND_BACK,
	};

	return vk_cull_mode_flags[mode];
}

VkFrontFace vulkan_helperToFrontFace(Opal_FrontFace face)
{
	static VkFrontFace vk_front_faces[] =
	{
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FRONT_FACE_CLOCKWISE,
	};

	return vk_front_faces[face];
}

VkStencilOp vulkan_helperToStencilOp(Opal_StencilOp op)
{
	static VkStencilOp vk_stencil_ops[] =
	{
		VK_STENCIL_OP_KEEP,
		VK_STENCIL_OP_ZERO,
		VK_STENCIL_OP_REPLACE,
		VK_STENCIL_OP_INVERT,
		VK_STENCIL_OP_INCREMENT_AND_CLAMP,
		VK_STENCIL_OP_DECREMENT_AND_CLAMP,
		VK_STENCIL_OP_INCREMENT_AND_WRAP,
		VK_STENCIL_OP_DECREMENT_AND_WRAP,
	};

	return vk_stencil_ops[op];
}

VkBlendFactor vulkan_helperToBlendFactor(Opal_BlendFactor factor)
{
	static VkBlendFactor vk_blend_factors[] =
	{
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_FACTOR_ONE,
		VK_BLEND_FACTOR_SRC_COLOR,
		VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
		VK_BLEND_FACTOR_DST_COLOR,
		VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
		VK_BLEND_FACTOR_SRC_ALPHA,
		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		VK_BLEND_FACTOR_DST_ALPHA,
		VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	};

	return vk_blend_factors[factor];
}

VkBlendOp vulkan_helperToBlendOp(Opal_BlendOp op)
{
	static VkBlendOp vk_blend_ops[] =
	{
		VK_BLEND_OP_ADD,
		VK_BLEND_OP_SUBTRACT,
		VK_BLEND_OP_REVERSE_SUBTRACT,
		VK_BLEND_OP_MIN,
		VK_BLEND_OP_MAX,
	};

	return vk_blend_ops[op];
}

VkAttachmentLoadOp vulkan_helperToLoadOp(Opal_LoadOp op)
{
	static VkAttachmentLoadOp vk_load_ops[] =
	{
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_LOAD_OP_LOAD,
	};

	return vk_load_ops[op];
}

VkAttachmentStoreOp vulkan_helperToStoreOp(Opal_StoreOp op)
{
	static VkAttachmentStoreOp vk_store_ops[] =
	{
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_STORE,
	};

	return vk_store_ops[op];
}

VkPipelineStageFlags vulkan_helperToPipelineWaitStage(Opal_ResourceState state)
{
	VkPipelineStageFlags all_compute = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
		| VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR
		| VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT
		| VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;

	VkPipelineStageFlags all_non_fragment = all_compute
		| VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
		| VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
		| VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
		| VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;

	VkPipelineStageFlags result = 0;

	if (state & OPAL_RESOURCE_STATE_GENERAL)
		result |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	if (state & OPAL_RESOURCE_STATE_VERTEX_AND_UNIFORM_BUFFER)
		result |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

	if (state & OPAL_RESOURCE_STATE_INDEX_BUFFER)
		result |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

	if (state & OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT)
		result |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	if (state & OPAL_RESOURCE_STATE_UNORDERED_ACCESS)
		result |= all_compute;

	if (state & OPAL_RESOURCE_STATE_DEPTHSTENCIL_WRITE)
		result |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	if (state & OPAL_RESOURCE_STATE_DEPTHSTENCIL_READ)
		result |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	if (state & OPAL_RESOURCE_STATE_NON_FRAGMENT_SHADER_RESOURCE)
		result |= all_non_fragment;

	if (state & OPAL_RESOURCE_STATE_FRAGMENT_SHADER_RESOURCE)
		result |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	if (state & OPAL_RESOURCE_STATE_COPY_DEST)
		result |= VK_PIPELINE_STAGE_TRANSFER_BIT;

	if (state & OPAL_RESOURCE_STATE_COPY_SOURCE)
		result |= VK_PIPELINE_STAGE_TRANSFER_BIT;

	if (state & OPAL_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
		result |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

	if (state & OPAL_RESOURCE_STATE_PRESENT)
		result |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	return result;
}

VkPipelineStageFlags vulkan_helperToPipelineBlockStage(Opal_ResourceState state)
{
	VkPipelineStageFlags all_compute = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
		| VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR
		| VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT
		| VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;

	VkPipelineStageFlags all_non_fragment = all_compute
		| VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
		| VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
		| VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
		| VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;

	VkPipelineStageFlags result = 0;

	if (state & OPAL_RESOURCE_STATE_GENERAL)
		result |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	if (state & OPAL_RESOURCE_STATE_VERTEX_AND_UNIFORM_BUFFER)
		result |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

	if (state & OPAL_RESOURCE_STATE_INDEX_BUFFER)
		result |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

	if (state & OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT)
		result |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	if (state & OPAL_RESOURCE_STATE_UNORDERED_ACCESS)
		result |= all_compute;

	if (state & OPAL_RESOURCE_STATE_DEPTHSTENCIL_WRITE)
		result |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	if (state & OPAL_RESOURCE_STATE_DEPTHSTENCIL_READ)
		result |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	if (state & OPAL_RESOURCE_STATE_NON_FRAGMENT_SHADER_RESOURCE)
		result |= all_non_fragment;

	if (state & OPAL_RESOURCE_STATE_FRAGMENT_SHADER_RESOURCE)
		result |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	if (state & OPAL_RESOURCE_STATE_COPY_DEST)
		result |= VK_PIPELINE_STAGE_TRANSFER_BIT;

	if (state & OPAL_RESOURCE_STATE_COPY_SOURCE)
		result |= VK_PIPELINE_STAGE_TRANSFER_BIT;

	if (state & OPAL_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
		result |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

	if (state & OPAL_RESOURCE_STATE_PRESENT)
		result |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	return result;
}

VkAccessFlags vulkan_helperToFlushAccessMask(Opal_ResourceState state)
{
	VkAccessFlags result = VK_ACCESS_NONE;

	if (state & OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT)
		result |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // depthstencil attachments?

	if (state & OPAL_RESOURCE_STATE_UNORDERED_ACCESS)
		result |= VK_ACCESS_SHADER_WRITE_BIT;

	if (state & OPAL_RESOURCE_STATE_DEPTHSTENCIL_WRITE)
		result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	if (state & OPAL_RESOURCE_STATE_NON_FRAGMENT_SHADER_RESOURCE)
		result |= VK_ACCESS_SHADER_WRITE_BIT;

	if (state & OPAL_RESOURCE_STATE_FRAGMENT_SHADER_RESOURCE)
		result |= VK_ACCESS_SHADER_WRITE_BIT;

	if (state & OPAL_RESOURCE_STATE_COPY_DEST)
		result |= VK_ACCESS_TRANSFER_WRITE_BIT;

	if (state & OPAL_RESOURCE_STATE_COPY_SOURCE)
		result |= VK_ACCESS_TRANSFER_WRITE_BIT;

	if (state & OPAL_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
		result |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;

	return result;
}

VkAccessFlags vulkan_helperToInvalidateAccessMask(Opal_ResourceState state)
{
	VkAccessFlags result = VK_ACCESS_NONE;

	if (state & OPAL_RESOURCE_STATE_VERTEX_AND_UNIFORM_BUFFER)
		result |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT;

	if (state & OPAL_RESOURCE_STATE_INDEX_BUFFER)
		result |= VK_ACCESS_INDEX_READ_BIT;

	if (state & OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT)
		result |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT; // depthstencil attachments?

	if (state & OPAL_RESOURCE_STATE_UNORDERED_ACCESS)
		result |= VK_ACCESS_SHADER_READ_BIT;

	if (state & OPAL_RESOURCE_STATE_DEPTHSTENCIL_READ)
		result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

	if (state & OPAL_RESOURCE_STATE_NON_FRAGMENT_SHADER_RESOURCE)
		result |= VK_ACCESS_SHADER_READ_BIT;

	if (state & OPAL_RESOURCE_STATE_FRAGMENT_SHADER_RESOURCE)
		result |= VK_ACCESS_SHADER_READ_BIT;

	if (state & OPAL_RESOURCE_STATE_COPY_DEST)
		result |= VK_ACCESS_TRANSFER_READ_BIT;

	if (state & OPAL_RESOURCE_STATE_COPY_SOURCE)
		result |= VK_ACCESS_TRANSFER_READ_BIT;

	if (state & OPAL_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE)
		result |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

	return result;
}

VkImageLayout vulkan_helperToImageLayoutTransition(Opal_ResourceState state, VkImageAspectFlags aspect)
{
	if (state & OPAL_RESOURCE_STATE_GENERAL)
		return VK_IMAGE_LAYOUT_GENERAL;

	if (state & OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT)
	{
		if (aspect & VK_IMAGE_ASPECT_COLOR_BIT)
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		if (aspect & VK_IMAGE_ASPECT_DEPTH_BIT)
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	if (state & OPAL_RESOURCE_STATE_UNORDERED_ACCESS)
		return VK_IMAGE_LAYOUT_GENERAL;

	if (state & OPAL_RESOURCE_STATE_DEPTHSTENCIL_WRITE)
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	if (state & OPAL_RESOURCE_STATE_DEPTHSTENCIL_READ)
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	if (state & OPAL_RESOURCE_STATE_NON_FRAGMENT_SHADER_RESOURCE)
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	if (state & OPAL_RESOURCE_STATE_FRAGMENT_SHADER_RESOURCE)
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	if (state & OPAL_RESOURCE_STATE_COPY_DEST)
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	if (state & OPAL_RESOURCE_STATE_COPY_SOURCE)
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	if (state & OPAL_RESOURCE_STATE_PRESENT)
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	return VK_IMAGE_LAYOUT_UNDEFINED;
}

Opal_Result vulkan_helperFillDeviceEnginesInfo(VkPhysicalDevice physical_device, Vulkan_DeviceEnginesInfo *info)
{
	assert(info);

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

	VkQueueFamilyProperties *queue_families = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

	static VkQueueFlags device_engine_required_masks[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX] =
	{
		VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
		VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
		VK_QUEUE_TRANSFER_BIT,
	};

	static VkQueueFlags device_engine_exclude_masks[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX] =
	{
		0,
		VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_DECODE_BIT_KHR,
		VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_DECODE_BIT_KHR,
	};

	for (uint32_t engine_type = 0; engine_type < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++engine_type)
	{
		VkQueueFlags required_mask = device_engine_required_masks[engine_type];
		VkQueueFlags exclude_mask = device_engine_exclude_masks[engine_type];

		uint32_t queue_count = 0;
		uint32_t queue_family_index = VK_QUEUE_FAMILY_IGNORED;

		for (uint32_t i = 0; i < queue_family_count; ++i)
		{
			const VkQueueFamilyProperties *queue_family = &queue_families[i];

			VkQueueFlags current_flags = queue_family->queueFlags;

			if ((current_flags & exclude_mask) != 0)
				continue;

			if ((~current_flags & required_mask) != 0)
				continue;
			
			if (queue_count < queue_family->queueCount)
			{
				queue_family_index = i;
				queue_count = queue_family->queueCount;
			}
		}

		info->queue_families[engine_type] = queue_family_index;
		info->queue_counts[engine_type] = queue_count;
	}

	free(queue_families);
	return OPAL_SUCCESS;
}

Opal_Result vulkan_helperFindBestMemoryType(const VkPhysicalDeviceMemoryProperties *memory_properties, uint32_t memory_type_mask, uint32_t required_flags, uint32_t preferred_flags, uint32_t not_preferred_flags, uint32_t *memory_type)
{
	assert(memory_type);
	assert(memory_properties);

	uint32_t best_cost = UINT32_MAX;
	Opal_Result result = OPAL_VULKAN_ERROR;

	for (uint32_t i = 0; i < memory_properties->memoryTypeCount; ++i)
	{
		uint32_t mask = 1 << i;
		if ((mask & memory_type_mask) == 0)
			continue;

		VkMemoryType vulkan_memory_type = memory_properties->memoryTypes[i];

		if ((~vulkan_memory_type.propertyFlags & required_flags) != 0)
			continue;

		uint32_t preferred_cost_bits = ~vulkan_memory_type.propertyFlags & preferred_flags;
		uint32_t not_preferred_cost_bits = vulkan_memory_type.propertyFlags & not_preferred_flags;

		uint32_t score = popcnt(preferred_cost_bits) + popcnt(not_preferred_cost_bits);

		if (score <= best_cost)
		{
			*memory_type = i;
			best_cost = score;
			result = OPAL_SUCCESS;
		}
	}

	return result;
}

/*
 */
Opal_Result vulkan_helperCreateDevice(VkPhysicalDevice physical_device, Vulkan_DeviceEnginesInfo *info, VkDevice *device)
{
	assert(physical_device != VK_NULL_HANDLE);
	assert(info);
	assert(device);

	// get physical device features
	VkPhysicalDeviceFeatures2 features = {0};
	features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = {0};
	dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;

	VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features = {0};
	acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

	VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_features = {0};
	buffer_device_address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
		
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_features = {0};
	raytracing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;

	VkPhysicalDeviceMeshShaderFeaturesEXT mesh_features = {0};
	mesh_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;

	features.pNext = &dynamic_rendering_features;
	dynamic_rendering_features.pNext = &acceleration_structure_features;
	acceleration_structure_features.pNext = &buffer_device_address_features;
	buffer_device_address_features.pNext = &raytracing_features;
	raytracing_features.pNext = &mesh_features;

	vkGetPhysicalDeviceFeatures2(physical_device, &features);

	// fill required extensions
	const char *extensions[16];
	uint32_t num_extensions = 0;

	extensions[num_extensions++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

	typedef struct VkParavozikKHR_t
	{
		VkStructureType type;
		void *next;
	} VkParavozikKHR;

	features.pNext = NULL;
	dynamic_rendering_features.pNext = NULL;
	acceleration_structure_features.pNext = NULL;
	buffer_device_address_features.pNext = NULL;
	raytracing_features.pNext = NULL;

	VkParavozikKHR *paravozik = (VkParavozikKHR *)&features;

	if (dynamic_rendering_features.dynamicRendering == VK_TRUE)
	{
		extensions[num_extensions++] = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME;
		extensions[num_extensions++] = VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME;
		extensions[num_extensions++] = VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME;

		paravozik->next = &dynamic_rendering_features;
		paravozik = (VkParavozikKHR *)&dynamic_rendering_features;
	}

	if (acceleration_structure_features.accelerationStructure == VK_TRUE)
	{
		extensions[num_extensions++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
		extensions[num_extensions++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
		extensions[num_extensions++] = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;

		paravozik->next = &acceleration_structure_features;
		paravozik = (VkParavozikKHR *)&acceleration_structure_features;
	}

	if (buffer_device_address_features.bufferDeviceAddress == VK_TRUE)
	{
		extensions[num_extensions++] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;

		paravozik->next = &buffer_device_address_features;
		paravozik = (VkParavozikKHR *)&buffer_device_address_features;
	}

	if (raytracing_features.rayTracingPipeline == VK_TRUE)
	{
		extensions[num_extensions++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
		extensions[num_extensions++] = VK_KHR_SPIRV_1_4_EXTENSION_NAME;
		extensions[num_extensions++] = VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME;

		paravozik->next = &raytracing_features;
		paravozik = (VkParavozikKHR *)&raytracing_features;
	}

	if (mesh_features.meshShader == VK_TRUE && mesh_features.taskShader == VK_TRUE)
	{
		extensions[num_extensions++] = VK_EXT_MESH_SHADER_EXTENSION_NAME;

		paravozik->next = &mesh_features;
		paravozik = (VkParavozikKHR *)&mesh_features;
	}

	// get physical device queues
	vulkan_helperFillDeviceEnginesInfo(physical_device, info);
	VkDeviceQueueCreateInfo queue_infos[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX];

	uint32_t max_queues = 0;
	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
	{
		uint32_t queue_count = info->queue_counts[i];
		if (max_queues < queue_count)
			max_queues = queue_count;
	}

	float *queue_priorities = (float *)malloc(sizeof(float) * max_queues);
	for (uint32_t i = 0; i < max_queues; ++i)
		queue_priorities[i] = 1.0f;

	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
	{
		VkDeviceQueueCreateInfo *queue_info = &queue_infos[i];

		queue_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info->pNext = NULL;
		queue_info->flags = 0;
		queue_info->pQueuePriorities = queue_priorities;
		queue_info->queueCount = info->queue_counts[i];
		queue_info->queueFamilyIndex = info->queue_families[i];
	}

	// create vulkan device
	VkDeviceCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pNext = &features;
	create_info.enabledExtensionCount = num_extensions;
	create_info.ppEnabledExtensionNames = extensions;
	create_info.queueCreateInfoCount = OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX;
	create_info.pQueueCreateInfos = queue_infos;

	VkResult result = vkCreateDevice(physical_device, &create_info, NULL, device);

	free(queue_priorities);

	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

Opal_Result vulkan_helperFillDeviceInfo(VkPhysicalDevice device, Opal_DeviceInfo *info)
{
	assert(device != VK_NULL_HANDLE);
	assert(info);

	VkPhysicalDeviceProperties properties = {0};

	VkPhysicalDeviceFeatures2 features = {0};
	features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

	VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features = {0};
	acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_features = {0};
	raytracing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;

	VkPhysicalDeviceMeshShaderFeaturesEXT mesh_features = {0};
	mesh_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;

	mesh_features.pNext = &acceleration_structure_features;
	raytracing_features.pNext = &mesh_features;
	features.pNext = &raytracing_features;

	vkGetPhysicalDeviceProperties(device, &properties);
	vkGetPhysicalDeviceFeatures2(device, &features);

	memset(info, 0, sizeof(Opal_DeviceInfo));

	strncpy(info->name, properties.deviceName, 256);

	switch (properties.deviceType)
	{
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: info->device_type = OPAL_DEVICE_TYPE_DISCRETE; break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: info->device_type = OPAL_DEVICE_TYPE_INTEGRATED; break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU: info->device_type = OPAL_DEVICE_TYPE_CPU; break;
		default: info->device_type = OPAL_DEVICE_TYPE_UNKNOWN; break;
	}

	info->driver_version = properties.driverVersion;
	info->vendor_id = properties.vendorID;
	info->device_id = properties.deviceID;
	info->tessellation_shader = features.features.tessellationShader;
	info->geometry_shader = features.features.geometryShader;
	info->compute_pipeline = 1;
	info->meshlet_pipeline = mesh_features.meshShader && mesh_features.taskShader;
info->raytrace_pipeline = raytracing_features.rayTracingPipeline && acceleration_structure_features.accelerationStructure;
	info->texture_compression_etc2 = features.features.textureCompressionETC2;
	info->texture_compression_astc = features.features.textureCompressionASTC_LDR;
	info->texture_compression_bc = features.features.textureCompressionBC;

	uint64_t offset = properties.limits.minUniformBufferOffsetAlignment;
	if (offset < properties.limits.minStorageBufferOffsetAlignment)
		offset = properties.limits.minStorageBufferOffsetAlignment;

	info->max_buffer_alignment = offset;

	VkQueueFlags device_engine_required_masks[] =
	{
		VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
		VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
		VK_QUEUE_TRANSFER_BIT,
	};

	VkQueueFlags device_engine_exclude_masks[] =
	{
		0,
		VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_DECODE_BIT_KHR,
		VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_DECODE_BIT_KHR,
	};

	Vulkan_DeviceEnginesInfo device_engines_info = {0};
	vulkan_helperFillDeviceEnginesInfo(device, &device_engines_info);

	memcpy(info->queue_count, &device_engines_info.queue_counts, sizeof(uint32_t) * OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	return OPAL_SUCCESS;
}
