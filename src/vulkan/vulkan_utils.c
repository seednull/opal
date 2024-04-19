#include "vulkan_internal.h"
#include "common/intrinsics.h"

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

Opal_Result vulkan_helperFindBestMemoryType(VkPhysicalDevice physical_device, uint32_t memory_type_mask, Opal_AllocationMemoryType allocation_memory_type, uint32_t *memory_type)
{
	assert(memory_type);
	assert(physical_device != VK_NULL_HANDLE);

	static uint32_t memory_required_flags[] =
	{
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
	};

	static uint32_t memory_preferred_flags[] =
	{
		0,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0,
		VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
	};

	static uint32_t memory_not_preferred_flags[] =
	{
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
		VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0,
	};

	uint32_t required_flags = memory_required_flags[allocation_memory_type];
	uint32_t preferred_flags = memory_preferred_flags[allocation_memory_type];
	uint32_t not_preferred_flags = memory_not_preferred_flags[allocation_memory_type];

	VkPhysicalDeviceMemoryProperties memory_properties = {0};
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

	uint32_t best_cost = UINT32_MAX;
	Opal_Result result = OPAL_VULKAN_ERROR;

	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
	{
		uint32_t mask = 1 << i;
		if ((mask & memory_type_mask) == 0)
			continue;

		VkMemoryType vulkan_memory_type = memory_properties.memoryTypes[i];

		if ((~vulkan_memory_type.propertyFlags & required_flags) != 0)
			continue;

		// TODO: count bits
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
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: info->gpu_type = OPAL_GPU_TYPE_DISCRETE; break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: info->gpu_type = OPAL_GPU_TYPE_INTEGRATED; break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU: info->gpu_type = OPAL_GPU_TYPE_CPU; break;
		default: info->gpu_type = OPAL_GPU_TYPE_UNKNOWN; break;
	}

	info->driver_version = properties.driverVersion;
	info->vendor_id = properties.vendorID;
	info->device_id = properties.deviceID;
	info->tessellation_shader = features.features.tessellationShader;
	info->geometry_shader = features.features.geometryShader;
	info->compute_shader = 1;
	info->mesh_pipeline = mesh_features.meshShader && mesh_features.taskShader;
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
