#include "vulkan_internal.h"
#include "common/intrinsics.h"

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

VkFormat vulkan_helperToImageFormat(Opal_Format format)
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

VkSampleCountFlagBits vulkan_helperToImageSamples(Opal_Samples samples)
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
Opal_Result vulkan_helperCreateDevice(VkPhysicalDevice physical_device, VkDevice *device)
{
	assert(device);
	assert(physical_device != VK_NULL_HANDLE);

	// get physical device extensions
	uint32_t extension_count = 0;
	VkResult result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extension_count, NULL);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	VkExtensionProperties *extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * extension_count);
	const char **extension_names = (const char **)malloc(sizeof(const char *) * extension_count);

	result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extension_count, extensions);

	if (result != VK_SUCCESS)
	{
		free(extensions);
		free(extension_names);
		return OPAL_VULKAN_ERROR;
	}

	for (uint32_t i = 0; i < extension_count; ++i)
		extension_names[i] = extensions[i].extensionName;

	// get physical device features
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

	vkGetPhysicalDeviceFeatures2(physical_device, &features);

	// get physical device queues
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

	VkQueueFamilyProperties *queue_families = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
	VkDeviceQueueCreateInfo *queue_infos = (VkDeviceQueueCreateInfo *)malloc(sizeof(VkDeviceQueueCreateInfo) * queue_family_count);

	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

	uint32_t max_queues = 0;
	for (uint32_t i = 0; i < queue_family_count; ++i)
	{
		VkQueueFamilyProperties *family = &queue_families[i];

		if (max_queues < family->queueCount)
			max_queues = family->queueCount;
	}

	float *queue_priorities = (float *)malloc(sizeof(float) * max_queues);
	for (uint32_t i = 0; i < max_queues; ++i)
		queue_priorities[i] = 1.0f;

	for (uint32_t i = 0; i < queue_family_count; ++i)
	{
		VkQueueFamilyProperties *family = &queue_families[i];
		VkDeviceQueueCreateInfo *info = &queue_infos[i];

		info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info->pNext = NULL;
		info->flags = 0;
		info->pQueuePriorities = queue_priorities;
		info->queueCount = family->queueCount;
		info->queueFamilyIndex = i;
	}

	// create vulkan device
	VkDeviceCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pNext = &features;
	create_info.enabledExtensionCount = extension_count;
	create_info.ppEnabledExtensionNames = extension_names;
	create_info.queueCreateInfoCount = queue_family_count;
	create_info.pQueueCreateInfos = queue_infos;

	result = vkCreateDevice(physical_device, &create_info, NULL, device);

	free(extensions);
	free(extension_names);
	free(queue_families);
	free(queue_priorities);
	free(queue_infos);

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

	return OPAL_SUCCESS;
}
