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

Opal_PresentMode vulkan_helperFromPresentMode(VkPresentModeKHR mode)
{
	switch (mode)
	{
		case VK_PRESENT_MODE_IMMEDIATE_KHR: return OPAL_PRESENT_MODE_IMMEDIATE;
		case VK_PRESENT_MODE_MAILBOX_KHR: return OPAL_PRESENT_MODE_MAILBOX;
		case VK_PRESENT_MODE_FIFO_KHR: return OPAL_PRESENT_MODE_FIFO;
		default: assert(0); return OPAL_PRESENT_MODE_ENUM_FORCE32;
	}
}

VkColorSpaceKHR vulkan_helperToColorSpace(Opal_ColorSpace color_space)
{
	static VkColorSpaceKHR vk_color_spaces[] =
	{
		VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	};

	return vk_color_spaces[color_space];
}

Opal_ColorSpace vulkan_helperFromColorSpace(VkColorSpaceKHR color_space)
{
	switch (color_space)
	{
		case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: return OPAL_COLOR_SPACE_SRGB;
		default: assert(0); return OPAL_COLOR_SPACE_ENUM_FORCE32;
	}
}

VkImageAspectFlags vulkan_helperToAspectMask(Opal_TextureFormat format)
{
	if (format >= OPAL_TEXTURE_FORMAT_COLOR_BEGIN && format <= OPAL_TEXTURE_FORMAT_COLOR_END)
		return VK_IMAGE_ASPECT_COLOR_BIT;

	if (format >= OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_BEGIN && format <= OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_END)
	{
		VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (format >= OPAL_TEXTURE_FORMAT_D16_UNORM_S8_UINT)
			aspect_flags |= VK_IMAGE_ASPECT_STENCIL_BIT;

		return aspect_flags;
	}

	return 0;
}

VkFormat vulkan_helperToImageFormat(Opal_TextureFormat format)
{
	static VkFormat vk_formats[] =
	{
		VK_FORMAT_UNDEFINED,

		// 8-bit formats
		VK_FORMAT_R8_UNORM, VK_FORMAT_R8_SNORM, VK_FORMAT_R8_UINT, VK_FORMAT_R8_SINT,
		VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R8G8_UINT,VK_FORMAT_R8G8_SINT,
		VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8G8B8A8_SINT,

		// 16-bit formats
		VK_FORMAT_R16_UINT, VK_FORMAT_R16_SINT, VK_FORMAT_R16_SFLOAT,
		VK_FORMAT_R16G16_UINT, VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SFLOAT,
		VK_FORMAT_R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SFLOAT,

		// 32-bit formats
		VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT, VK_FORMAT_R32_SFLOAT,
		VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32_SFLOAT,
		VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_SINT, VK_FORMAT_R32G32B32A32_SFLOAT,

		// special 4-channel formats
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_FORMAT_B8G8R8A8_SRGB,
		VK_FORMAT_R8G8B8A8_SRGB,

		// hdr 32-bit formats
		VK_FORMAT_B10G11R11_UFLOAT_PACK32,
		VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,

		// bc formats
		VK_FORMAT_BC1_RGB_UNORM_BLOCK,
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

		// depth_stencil formats
		VK_FORMAT_D16_UNORM,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
	};

	return vk_formats[format];
}

Opal_TextureFormat vulkan_helperFromImageFormat(VkFormat format)
{
	switch (format)
	{
		case VK_FORMAT_UNDEFINED: return OPAL_TEXTURE_FORMAT_UNDEFINED;

		// 8-bit formats
		case VK_FORMAT_R8_UNORM: return OPAL_TEXTURE_FORMAT_R8_UNORM;
		case VK_FORMAT_R8_SNORM: return OPAL_TEXTURE_FORMAT_R8_SNORM;
		case VK_FORMAT_R8_UINT: return OPAL_TEXTURE_FORMAT_R8_UINT;
		case VK_FORMAT_R8_SINT: return OPAL_TEXTURE_FORMAT_R8_SINT;
		case VK_FORMAT_R8G8_UNORM: return OPAL_TEXTURE_FORMAT_RG8_UNORM;
		case VK_FORMAT_R8G8_SNORM: return OPAL_TEXTURE_FORMAT_RG8_SNORM;
		case VK_FORMAT_R8G8_UINT: return OPAL_TEXTURE_FORMAT_RG8_UINT;
		case VK_FORMAT_R8G8_SINT: return OPAL_TEXTURE_FORMAT_RG8_SINT;
		case VK_FORMAT_R8G8B8A8_UNORM: return OPAL_TEXTURE_FORMAT_RGBA8_UNORM;
		case VK_FORMAT_R8G8B8A8_SNORM: return OPAL_TEXTURE_FORMAT_RGBA8_SNORM;
		case VK_FORMAT_R8G8B8A8_UINT: return OPAL_TEXTURE_FORMAT_RGBA8_UINT;
		case VK_FORMAT_R8G8B8A8_SINT: return OPAL_TEXTURE_FORMAT_RGBA8_SINT;

		// 16-bit formats
		case VK_FORMAT_R16_UINT: return OPAL_TEXTURE_FORMAT_R16_UINT;
		case VK_FORMAT_R16_SINT: return OPAL_TEXTURE_FORMAT_R16_SINT;
		case VK_FORMAT_R16_SFLOAT: return OPAL_TEXTURE_FORMAT_R16_SFLOAT;
		case VK_FORMAT_R16G16_UINT: return OPAL_TEXTURE_FORMAT_RG16_UINT;
		case VK_FORMAT_R16G16_SINT: return OPAL_TEXTURE_FORMAT_RG16_SINT;
		case VK_FORMAT_R16G16_SFLOAT: return OPAL_TEXTURE_FORMAT_RG16_SFLOAT;
		case VK_FORMAT_R16G16B16A16_UINT: return OPAL_TEXTURE_FORMAT_RGBA16_UINT;
		case VK_FORMAT_R16G16B16A16_SINT: return OPAL_TEXTURE_FORMAT_RGBA16_SINT;
		case VK_FORMAT_R16G16B16A16_SFLOAT: return OPAL_TEXTURE_FORMAT_RGBA16_SFLOAT;

		// 32-bit formats
		case VK_FORMAT_R32_UINT: return OPAL_TEXTURE_FORMAT_R32_UINT;
		case VK_FORMAT_R32_SINT: return OPAL_TEXTURE_FORMAT_R32_SINT;
		case VK_FORMAT_R32_SFLOAT: return OPAL_TEXTURE_FORMAT_R32_SFLOAT;
		case VK_FORMAT_R32G32_UINT: return OPAL_TEXTURE_FORMAT_RG32_UINT;
		case VK_FORMAT_R32G32_SINT: return OPAL_TEXTURE_FORMAT_RG32_SINT;
		case VK_FORMAT_R32G32_SFLOAT: return OPAL_TEXTURE_FORMAT_RG32_SFLOAT;
		case VK_FORMAT_R32G32B32A32_UINT: return OPAL_TEXTURE_FORMAT_RGBA32_UINT;
		case VK_FORMAT_R32G32B32A32_SINT: return OPAL_TEXTURE_FORMAT_RGBA32_SINT;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return OPAL_TEXTURE_FORMAT_RGBA32_SFLOAT;

		// special 4-channel formats
		case VK_FORMAT_B8G8R8A8_UNORM: return OPAL_TEXTURE_FORMAT_BGRA8_UNORM;
		case VK_FORMAT_B8G8R8A8_SRGB: return OPAL_TEXTURE_FORMAT_BGRA8_UNORM_SRGB;
		case VK_FORMAT_R8G8B8A8_SRGB: return OPAL_TEXTURE_FORMAT_RGBA8_UNORM_SRGB;

		// hdr 32-bit formats
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return OPAL_TEXTURE_FORMAT_RG11B10_UFLOAT;
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return OPAL_TEXTURE_FORMAT_RGB9E5_UFLOAT;

		// bc formats
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_BC1_R5G6B5_UNORM;
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_BC1_R5G6B5_UNORM_SRGB;
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_BC1_R5G6B5A1_UNORM;
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_BC1_R5G6B5A1_UNORM_SRGB;
		case VK_FORMAT_BC2_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_BC2_R5G6B5A4_UNORM;
		case VK_FORMAT_BC2_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_BC2_R5G6B5A4_UNORM_SRGB;
		case VK_FORMAT_BC3_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_BC3_RGBA8_UNORM;
		case VK_FORMAT_BC3_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_BC3_RGBA8_UNORM_SRGB;
		case VK_FORMAT_BC4_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_BC4_R8_UNORM;
		case VK_FORMAT_BC4_SNORM_BLOCK: return OPAL_TEXTURE_FORMAT_BC4_R8_SNORM;
		case VK_FORMAT_BC5_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_BC5_RG8_UNORM;
		case VK_FORMAT_BC5_SNORM_BLOCK: return OPAL_TEXTURE_FORMAT_BC5_RG8_SNORM;
		case VK_FORMAT_BC6H_UFLOAT_BLOCK: return OPAL_TEXTURE_FORMAT_BC6H_RGB16_UFLOAT;
		case VK_FORMAT_BC6H_SFLOAT_BLOCK: return OPAL_TEXTURE_FORMAT_BC6H_RGB16_SFLOAT;
		case VK_FORMAT_BC7_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_BC7_RGBA8_UNORM;
		case VK_FORMAT_BC7_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_BC7_RGBA8_UNORM_SRGB;

		// etc formats
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ETC2_RGB8_UNORM;
		case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ETC2_RGB8_UNORM_SRGB;
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ETC2_RGB8A1_UNORM;
		case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ETC2_RGB8A1_UNORM_SRGB;
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ETC2_RGBA8_UNORM;
		case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ETC2_RGBA8_UNORM_SRGB;
		case VK_FORMAT_EAC_R11_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_EAC_R11_UNORM;
		case VK_FORMAT_EAC_R11_SNORM_BLOCK: return OPAL_TEXTURE_FORMAT_EAC_R11_SNORM;
		case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_EAC_RG11_UNORM;
		case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return OPAL_TEXTURE_FORMAT_EAC_RG11_SNORM;

		// astc formats
		case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_4x4_UNORM;
		case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_4x4_UNORM_SRGB;
		case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_5x4_UNORM;
		case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_5x4_UNORM_SRGB;
		case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_5x5_UNORM;
		case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_5x5_UNORM_SRGB;
		case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_6x5_UNORM;
		case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_6x5_UNORM_SRGB;
		case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_6x6_UNORM;
		case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_6x6_UNORM_SRGB;
		case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_8x5_UNORM;
		case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_8x5_UNORM_SRGB;
		case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_8x6_UNORM;
		case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_8x6_UNORM_SRGB;
		case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_8x8_UNORM;
		case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_8x8_UNORM_SRGB;
		case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_10x5_UNORM;
		case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_10x5_UNORM_SRGB;
		case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_10x6_UNORM;
		case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_10x6_UNORM_SRGB;
		case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_10x8_UNORM;
		case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_10x8_UNORM_SRGB;
		case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_10x10_UNORM;
		case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_10x10_UNORM_SRGB;
		case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_12x10_UNORM;
		case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_12x10_UNORM_SRGB;
		case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_12x12_UNORM;
		case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return OPAL_TEXTURE_FORMAT_ASTC_12x12_UNORM_SRGB;

		// depth_stencil formats
		case VK_FORMAT_D16_UNORM: return OPAL_TEXTURE_FORMAT_D16_UNORM;
		case VK_FORMAT_D32_SFLOAT: return OPAL_TEXTURE_FORMAT_D32_SFLOAT;
		case VK_FORMAT_D16_UNORM_S8_UINT: return OPAL_TEXTURE_FORMAT_D16_UNORM_S8_UINT;
		case VK_FORMAT_D24_UNORM_S8_UINT: return OPAL_TEXTURE_FORMAT_D24_UNORM_S8_UINT;
		case VK_FORMAT_D32_SFLOAT_S8_UINT: return OPAL_TEXTURE_FORMAT_D32_SFLOAT_S8_UINT;

		default: assert(0); return OPAL_TEXTURE_FORMAT_ENUM_FORCE32;
	};
}

VkFormat vulkan_helperToVertexFormat(Opal_VertexFormat format)
{
	static VkFormat vk_formats[] =
	{
		VK_FORMAT_UNDEFINED,

		// 8-bit formats
		VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R8G8_UINT,VK_FORMAT_R8G8_SINT,
		VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8G8B8A8_SINT,

		// 16-bit formats
		VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16_UINT,VK_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SFLOAT,
		VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SFLOAT,

		// 32-bit formats
		VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT, VK_FORMAT_R32_SFLOAT,
		VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32_SFLOAT,
		VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32_SFLOAT,
		VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_SINT, VK_FORMAT_R32G32B32A32_SFLOAT,

		// special formats
		VK_FORMAT_A2R10G10B10_UNORM_PACK32,
	};

	return vk_formats[format];
}

VkIndexType vulkan_helperToIndexType(Opal_IndexFormat format)
{
	static VkIndexType vk_index_types[] =
	{
		VK_INDEX_TYPE_NONE_KHR,
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

VkImageUsageFlags vulkan_helperToImageUsage(Opal_TextureUsageFlags flags, Opal_TextureFormat format)
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
		if (format >= OPAL_TEXTURE_FORMAT_COLOR_BEGIN && format <= OPAL_TEXTURE_FORMAT_COLOR_END)
			result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		
		if (format >= OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_BEGIN && format <= OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_END)
			result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	return result;
}

VkImageAspectFlags vulkan_helperToImageAspectMask(Opal_TextureFormat format)
{
	VkImageAspectFlags result = 0;

	if (format >= OPAL_TEXTURE_FORMAT_COLOR_BEGIN && format <= OPAL_TEXTURE_FORMAT_COLOR_END)
		result |= VK_IMAGE_ASPECT_COLOR_BIT;

	if (format >= OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_BEGIN && format <= OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_END)
		result |= VK_IMAGE_ASPECT_DEPTH_BIT;

	if (format == OPAL_TEXTURE_FORMAT_D16_UNORM_S8_UINT || format == OPAL_TEXTURE_FORMAT_D24_UNORM_S8_UINT || format == OPAL_TEXTURE_FORMAT_D32_SFLOAT_S8_UINT)
		result |= VK_IMAGE_ASPECT_STENCIL_BIT;

	return result;
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

	if (flags & OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE)
		result |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;

	if (flags & OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT)
		result |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

	if (flags & OPAL_BUFFER_USAGE_SHADER_BINDING_TABLE)
		result |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;

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

VkGeometryTypeKHR vulkan_helperToAccelerationStructureGeometryType(Opal_AccelerationStructureGeometryType type)
{
	static VkGeometryTypeKHR vk_geometry_type[] =
	{
		VK_GEOMETRY_TYPE_TRIANGLES_KHR,
		VK_GEOMETRY_TYPE_AABBS_KHR,
	};

	return vk_geometry_type[type];
}

VkGeometryFlagsKHR vulkan_helperToAccelerationStructureGeometryFlags(Opal_AccelerationStructureGeometryFlags flags)
{
	VkGeometryFlagsKHR vk_flags = 0;

	if (flags & OPAL_ACCELERATION_STRUCTURE_GEOMETRY_FLAGS_OPAQUE)
		vk_flags |= VK_GEOMETRY_OPAQUE_BIT_KHR;

	if (flags & OPAL_ACCELERATION_STRUCTURE_GEOMETRY_FLAGS_NO_DUPLICATE_ANYHIT)
		vk_flags |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;

	return vk_flags;
}

VkGeometryInstanceFlagsKHR vulkan_helperToAccelerationStructureGeometryInstanceFlags(Opal_AccelerationStructureInstanceFlags flags)
{
	VkGeometryInstanceFlagsKHR vk_flags = 0;

	if (flags & OPAL_ACCELERATION_STRUCTURE_INSTANCE_FLAGS_TRIANGLE_CULL_DISABLE)
		vk_flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;

	if (flags & OPAL_ACCELERATION_STRUCTURE_INSTANCE_FLAGS_TRIANGLE_FRONT_COUNTER_CLOCKWISE)
		vk_flags |= VK_GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT_KHR;

	if (flags & OPAL_ACCELERATION_STRUCTURE_INSTANCE_FLAGS_FORCE_OPAQUE)
		vk_flags |= VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR;

	if (flags & OPAL_ACCELERATION_STRUCTURE_INSTANCE_FLAGS_FORCE_NO_OPAQUE)
		vk_flags |= VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR;

	return vk_flags;
}

VkAccelerationStructureTypeKHR vulkan_helperToAccelerationStructureType(Opal_AccelerationStructureType type)
{
	static VkAccelerationStructureTypeKHR vk_acceleration_structure_types[] =
	{
		VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
	};

	return vk_acceleration_structure_types[type];
}

VkBuildAccelerationStructureFlagBitsKHR vulkan_helperToAccelerationStructureBuildFlags(Opal_AccelerationStructureBuildFlags flags)
{
	VkBuildAccelerationStructureFlagBitsKHR vk_flags = 0;

	if (flags & OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_ALLOW_UPDATE)
		vk_flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;

	if (flags & OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_ALLOW_COMPACTION)
		vk_flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;

	if (flags & OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_PREFER_FAST_TRACE)
		vk_flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

	if (flags & OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_PREFER_FAST_BUILD)
		vk_flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;

	if (flags & OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_MINIMIZE_MEMORY)
		vk_flags |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;

	return vk_flags;
}

VkBuildAccelerationStructureModeKHR vulkan_helperToAccelerationStructureBuildMode(Opal_AccelerationStructureBuildMode mode)
{
	static VkBuildAccelerationStructureModeKHR vk_acceleration_structure_build_modes[] =
	{
		VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR,
	};

	return vk_acceleration_structure_build_modes[mode];
}

VkCopyAccelerationStructureModeKHR vulkan_helperToAccelerationStructureCopyMode(Opal_AccelerationStructureCopyMode mode)
{
	static VkCopyAccelerationStructureModeKHR vk_acceleration_structure_copy_modes[] =
	{
		VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR,
		VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR,
	};

	return vk_acceleration_structure_copy_modes[mode];
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

VkDescriptorType vulkan_helperToDescriptorType(Opal_DescriptorType type)
{
	static VkDescriptorType vk_descriptor_types[] =
	{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		VK_DESCRIPTOR_TYPE_SAMPLER,
		VK_DESCRIPTOR_TYPE_SAMPLER,
		VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
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
		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
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
		| VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;

	VkPipelineStageFlags all_non_fragment = all_compute
		| VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
		| VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
		| VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
		| VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;

	VkPipelineStageFlags result = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	if (state & OPAL_RESOURCE_STATE_VERTEX_AND_UNIFORM_BUFFER)
		result |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | all_non_fragment;

	if (state & OPAL_RESOURCE_STATE_INDEX_BUFFER)
		result |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

	if (state & OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT)
		result |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	if (state & OPAL_RESOURCE_STATE_UNORDERED_ACCESS)
		result |= all_compute;

	if (state & OPAL_RESOURCE_STATE_DEPTH_STENCIL_WRITE)
		result |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	if (state & OPAL_RESOURCE_STATE_DEPTH_STENCIL_READ)
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
		| VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;

	VkPipelineStageFlags all_non_fragment = all_compute
		| VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
		| VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
		| VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
		| VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;

	VkPipelineStageFlags result = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	if (state & OPAL_RESOURCE_STATE_VERTEX_AND_UNIFORM_BUFFER)
		result |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | all_non_fragment;

	if (state & OPAL_RESOURCE_STATE_INDEX_BUFFER)
		result |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

	if (state & OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT)
		result |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	if (state & OPAL_RESOURCE_STATE_UNORDERED_ACCESS)
		result |= all_compute;

	if (state & OPAL_RESOURCE_STATE_DEPTH_STENCIL_WRITE)
		result |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

	if (state & OPAL_RESOURCE_STATE_DEPTH_STENCIL_READ)
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
		result |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // depth_stencil attachments?

	if (state & OPAL_RESOURCE_STATE_UNORDERED_ACCESS)
		result |= VK_ACCESS_SHADER_WRITE_BIT;

	if (state & OPAL_RESOURCE_STATE_DEPTH_STENCIL_WRITE)
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
		result |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT; // depth_stencil attachments?

	if (state & OPAL_RESOURCE_STATE_UNORDERED_ACCESS)
		result |= VK_ACCESS_SHADER_READ_BIT;

	if (state & OPAL_RESOURCE_STATE_DEPTH_STENCIL_READ)
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

	if (state & OPAL_RESOURCE_STATE_DEPTH_STENCIL_WRITE)
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	if (state & OPAL_RESOURCE_STATE_DEPTH_STENCIL_READ)
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	if (state & OPAL_RESOURCE_STATE_NON_FRAGMENT_SHADER_RESOURCE)
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	if (state & OPAL_RESOURCE_STATE_FRAGMENT_SHADER_RESOURCE)
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	if (state & OPAL_RESOURCE_STATE_COPY_DEST)
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	if (state & OPAL_RESOURCE_STATE_COPY_SOURCE)
		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

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

	VkPhysicalDeviceBufferDeviceAddressFeaturesKHR buffer_device_address_features = {0};
	buffer_device_address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracing_features = {0};
	raytracing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;

	VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR raytracing_maintenance_features = {0};
	raytracing_maintenance_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR;

	VkPhysicalDeviceMeshShaderFeaturesEXT mesh_features = {0};
	mesh_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;

	VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timeline_semaphore_features = {0};
	timeline_semaphore_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR;

	VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptor_buffer_features = {0};
	descriptor_buffer_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;

	features.pNext = &dynamic_rendering_features;
	dynamic_rendering_features.pNext = &acceleration_structure_features;
	acceleration_structure_features.pNext = &buffer_device_address_features;
	buffer_device_address_features.pNext = &raytracing_features;
	raytracing_features.pNext = &raytracing_maintenance_features;
	raytracing_maintenance_features.pNext = &mesh_features;
	mesh_features.pNext = &timeline_semaphore_features;
	timeline_semaphore_features.pNext = &descriptor_buffer_features;

	vkGetPhysicalDeviceFeatures2(physical_device, &features);

	// check available device extensions
	uint32_t num_device_extensions = 0;
	VkResult result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &num_device_extensions, NULL);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	VkExtensionProperties *device_extensions = malloc(sizeof(VkExtensionProperties) * num_device_extensions);
	result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &num_device_extensions, device_extensions);
	if (result != VK_SUCCESS)
	{
		free(device_extensions);
		return OPAL_VULKAN_ERROR;
	}

	VkBool32 has_dynamic_rendering = VK_FALSE;
	VkBool32 has_acceleration_structure = VK_FALSE;
	VkBool32 has_buffer_device_address = VK_FALSE;
	VkBool32 has_raytracing = VK_FALSE;
	VkBool32 has_raytracing_maintenance = VK_FALSE;
	VkBool32 has_meshlet = VK_FALSE;
	VkBool32 has_timeline_semaphores = VK_FALSE;
	VkBool32 has_descriptor_buffer = VK_FALSE;

	for (uint32_t i = 0; i < num_device_extensions; ++i)
	{
		const char *device_extension_name = device_extensions[i].extensionName;

		if (strcmp(device_extension_name, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
			has_dynamic_rendering = dynamic_rendering_features.dynamicRendering;

		if (strcmp(device_extension_name, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0)
			has_acceleration_structure = acceleration_structure_features.accelerationStructure;

		if (strcmp(device_extension_name, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) == 0)
			has_buffer_device_address = buffer_device_address_features.bufferDeviceAddress;

		if (strcmp(device_extension_name, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0)
			has_raytracing = raytracing_features.rayTracingPipeline;

		if (strcmp(device_extension_name, VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME) == 0)
			has_raytracing_maintenance = raytracing_maintenance_features.rayTracingMaintenance1;

		if (strcmp(device_extension_name, VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0)
			has_meshlet = mesh_features.meshShader && mesh_features.taskShader;

		if (strcmp(device_extension_name, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) == 0)
			has_timeline_semaphores = timeline_semaphore_features.timelineSemaphore;

		if (strcmp(device_extension_name, VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME) == 0)
			has_descriptor_buffer = descriptor_buffer_features.descriptorBuffer;
	}

	free(device_extensions);

	// fill required extensions
	const char *extensions[32];
	uint32_t num_extensions = 0;

	extensions[num_extensions++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

	typedef struct VkParavozikKHR_t
	{
		VkStructureType type;
		void *next;
	} VkParavozikKHR;

	VkParavozikKHR *paravozik = (VkParavozikKHR *)&features;
	paravozik->next = NULL;

	if (has_dynamic_rendering == VK_TRUE)
	{
		extensions[num_extensions++] = VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME;
		extensions[num_extensions++] = VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME;
		extensions[num_extensions++] = VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME;

		paravozik->next = &dynamic_rendering_features;

		paravozik = (VkParavozikKHR *)&dynamic_rendering_features;
		paravozik->next = NULL;
	}

	if (has_acceleration_structure == VK_TRUE)
	{
		extensions[num_extensions++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
		extensions[num_extensions++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
		extensions[num_extensions++] = VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;

		paravozik->next = &acceleration_structure_features;

		paravozik = (VkParavozikKHR *)&acceleration_structure_features;
		paravozik->next = NULL;
	}

	if (has_buffer_device_address == VK_TRUE)
	{
		extensions[num_extensions++] = VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME;

		paravozik->next = &buffer_device_address_features;

		paravozik = (VkParavozikKHR *)&buffer_device_address_features;
		paravozik->next = NULL;
	}

	if (has_raytracing == VK_TRUE)
	{
		extensions[num_extensions++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
		extensions[num_extensions++] = VK_KHR_SPIRV_1_4_EXTENSION_NAME;
		extensions[num_extensions++] = VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME;

		paravozik->next = &raytracing_features;

		paravozik = (VkParavozikKHR *)&raytracing_features;
		paravozik->next = NULL;
	}

	if (has_raytracing_maintenance == VK_TRUE)
	{
		extensions[num_extensions++] = VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME;

		paravozik->next = &raytracing_maintenance_features;

		paravozik = (VkParavozikKHR *)&raytracing_maintenance_features;
		paravozik->next = NULL;
	}

	if (has_meshlet == VK_TRUE)
	{
		extensions[num_extensions++] = VK_EXT_MESH_SHADER_EXTENSION_NAME;

		mesh_features.multiviewMeshShader = VK_FALSE;
		mesh_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;

		paravozik->next = &mesh_features;

		paravozik = (VkParavozikKHR *)&mesh_features;
		paravozik->next = NULL;
	}

	if (has_timeline_semaphores == VK_TRUE)
	{
		extensions[num_extensions++] = VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME;

		paravozik->next = &timeline_semaphore_features;

		paravozik = (VkParavozikKHR *)&timeline_semaphore_features;
		paravozik->next = NULL;
	}

	if (has_descriptor_buffer == VK_TRUE)
	{
		extensions[num_extensions++] = VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME;

		paravozik->next = &descriptor_buffer_features;

		paravozik = (VkParavozikKHR *)&descriptor_buffer_features;
		paravozik->next = NULL;
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

	result = vkCreateDevice(physical_device, &create_info, NULL, device);

	free(queue_priorities);

	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

Opal_Result vulkan_helperFillDeviceInfo(VkPhysicalDevice device, Opal_DeviceInfo *info)
{
	assert(device != VK_NULL_HANDLE);
	assert(info);

	// get properties
	VkPhysicalDeviceMaintenance3Properties maintenance = {0};
	maintenance.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES;

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties = {0};
	raytracing_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	raytracing_properties.pNext = &maintenance;

	VkPhysicalDeviceProperties2 properties = {0};
	properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties.pNext = &raytracing_properties;

	vkGetPhysicalDeviceProperties2(device, &properties);

	// get features
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

	vkGetPhysicalDeviceFeatures2(device, &features);

	// check available device extensions
	uint32_t num_device_extensions = 0;
	VkResult result = vkEnumerateDeviceExtensionProperties(device, NULL, &num_device_extensions, NULL);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	VkExtensionProperties *device_extensions = malloc(sizeof(VkExtensionProperties) * num_device_extensions);
	result = vkEnumerateDeviceExtensionProperties(device, NULL, &num_device_extensions, device_extensions);
	if (result != VK_SUCCESS)
	{
		free(device_extensions);
		return OPAL_VULKAN_ERROR;
	}

	VkBool32 has_acceleration_structure = VK_FALSE;
	VkBool32 has_raytracing = VK_FALSE;
	VkBool32 has_meshlet = VK_FALSE;

	for (uint32_t i = 0; i < num_device_extensions; ++i)
	{
		const char *device_extension_name = device_extensions[i].extensionName;

		if (strcmp(device_extension_name, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0)
			has_acceleration_structure = acceleration_structure_features.accelerationStructure;

		if (strcmp(device_extension_name, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0)
			has_raytracing = raytracing_features.rayTracingPipeline;

		if (strcmp(device_extension_name, VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0)
			has_meshlet = mesh_features.meshShader && mesh_features.taskShader;
	}

	free(device_extensions);

	// fill basic info
	memset(info, 0, sizeof(Opal_DeviceInfo));

	strncpy(info->name, properties.properties.deviceName, 256);

	switch (properties.properties.deviceType)
	{
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: info->device_type = OPAL_DEVICE_TYPE_DISCRETE; break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: info->device_type = OPAL_DEVICE_TYPE_INTEGRATED; break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU: info->device_type = OPAL_DEVICE_TYPE_CPU; break;
		default: info->device_type = OPAL_DEVICE_TYPE_UNKNOWN; break;
	}

	info->api = OPAL_API_VULKAN;
	info->driver_version = properties.properties.driverVersion;
	info->vendor_id = properties.properties.vendorID;
	info->device_id = properties.properties.deviceID;

	// fill features
	Vulkan_DeviceEnginesInfo device_engines_info = {0};
	vulkan_helperFillDeviceEnginesInfo(device, &device_engines_info);

	memcpy(info->features.queue_count, &device_engines_info.queue_counts, sizeof(uint32_t) * OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	info->features.tessellation_shader = (features.features.tessellationShader == VK_TRUE);
	info->features.geometry_shader = (features.features.geometryShader == VK_TRUE);
	info->features.compute_pipeline = 1;
	info->features.meshlet_pipeline = (has_meshlet == VK_TRUE);
	info->features.raytrace_pipeline = has_raytracing && has_acceleration_structure;
	info->features.texture_compression_etc2 = (features.features.textureCompressionETC2 == VK_TRUE);
	info->features.texture_compression_astc = (features.features.textureCompressionASTC_LDR == VK_TRUE);
	info->features.texture_compression_bc = (features.features.textureCompressionBC == VK_TRUE);

	// fill limits
	info->limits.max_texture_dimension_1d = properties.properties.limits.maxImageDimension1D;
	info->limits.max_texture_dimension_2d = properties.properties.limits.maxImageDimension2D;
	info->limits.max_texture_dimension_3d = properties.properties.limits.maxImageDimension3D;
	info->limits.max_texture_array_layers = properties.properties.limits.maxImageArrayLayers;

	info->limits.max_buffer_size = maintenance.maxMemoryAllocationSize;
	info->limits.min_uniform_buffer_offset_alignment = properties.properties.limits.minUniformBufferOffsetAlignment;
	info->limits.min_storage_buffer_offset_alignment = properties.properties.limits.minStorageBufferOffsetAlignment;

	info->limits.max_descriptor_sets = properties.properties.limits.maxBoundDescriptorSets;
	info->limits.max_uniform_buffer_binding_size = properties.properties.limits.maxUniformBufferRange;
	info->limits.max_storage_buffer_binding_size = properties.properties.limits.maxStorageBufferRange;

	info->limits.max_vertex_buffers = properties.properties.limits.maxVertexInputBindings;
	info->limits.max_vertex_attributes = properties.properties.limits.maxVertexInputAttributes;
	info->limits.max_vertex_buffer_stride = properties.properties.limits.maxVertexInputBindingStride;
	info->limits.max_color_attachments = properties.properties.limits.maxFragmentOutputAttachments;

	info->limits.max_compute_shared_memory_size = properties.properties.limits.maxComputeSharedMemorySize;

	info->limits.max_compute_workgroup_count_x = properties.properties.limits.maxComputeWorkGroupCount[0];
	info->limits.max_compute_workgroup_count_y = properties.properties.limits.maxComputeWorkGroupCount[1];
	info->limits.max_compute_workgroup_count_z = properties.properties.limits.maxComputeWorkGroupCount[2];

	info->limits.max_compute_workgroup_invocations = properties.properties.limits.maxComputeWorkGroupInvocations;

	info->limits.max_compute_workgroup_local_size_x = properties.properties.limits.maxComputeWorkGroupSize[0];
	info->limits.max_compute_workgroup_local_size_y = properties.properties.limits.maxComputeWorkGroupSize[1];
	info->limits.max_compute_workgroup_local_size_z = properties.properties.limits.maxComputeWorkGroupSize[2];

	info->limits.max_raytrace_recursion_depth = raytracing_properties.maxRayRecursionDepth;
	info->limits.max_raytrace_hit_attribute_size = raytracing_properties.maxRayHitAttributeSize;

	return OPAL_SUCCESS;
}
