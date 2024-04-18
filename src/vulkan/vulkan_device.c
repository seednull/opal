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
	Opal_Result result = vulkan_allocatorInitialize(&device_ptr->allocator, instance_ptr->heap_size, instance_ptr->max_heap_allocations, instance_ptr->max_heaps);
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
Opal_Result vulkan_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	assert(this);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
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

	VkBool32 prefers_dedicated = dedicated_requirements.prefersDedicatedAllocation;
	VkBool32 requires_dedicated = dedicated_requirements.requiresDedicatedAllocation;

	// memory type loop
	Vulkan_Allocator *allocator = &device_ptr->allocator;
	Vulkan_Allocation allocation = {0};
	Vulkan_AllocationDesc allocation_desc = {0};

	allocation_desc.size = memory_requirements.memoryRequirements.size;
	allocation_desc.alignment = memory_requirements.memoryRequirements.alignment;

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

	uint32_t memory_type_bits = memory_requirements.memoryRequirements.memoryTypeBits;

	while (memory_type_bits > 0)
	{
		Opal_Result opal_result = vulkan_helperFindBestMemoryType(vulkan_physical_device, memory_type_bits, desc->memory_type, &allocation_desc.memory_type);
		if (opal_result != OPAL_SUCCESS)
		{
			vkDestroyBuffer(vulkan_device, vulkan_buffer, NULL);
			return OPAL_NO_MEMORY;
		}

		// allocate memory
		opal_result = OPAL_NO_MEMORY;

		if (dedicated == VK_FALSE)
			opal_result = vulkan_allocatorAllocateMemory(allocator, vulkan_device, &allocation_desc, 0, &allocation);

		if (opal_result == OPAL_NO_MEMORY)
			opal_result = vulkan_allocatorAllocateMemory(allocator, vulkan_device, &allocation_desc, 1, &allocation);

		if (opal_result == OPAL_SUCCESS)
			break;

		memory_type_bits &= ~(1 << allocation_desc.memory_type);
	}

	// bind memory
	result = vkBindBufferMemory(vulkan_device, vulkan_buffer, allocation.memory, allocation.offset);
	if (result != VK_SUCCESS)
	{
		vkDestroyBuffer(vulkan_device, vulkan_buffer, NULL);
		vulkan_allocatorFreeMemory(allocator, allocation);
		return OPAL_VULKAN_ERROR;
	}

	// create opal struct
	Vulkan_Buffer *ptr = (Vulkan_Buffer *)malloc(sizeof(Vulkan_Buffer));
	assert(ptr);

	ptr->buffer = vulkan_buffer;
	ptr->allocation = allocation;
	ptr->size = allocation_desc.size;

	*buffer = (Opal_Instance)ptr;
	return OPAL_SUCCESS;
}

Opal_Result vulkan_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result vulkan_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result vulkan_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result vulkan_deviceUnmapBuffer(Device *this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
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
	return vulkan_allocatorFreeMemory(allocator, buffer_ptr->allocation);
}

Opal_Result vulkan_deviceDestroyTexture(Device *this, Opal_Texture texture)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result vulkan_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view)
{
	return OPAL_NOT_SUPPORTED;
}
