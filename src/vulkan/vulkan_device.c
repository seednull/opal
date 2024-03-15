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

	// allocators
	for (uint32_t i = 0; i < OPAL_BUFFER_HEAP_TYPE_MAX; ++i)
	{
		// TODO: there could be more heaps with the same memory properties, so more allocators is actually needed
		int32_t memory_type = vulkan_helperFindBestMemoryType(physical_device, i);
		assert(memory_type >= 0);

		Opal_Result result = vulkan_allocatorInitialize(&device_ptr->allocators[i], (uint32_t)memory_type, instance_ptr->heap_size, instance_ptr->max_heap_allocations, instance_ptr->max_heaps);
		assert(result == OPAL_SUCCESS);
	}

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

	for (uint32_t i = 0; i < OPAL_BUFFER_HEAP_TYPE_MAX; ++i)
	{
		Opal_Result result = vulkan_allocatorShutdown(&ptr->allocators[i], ptr->device);
		assert(result == OPAL_SUCCESS);
	}
	
	vkDestroyDevice(ptr->device, NULL);
	return OPAL_SUCCESS;
}

/*
 */
Opal_Result vulkan_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	assert(this);

	// get allocator
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	uint32_t allocator_index = desc->heap;

	Vulkan_Allocator *allocator = &device_ptr->allocators[allocator_index];
	Vulkan_Allocation allocation = {0};

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

	// allocate memory
	VkMemoryRequirements memory_requirements = {0};
	vkGetBufferMemoryRequirements(vulkan_device, vulkan_buffer, &memory_requirements);

	VkDeviceSize size = memory_requirements.size;
	VkDeviceSize alignment = memory_requirements.alignment;

	Opal_Result opal_result = vulkan_allocatorAllocateMemory(allocator, vulkan_device, size, alignment, &allocation);
	if (opal_result != OPAL_SUCCESS)
	{
		vkDestroyBuffer(vulkan_device, vulkan_buffer, NULL);
		vulkan_allocatorFreeMemory(allocator, allocation);
		return opal_result;
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
	ptr->allocator_index = allocator_index;
	ptr->size = size;

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

	uint32_t allocator_index = buffer_ptr->allocator_index;
	Vulkan_Allocator *allocator = &device_ptr->allocators[allocator_index];
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
