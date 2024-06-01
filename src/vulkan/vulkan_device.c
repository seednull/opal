#include "vulkan_internal.h"

/*
 */
Opal_Result vulkan_deviceInitialize(Vulkan_Device *device_ptr, Vulkan_Instance *instance_ptr, VkPhysicalDevice physical_device, VkDevice device)
{
	assert(device_ptr);
	assert(instance_ptr);
	assert(physical_device != VK_NULL_HANDLE);
	assert(device != VK_NULL_HANDLE);

	// vtable
	device_ptr->vtbl.getInfo = vulkan_deviceGetInfo;
	device_ptr->vtbl.getQueue = vulkan_deviceGetQueue;
	device_ptr->vtbl.destroy = vulkan_deviceDestroy;
	device_ptr->vtbl.createBuffer = vulkan_deviceCreateBuffer;
	device_ptr->vtbl.createTexture = vulkan_deviceCreateTexture;
	device_ptr->vtbl.createTextureView = vulkan_deviceCreateTextureView;
	device_ptr->vtbl.mapBuffer = vulkan_deviceMapBuffer;
	device_ptr->vtbl.unmapBuffer = vulkan_deviceUnmapBuffer;
	device_ptr->vtbl.destroyBuffer = vulkan_deviceDestroyBuffer;
	device_ptr->vtbl.destroyTexture = vulkan_deviceDestroyTexture;
	device_ptr->vtbl.destroyTextureView = vulkan_deviceDestroyTextureView;

	// data
	device_ptr->physical_device = physical_device;
	device_ptr->device = device;

	// allocator
#ifdef OPAL_HAS_VMA
	device_ptr->use_vma = instance_ptr->flags & OPAL_INSTANCE_CREATION_FLAGS_USE_VMA;

	if (device_ptr->use_vma > 0)
	{
		VmaVulkanFunctions vulkan_functions = {0};
		vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vulkan_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
		vulkan_functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
		vulkan_functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
		vulkan_functions.vkAllocateMemory = vkAllocateMemory;
		vulkan_functions.vkFreeMemory = vkFreeMemory;
		vulkan_functions.vkMapMemory = vkMapMemory;
		vulkan_functions.vkUnmapMemory = vkUnmapMemory;
		vulkan_functions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
		vulkan_functions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
		vulkan_functions.vkBindBufferMemory = vkBindBufferMemory;
		vulkan_functions.vkBindImageMemory = vkBindImageMemory;
		vulkan_functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
		vulkan_functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
		vulkan_functions.vkCreateBuffer = vkCreateBuffer;
		vulkan_functions.vkDestroyBuffer = vkDestroyBuffer;
		vulkan_functions.vkCreateImage = vkCreateImage;
		vulkan_functions.vkDestroyImage = vkDestroyImage;
		vulkan_functions.vkCmdCopyBuffer = vkCmdCopyBuffer;
		vulkan_functions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
		vulkan_functions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
		vulkan_functions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
		vulkan_functions.vkBindImageMemory2KHR = vkBindImageMemory2;
		vulkan_functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
		vulkan_functions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
		vulkan_functions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;

		VmaAllocatorCreateInfo create_info = {0};
		create_info.device = device;
		create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		create_info.physicalDevice = physical_device;
		create_info.instance = instance_ptr->instance;
		create_info.pVulkanFunctions = &vulkan_functions;

		VkResult result = vmaCreateAllocator(&create_info, &device_ptr->vma_allocator);
		assert(result == VK_SUCCESS);
	}
	else
#endif
	{
		VkPhysicalDeviceProperties properties = {0};
		vkGetPhysicalDeviceProperties(physical_device, &properties);

		uint32_t buffer_image_granularity = (uint32_t)properties.limits.bufferImageGranularity;

		Opal_Result result = vulkan_allocatorInitialize(&device_ptr->allocator, instance_ptr->heap_size, instance_ptr->max_heap_allocations, instance_ptr->max_heaps, buffer_image_granularity);
		assert(result == OPAL_SUCCESS);
	}

	// pools
	opal_poolInitialize(&device_ptr->queues, sizeof(Vulkan_Queue), 32);
	opal_poolInitialize(&device_ptr->buffers, sizeof(Vulkan_Buffer), 32);
	opal_poolInitialize(&device_ptr->images, sizeof(Vulkan_Image), 32);

	// queues
	const Vulkan_DeviceEnginesInfo *engines_info = &device_ptr->device_engines_info;

	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
	{
		uint32_t queue_count = engines_info->queue_counts[i];
		uint32_t queue_family = engines_info->queue_families[i];

		Vulkan_Queue queue = {0};
		queue.family_index = queue_family;

		Opal_PoolHandle *queue_handles = (Opal_PoolHandle *)malloc(sizeof(Opal_PoolHandle) * queue_count);

		for (uint32_t j = 0; j < queue_count; j++)
		{
			vkGetDeviceQueue(device, queue_family, j, &queue.queue);
			queue_handles[j] = opal_poolAddElement(&device_ptr->queues, &queue);
		}

		device_ptr->queue_handles[i] = queue_handles;
	}

	return OPAL_SUCCESS;
}

/*
 */
Opal_Result vulkan_deviceAllocateMemory(Device *this, const Vulkan_AllocationDesc *desc, Vulkan_Allocation *allocation)
{
	assert(this);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Allocator *allocator = &device_ptr->allocator;
	VkDevice vulkan_device = device_ptr->device;
	VkPhysicalDevice vulkan_physical_device = device_ptr->physical_device;

	// resolve allocation type (suballocated or dedicated)
	VkBool32 dedicated = (desc->hint == OPAL_ALLOCATION_HINT_PREFER_DEDICATED);
	const float heap_threshold = 0.7f;

	switch (desc->hint)
	{
		case OPAL_ALLOCATION_HINT_AUTO:
		{
			VkBool32 too_big_for_heap = desc->size > allocator->heap_size * heap_threshold;
			dedicated = too_big_for_heap || desc->requires_dedicated || desc->prefers_dedicated;
		}
		break;

		case OPAL_ALLOCATION_HINT_PREFER_HEAP:
		{
			VkBool32 wont_fit_in_heap = desc->size > allocator->heap_size;
			dedicated = wont_fit_in_heap || desc->requires_dedicated;
			break;
		}
	}

	// fetch memory properties
	VkPhysicalDeviceMemoryProperties memory_properties = {0};
	vkGetPhysicalDeviceMemoryProperties(vulkan_physical_device, &memory_properties);

	// loop over best memory types
	uint32_t memory_type_bits = desc->memory_type_bits;
	uint32_t memory_type = 0;

	while (memory_type_bits > 0)
	{
		Opal_Result opal_result = vulkan_helperFindBestMemoryType(&memory_properties, memory_type_bits, desc->required_flags, desc->preferred_flags, desc->not_preferred_flags, &memory_type);
		if (opal_result != OPAL_SUCCESS)
			return OPAL_NO_MEMORY;

		opal_result = OPAL_NO_MEMORY;

		if (dedicated == VK_FALSE)
			opal_result = vulkan_allocatorAllocateMemory(allocator, vulkan_device, vulkan_physical_device, desc, memory_type, 0, allocation);

		if (opal_result == OPAL_NO_MEMORY)
			opal_result = vulkan_allocatorAllocateMemory(allocator, vulkan_device, vulkan_physical_device, desc, memory_type, 1, allocation);

		if (opal_result == OPAL_SUCCESS)
			return OPAL_SUCCESS;

		memory_type_bits &= ~(1 << memory_type);
	}

	return OPAL_NO_MEMORY;
}

/*
 */
Opal_Result vulkan_deviceGetInfo(Device *this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	Vulkan_Device *ptr = (Vulkan_Device *)this;
	return vulkan_helperFillDeviceInfo(ptr->physical_device, info);
}

Opal_Result vulkan_deviceGetQueue(Device *this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	assert(this);
	assert(queue);
	assert(engine_type < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	Vulkan_Device *ptr = (Vulkan_Device *)this;
	uint32_t queue_count = ptr->device_engines_info.queue_counts[engine_type];

	if (index >= queue_count)
		return OPAL_INVALID_QUEUE_INDEX;

	Opal_PoolHandle *queue_handles = ptr->queue_handles[engine_type];
	assert(queue_handles);

	*queue = queue_handles[index];
	return OPAL_SUCCESS;
}

Opal_Result vulkan_deviceDestroy(Device *this)
{
	assert(this);

	Vulkan_Device *ptr = (Vulkan_Device *)this;

#ifdef OPAL_HAS_VMA
	if (ptr->use_vma > 0)
	{
		vmaDestroyAllocator(ptr->vma_allocator);
	}
	else
#endif
	{
		Opal_Result result = vulkan_allocatorShutdown(&ptr->allocator, ptr->device);
		assert(result == OPAL_SUCCESS);
	}

	// TODO: proper cleanup for all previously created buffers & images
	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
		free(ptr->queue_handles[i]);

	opal_poolShutdown(&ptr->queues);
	opal_poolShutdown(&ptr->buffers);
	opal_poolShutdown(&ptr->images);

	vkDestroyDevice(ptr->device, NULL);
	return OPAL_SUCCESS;
}

/*
 */
Opal_Result vulkan_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	assert(this);
	assert(desc);
	assert(buffer);

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

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;
	VkPhysicalDevice vulkan_physical_device = device_ptr->physical_device;

	VkBuffer vulkan_buffer = VK_NULL_HANDLE;

	VkBufferCreateInfo buffer_info = {0};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = desc->size;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_info.usage = vulkan_helperToBufferUsage(desc->usage);

	Vulkan_Allocation allocation = {0};

#ifdef OPAL_HAS_VMA
	VmaAllocation vma_allocation = VK_NULL_HANDLE;

	if (device_ptr->use_vma > 0)
	{
		VmaAllocationCreateInfo allocation_info = {0};
		allocation_info.preferredFlags = memory_preferred_flags[desc->memory_type];
		allocation_info.requiredFlags = memory_required_flags[desc->memory_type];

		VkResult result = vmaCreateBuffer(device_ptr->vma_allocator, &buffer_info, &allocation_info, &vulkan_buffer, &vma_allocation, NULL);
		if (result != VK_SUCCESS)
			return OPAL_VULKAN_ERROR;
	}
	else
#endif
	{
		Vulkan_Allocator *allocator = &device_ptr->allocator;

		// create buffer
		VkResult result = vkCreateBuffer(vulkan_device, &buffer_info, NULL, &vulkan_buffer);
		if (result != VK_SUCCESS)
			return OPAL_VULKAN_ERROR;

		// get memory requirements
		VkMemoryRequirements2 memory_requirements = {0};
		VkMemoryDedicatedRequirements dedicated_requirements = {0};

		VkBufferMemoryRequirementsInfo2 memory_info = {0};
		memory_info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
		memory_info.buffer = vulkan_buffer;

		dedicated_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;

		memory_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		memory_requirements.pNext = &dedicated_requirements;
		vkGetBufferMemoryRequirements2(vulkan_device, &memory_info, &memory_requirements);

		// fill allocation info
		Vulkan_AllocationDesc allocation_desc = {0};
		allocation_desc.size = memory_requirements.memoryRequirements.size;
		allocation_desc.alignment = memory_requirements.memoryRequirements.alignment;
		allocation_desc.memory_type_bits = memory_requirements.memoryRequirements.memoryTypeBits;
		allocation_desc.required_flags = memory_required_flags[desc->memory_type];
		allocation_desc.preferred_flags = memory_preferred_flags[desc->memory_type];
		allocation_desc.not_preferred_flags = memory_not_preferred_flags[desc->memory_type];
		allocation_desc.resource_type = VULKAN_RESOURCE_TYPE_LINEAR;
		allocation_desc.hint = desc->hint;
		allocation_desc.prefers_dedicated = dedicated_requirements.prefersDedicatedAllocation;
		allocation_desc.requires_dedicated = dedicated_requirements.requiresDedicatedAllocation;

		Opal_Result opal_result = vulkan_deviceAllocateMemory(this, &allocation_desc, &allocation);
		if (opal_result != OPAL_SUCCESS)
		{
			vkDestroyBuffer(vulkan_device, vulkan_buffer, NULL);
			return opal_result;
		}

		// bind memory
		result = vkBindBufferMemory(vulkan_device, vulkan_buffer, allocation.memory, allocation.offset);
		if (result != VK_SUCCESS)
		{
			vkDestroyBuffer(vulkan_device, vulkan_buffer, NULL);
			vulkan_allocatorFreeMemory(allocator, vulkan_device, allocation);
			return OPAL_VULKAN_ERROR;
		}
	}

	// create opal struct
	Vulkan_Buffer result = {0};

	result.buffer = vulkan_buffer;
#if OPAL_HAS_VMA
	result.vma_allocation = vma_allocation;
#endif
	result.allocation = allocation;
	result.map_count = 0;

	*buffer = (Opal_Buffer)opal_poolAddElement(&device_ptr->buffers, &result);
	return OPAL_SUCCESS;
}

Opal_Result vulkan_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	assert(this);
	assert(desc);
	assert(texture);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;
	VkPhysicalDevice vulkan_physical_device = device_ptr->physical_device;

	VkImage vulkan_image = VK_NULL_HANDLE;

	VkImageCreateInfo image_info = {0};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.flags = vulkan_helperToImageCreateFlags(desc);
	image_info.imageType = vulkan_helperToImageType(desc->type);
	image_info.format = vulkan_helperToImageFormat(desc->format);
	image_info.extent.width = desc->width;
	image_info.extent.height = desc->height;
	image_info.extent.depth = desc->depth;
	image_info.mipLevels = desc->mip_count;
	image_info.arrayLayers = desc->layer_count;
	image_info.samples = vulkan_helperToImageSamples(desc->samples);
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = vulkan_helperToImageUsage(desc->usage, desc->format);
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	Vulkan_Allocation allocation = {0};

#if OPAL_HAS_VMA
	VmaAllocation vma_allocation = VK_NULL_HANDLE;

	if (device_ptr->use_vma > 0)
	{
		VmaAllocationCreateInfo allocation_info = {0};
		allocation_info.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VkResult result = vmaCreateImage(device_ptr->vma_allocator, &image_info, &allocation_info, &vulkan_image, &vma_allocation, NULL);
		if (result != VK_SUCCESS)
			return OPAL_VULKAN_ERROR;
	}
	else
#endif
	{
		Vulkan_Allocator *allocator = &device_ptr->allocator;

		// create image
		VkResult result = vkCreateImage(vulkan_device, &image_info, NULL, &vulkan_image);
		if (result != VK_SUCCESS)
			return OPAL_VULKAN_ERROR;

		// get memory requirements
		VkMemoryRequirements2 memory_requirements = {0};
		VkMemoryDedicatedRequirements dedicated_requirements = {0};

		VkImageMemoryRequirementsInfo2 memory_info = {0};
		memory_info.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
		memory_info.image = vulkan_image;

		dedicated_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;

		memory_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		memory_requirements.pNext = &dedicated_requirements;
		vkGetImageMemoryRequirements2(vulkan_device, &memory_info, &memory_requirements);

		// fill allocation info
		Vulkan_AllocationDesc allocation_desc = {0};
		allocation_desc.size = memory_requirements.memoryRequirements.size;
		allocation_desc.alignment = memory_requirements.memoryRequirements.alignment;
		allocation_desc.memory_type_bits = memory_requirements.memoryRequirements.memoryTypeBits;
		allocation_desc.required_flags = 0;
		allocation_desc.preferred_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		allocation_desc.not_preferred_flags = 0;
		allocation_desc.resource_type = VULKAN_RESOURCE_TYPE_NONLINEAR;
		allocation_desc.hint = desc->hint;
		allocation_desc.prefers_dedicated = dedicated_requirements.prefersDedicatedAllocation;
		allocation_desc.requires_dedicated = dedicated_requirements.requiresDedicatedAllocation;

		Opal_Result opal_result = vulkan_deviceAllocateMemory(this, &allocation_desc, &allocation);
		if (opal_result != OPAL_SUCCESS)
		{
			vkDestroyImage(vulkan_device, vulkan_image, NULL);
			return opal_result;
		}

		// bind memory
		result = vkBindImageMemory(vulkan_device, vulkan_image, allocation.memory, allocation.offset);
		if (result != VK_SUCCESS)
		{
			vkDestroyImage(vulkan_device, vulkan_image, NULL);
			vulkan_allocatorFreeMemory(allocator, vulkan_device, allocation);
			return OPAL_VULKAN_ERROR;
		}
	}

	// create opal struct
	Vulkan_Image result = {0};

	result.image = vulkan_image;
#if OPAL_HAS_VMA
	result.vma_allocation = vma_allocation;
#endif
	result.allocation = allocation;

	*texture = (Opal_Texture)opal_poolAddElement(&device_ptr->images, &result);
	return OPAL_SUCCESS;
}

Opal_Result vulkan_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result vulkan_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr)
{
	assert(this);
	assert(buffer != OPAL_NULL_HANDLE);
	assert(ptr);

	Opal_PoolHandle handle = (Opal_PoolHandle)buffer;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, handle);
	assert(buffer_ptr);

#if OPAL_HAS_VMA
	if (device_ptr->use_vma > 0)
	{
		VkResult result = vmaMapMemory(device_ptr->vma_allocator, buffer_ptr->vma_allocation, ptr);

		if (result != VK_SUCCESS)
			return OPAL_VULKAN_ERROR;

		return OPAL_SUCCESS;
	}
#endif

	Vulkan_Allocator *allocator = &device_ptr->allocator;
	assert(allocator);

	uint8_t *block_memory = NULL;
	Opal_Result result = vulkan_allocatorMapMemory(allocator, device_ptr->device, buffer_ptr->allocation, &block_memory);

	if (result != OPAL_SUCCESS)
		return result;

	buffer_ptr->map_count++;
	*ptr = block_memory + buffer_ptr->allocation.offset;

	return OPAL_SUCCESS;
}

Opal_Result vulkan_deviceUnmapBuffer(Device *this, Opal_Buffer buffer)
{
	assert(this);
	assert(buffer != OPAL_NULL_HANDLE);

	Opal_PoolHandle handle = (Opal_PoolHandle)buffer;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, handle);
	assert(buffer_ptr);

#if OPAL_HAS_VMA
	if (device_ptr->use_vma > 0)
	{
		vmaUnmapMemory(device_ptr->vma_allocator, buffer_ptr->vma_allocation);
		return OPAL_SUCCESS;
	}
#endif

	assert(buffer_ptr->map_count > 0);

	Vulkan_Allocator *allocator = &device_ptr->allocator;
	assert(allocator);

	buffer_ptr->map_count--;
	return vulkan_allocatorUnmapMemory(allocator, device_ptr->device, buffer_ptr->allocation);
}

/*
 */
Opal_Result vulkan_deviceDestroyBuffer(Device *this, Opal_Buffer buffer)
{
	assert(this);
	assert(buffer != OPAL_NULL_HANDLE);

	Opal_PoolHandle handle = (Opal_PoolHandle)buffer;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, handle);
	assert(buffer_ptr);

	opal_poolRemoveElement(&device_ptr->buffers, handle);

#if OPAL_HAS_VMA
	if (device_ptr->use_vma)
	{
		vmaDestroyBuffer(device_ptr->vma_allocator, buffer_ptr->buffer, buffer_ptr->vma_allocation);
		return OPAL_SUCCESS;
	}
#endif

	Vulkan_Allocator *allocator = &device_ptr->allocator;
	assert(allocator);

	vkDestroyBuffer(device_ptr->device, buffer_ptr->buffer, NULL);
	Opal_Result result = vulkan_allocatorFreeMemory(allocator, device_ptr->device, buffer_ptr->allocation);

	if (buffer_ptr->map_count > 0)
		vulkan_allocatorUnmapMemory(allocator, device_ptr->device, buffer_ptr->allocation);

	return result;
}

Opal_Result vulkan_deviceDestroyTexture(Device *this, Opal_Texture texture)
{
	assert(this);
	assert(texture != OPAL_NULL_HANDLE);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)texture;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Image *image_ptr = (Vulkan_Image *)opal_poolGetElement(&device_ptr->images, handle);
	assert(image_ptr);

	opal_poolRemoveElement(&device_ptr->images, handle);

#if OPAL_HAS_VMA
	if (device_ptr->use_vma)
	{
		vmaDestroyImage(device_ptr->vma_allocator, image_ptr->image, image_ptr->vma_allocation);
		return OPAL_SUCCESS;
	}
#endif

	Vulkan_Allocator *allocator = &device_ptr->allocator;
	assert(allocator);

	vkDestroyImage(device_ptr->device, image_ptr->image, NULL);
	Opal_Result result = vulkan_allocatorFreeMemory(allocator, device_ptr->device, image_ptr->allocation);
	return result;
}

Opal_Result vulkan_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view)
{
	return OPAL_NOT_SUPPORTED;
}
