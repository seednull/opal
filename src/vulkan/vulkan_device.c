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
	VkPhysicalDeviceProperties properties = {0};
	vkGetPhysicalDeviceProperties(physical_device, &properties);

	Opal_Result result = vulkan_allocatorInitialize(&device_ptr->allocator, instance_ptr->heap_size, instance_ptr->max_heap_allocations, instance_ptr->max_heaps, properties.limits.bufferImageGranularity);
	assert(result == OPAL_SUCCESS);

	return OPAL_SUCCESS;
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

Opal_Result vulkan_deviceDestroy(Device *this)
{
	assert(this);

	Vulkan_Device *ptr = (Vulkan_Device *)this;

	Opal_Result result = vulkan_allocatorShutdown(&ptr->allocator, ptr->device);
	assert(result == OPAL_SUCCESS);
	
	vkDestroyDevice(ptr->device, NULL);
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
	VkBool32 prefers_dedicated = desc->prefers_dedicated;
	VkBool32 requires_dedicated = desc->requires_dedicated;

	VkDeviceSize size = desc->size;
	VkDeviceSize alignment = desc->alignment;

	VkBool32 dedicated = (desc->hint == OPAL_ALLOCATION_HINT_PREFER_DEDICATED);
	const float heap_threshold = 0.7f;

	switch (desc->hint)
	{
		case OPAL_ALLOCATION_HINT_AUTO:
		{
			VkBool32 too_big_for_heap = desc->size > allocator->heap_size * heap_threshold;
			dedicated = too_big_for_heap || requires_dedicated || prefers_dedicated;
		}
		break;

		case OPAL_ALLOCATION_HINT_PREFER_HEAP:
		{
			VkBool32 wont_fit_in_heap = desc->size > allocator->heap_size;
			dedicated = wont_fit_in_heap || requires_dedicated;
			break;
		}
	}

	// fetch heap budgets & usage
	VkPhysicalDeviceMemoryProperties2 memory_properties = {0};
	VkPhysicalDeviceMemoryBudgetPropertiesEXT memory_budgets = {0};

	memory_budgets.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;

	memory_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
	memory_properties.pNext = &memory_budgets;
	vkGetPhysicalDeviceMemoryProperties2(vulkan_physical_device, &memory_properties);

	// loop over best memory types
	uint32_t memory_type_bits = desc->memory_type_bits;
	uint32_t memory_type = 0;
	uint32_t resource_type = desc->resource_type;

	while (memory_type_bits > 0)
	{
		Opal_Result opal_result = vulkan_helperFindBestMemoryType(vulkan_physical_device, memory_type_bits, desc->required_flags, desc->preferred_flags, desc->not_preferred_flags, &memory_type);
		if (opal_result != OPAL_SUCCESS)
			return OPAL_NO_MEMORY;

		uint32_t heap_index = memory_properties.memoryProperties.memoryTypes[memory_type].heapIndex;
		if (memory_budgets.heapUsage[heap_index] > memory_budgets.heapBudget[heap_index])
			return OPAL_NO_MEMORY;

		opal_result = OPAL_NO_MEMORY;

		if (dedicated == VK_FALSE)
			opal_result = vulkan_allocatorAllocateMemory(allocator, vulkan_device, size, alignment, resource_type, memory_type, 0, allocation);

		if (opal_result == OPAL_NO_MEMORY)
			opal_result = vulkan_allocatorAllocateMemory(allocator, vulkan_device, size, alignment, resource_type, memory_type, 1, allocation);

		if (opal_result == OPAL_SUCCESS)
			return OPAL_SUCCESS;

		memory_type_bits &= ~(1 << memory_type);
	}

	return OPAL_NO_MEMORY;
}

/*
 */
Opal_Result vulkan_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	assert(this);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Allocator *allocator = &device_ptr->allocator;
	VkDevice vulkan_device = device_ptr->device;
	VkPhysicalDevice vulkan_physical_device = device_ptr->physical_device;

	// create buffer
	VkBufferCreateInfo buffer_info = {0};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = desc->size;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_info.usage = vulkan_helperToBufferUsage(desc->usage);

	VkBuffer vulkan_buffer = VK_NULL_HANDLE;
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

	Vulkan_Allocation allocation = {0};
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

	// create opal struct
	Vulkan_Buffer *ptr = (Vulkan_Buffer *)malloc(sizeof(Vulkan_Buffer));
	assert(ptr);

	ptr->buffer = vulkan_buffer;
	ptr->allocation = allocation;
	ptr->size = allocation_desc.size;
	ptr->map_count = 0;

	*buffer = (Opal_Buffer)ptr;
	return OPAL_SUCCESS;
}

Opal_Result vulkan_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	assert(this);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Allocator *allocator = &device_ptr->allocator;
	VkDevice vulkan_device = device_ptr->device;
	VkPhysicalDevice vulkan_physical_device = device_ptr->physical_device;

	// create image
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

	VkImage vulkan_image = VK_NULL_HANDLE;
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
	Vulkan_Allocation allocation = {0};
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

	// create opal struct
	Vulkan_Image *ptr = (Vulkan_Image *)malloc(sizeof(Vulkan_Image));
	assert(ptr);

	ptr->image = vulkan_image;
	ptr->allocation = allocation;

	*texture = (Opal_Texture)ptr;
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

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)buffer;

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

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)buffer;
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

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)buffer;

	Vulkan_Allocator *allocator = &device_ptr->allocator;
	assert(allocator);

	vkDestroyBuffer(device_ptr->device, buffer_ptr->buffer, NULL);
	Opal_Result result = vulkan_allocatorFreeMemory(allocator, device_ptr->device, buffer_ptr->allocation);

	if (buffer_ptr->map_count > 0)
		vulkan_allocatorUnmapMemory(allocator, device_ptr->device, buffer_ptr->allocation);

	free(buffer_ptr);
	return result;
}

Opal_Result vulkan_deviceDestroyTexture(Device *this, Opal_Texture texture)
{
	assert(this);
	assert(texture != OPAL_NULL_HANDLE);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Image *image_ptr = (Vulkan_Image *)texture;

	Vulkan_Allocator *allocator = &device_ptr->allocator;
	assert(allocator);

	vkDestroyImage(device_ptr->device, image_ptr->image, NULL);
	Opal_Result result = vulkan_allocatorFreeMemory(allocator, device_ptr->device, image_ptr->allocation);

	free(image_ptr);
	return result;
}

Opal_Result vulkan_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view)
{
	return OPAL_NOT_SUPPORTED;
}
