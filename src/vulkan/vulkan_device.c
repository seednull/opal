#include "vulkan_internal.h"
#include "common/intrinsics.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

/*
 */
static Opal_Result vulkan_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view);
static Opal_Result vulkan_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr);
static Opal_Result vulkan_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer);
static Opal_Result vulkan_deviceFreeDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set);
static Opal_Result vulkan_deviceUpdateDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set, uint32_t num_entries, const Opal_DescriptorSetEntry *entries);

/*
 */
static void vulkan_destroySemaphore(Vulkan_Device *device_ptr, Vulkan_Semaphore *semaphore_ptr)
{
	assert(device_ptr);
	assert(semaphore_ptr);

	device_ptr->vk.vkDestroySemaphore(device_ptr->device, semaphore_ptr->semaphore, NULL);
}

static void vulkan_destroyBuffer(Vulkan_Device *device_ptr, Vulkan_Buffer *buffer_ptr)
{
	assert(device_ptr);
	assert(buffer_ptr);

#if OPAL_HAS_VMA
	if (device_ptr->use_vma)
	{
		if (buffer_ptr->map_count > 0)
			vmaUnmapMemory(device_ptr->vma_allocator, buffer_ptr->vma_allocation);

		vmaDestroyBuffer(device_ptr->vma_allocator, buffer_ptr->buffer, buffer_ptr->vma_allocation);
		return;
	}
#endif

	device_ptr->vk.vkDestroyBuffer(device_ptr->device, buffer_ptr->buffer, NULL);

	if (buffer_ptr->map_count > 0)
		vulkan_allocatorUnmapMemory(device_ptr, buffer_ptr->allocation);

	vulkan_allocatorFreeMemory(device_ptr, buffer_ptr->allocation);
}

static void vulkan_destroyTexture(Vulkan_Device *device_ptr, Vulkan_Image *image_ptr)
{
	assert(device_ptr);
	assert(image_ptr);
 
#if OPAL_HAS_VMA
	if (device_ptr->use_vma)
	{
		vmaDestroyImage(device_ptr->vma_allocator, image_ptr->image, image_ptr->vma_allocation);
		return;
	}
#endif

	device_ptr->vk.vkDestroyImage(device_ptr->device, image_ptr->image, NULL);
	vulkan_allocatorFreeMemory(device_ptr, image_ptr->allocation);
}

static void vulkan_destroyTextureView(Vulkan_Device *device_ptr, Vulkan_ImageView *image_view_ptr)
{
	assert(device_ptr);
	assert(image_view_ptr);
 
	device_ptr->vk.vkDestroyImageView(device_ptr->device, image_view_ptr->image_view, NULL);
}

static void vulkan_destroySampler(Vulkan_Device *device_ptr, Vulkan_Sampler *sampler_ptr)
{
	assert(device_ptr);
	assert(sampler_ptr);
 
	device_ptr->vk.vkDestroySampler(device_ptr->device, sampler_ptr->sampler, NULL);
}

static void vulkan_destroyAccelerationStructure(Vulkan_Device *device_ptr, Vulkan_AccelerationStructure *acceleration_structure_ptr)
{
	assert(device_ptr);
	assert(acceleration_structure_ptr);
 
	device_ptr->vk.vkDestroyAccelerationStructureKHR(device_ptr->device, acceleration_structure_ptr->acceleration_structure, NULL);
	device_ptr->vk.vkDestroyQueryPool(device_ptr->device, acceleration_structure_ptr->size_pool, NULL);
	device_ptr->vk.vkDestroyQueryPool(device_ptr->device, acceleration_structure_ptr->serialization_size_pool, NULL);
	device_ptr->vk.vkDestroyQueryPool(device_ptr->device, acceleration_structure_ptr->compacted_size_pool, NULL);
}

static void vulkan_destroyCommandAllocator(Vulkan_Device *device_ptr, Vulkan_CommandAllocator *command_allocator_ptr)
{
	assert(device_ptr);
	assert(command_allocator_ptr);
 
	device_ptr->vk.vkDestroyCommandPool(device_ptr->device, command_allocator_ptr->pool, NULL);
}

static void vulkan_destroyShader(Vulkan_Device *device_ptr, Vulkan_Shader *shader_ptr)
{
	assert(device_ptr);
	assert(shader_ptr);
 
	device_ptr->vk.vkDestroyShaderModule(device_ptr->device, shader_ptr->shader, NULL);
}

static void vulkan_destroyDescriptorSetLayout(Vulkan_Device *device_ptr, Vulkan_DescriptorSetLayout *descriptor_set_layout_ptr)
{
	assert(device_ptr);
	assert(descriptor_set_layout_ptr);
 
	device_ptr->vk.vkDestroyDescriptorSetLayout(device_ptr->device, descriptor_set_layout_ptr->layout, NULL);
	free(descriptor_set_layout_ptr->entries);
	free(descriptor_set_layout_ptr->offsets);
}

static void vulkan_destroyDescriptorHeap(Vulkan_Device *device_ptr, Vulkan_DescriptorHeap *descriptor_heap_ptr)
{
	assert(device_ptr);
	assert(descriptor_heap_ptr);

	opal_heapShutdown(&descriptor_heap_ptr->heap);

#if OPAL_HAS_VMA
	if (device_ptr->use_vma)
	{
		vmaUnmapMemory(device_ptr->vma_allocator, buffer_ptr->vma_allocation);
		vmaDestroyBuffer(device_ptr->vma_allocator, descriptor_heap_ptr->buffer, descriptor_heap_ptr->vma_allocation);
		return;
	}
#endif

	device_ptr->vk.vkDestroyBuffer(device_ptr->device, descriptor_heap_ptr->buffer, NULL);
	vulkan_allocatorUnmapMemory(device_ptr, descriptor_heap_ptr->allocation);
	vulkan_allocatorFreeMemory(device_ptr, descriptor_heap_ptr->allocation);
}

static void vulkan_destroyPipelineLayout(Vulkan_Device *device_ptr, Vulkan_PipelineLayout *pipeline_layout_ptr)
{
	assert(device_ptr);
	assert(pipeline_layout_ptr);
 
	device_ptr->vk.vkDestroyPipelineLayout(device_ptr->device, pipeline_layout_ptr->layout, NULL);
}

static void vulkan_destroyPipeline(Vulkan_Device *device_ptr, Vulkan_Pipeline *pipeline_ptr)
{
	assert(device_ptr);
	assert(pipeline_ptr);
 
	device_ptr->vk.vkDestroyPipeline(device_ptr->device, pipeline_ptr->pipeline, NULL);
}

static void vulkan_destroySwapchain(Vulkan_Device *device_ptr, Vulkan_Swapchain *swapchain_ptr)
{
	assert(device_ptr);
	assert(swapchain_ptr);
 
	for (uint32_t i = 0; i < swapchain_ptr->num_images; ++i)
	{
		device_ptr->vk.vkDestroySemaphore(device_ptr->device, swapchain_ptr->acquire_semaphores[i], NULL);
		device_ptr->vk.vkDestroySemaphore(device_ptr->device, swapchain_ptr->present_semaphores[i], NULL);
	}

	free(swapchain_ptr->acquire_semaphores);
	free(swapchain_ptr->present_semaphores);
	free(swapchain_ptr->texture_views);

	device_ptr->vk.vkDestroySwapchainKHR(device_ptr->device, swapchain_ptr->swapchain, NULL);
}

/*
 */
static Opal_Result vulkan_deviceGetInfo(Opal_Device this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	Vulkan_Device *ptr = (Vulkan_Device *)this;
	return vulkan_helperFillDeviceInfo(ptr->physical_device, info);
}

static Opal_Result vulkan_deviceGetQueue(Opal_Device this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	assert(this);
	assert(queue);
	assert(engine_type < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	Vulkan_Device *ptr = (Vulkan_Device *)this;
	uint32_t queue_count = ptr->device_engines_info.queue_counts[engine_type];

	if (index >= queue_count)
		return OPAL_INVALID_QUEUE_INDEX;

	Opal_Queue *queue_handles = ptr->queue_handles[engine_type];
	assert(queue_handles);

	*queue = queue_handles[index];
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceGetAccelerationStructurePrebuildInfo(Opal_Device this, const Opal_AccelerationStructureBuildDesc *desc, Opal_AccelerationStructurePrebuildInfo *info)
{
	assert(this);
	assert(desc);
	assert(info);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	uint32_t num_entries = 1;
	if (desc->type == OPAL_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
		num_entries = desc->input.bottom_level.num_geometries;

	opal_bumpReset(&device_ptr->bump);
	uint32_t entries_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkAccelerationStructureGeometryKHR) * num_entries);
	uint32_t max_primive_counts_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(uint32_t) * num_entries);

	VkAccelerationStructureGeometryKHR *entries = (VkAccelerationStructureGeometryKHR *)(device_ptr->bump.data + entries_offset);
	memset(entries, 0, sizeof(VkAccelerationStructureGeometryKHR) * num_entries);

	uint32_t *max_primitive_counts = (uint32_t *)(device_ptr->bump.data + max_primive_counts_offset);
	memset(max_primitive_counts, 0, sizeof(uint32_t) * num_entries);

	if (desc->type == OPAL_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
	{
		for (uint32_t i = 0; i < num_entries; ++i)
		{
			const Opal_AccelerationStructureGeometry *opal_geometry = &desc->input.bottom_level.geometries[i];
			VkAccelerationStructureGeometryKHR *geometry = &entries[i];
			
			geometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			geometry->geometryType = vulkan_helperToAccelerationStructureGeometryType(opal_geometry->type);
			geometry->flags = vulkan_helperToAccelerationStructureGeometryFlags(opal_geometry->flags);

			if (opal_geometry->type == OPAL_ACCELERATION_STRUCTURE_GEOMETRY_TYPE_TRIANGLES)
			{
				const Opal_AccelerationStructureGeometryDataTriangles *opal_triangles = &opal_geometry->data.triangles;
				VkAccelerationStructureGeometryTrianglesDataKHR *triangles = &geometry->geometry.triangles;

				triangles->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
				triangles->vertexFormat = vulkan_helperToVertexFormat(opal_triangles->vertex_format);
				triangles->vertexStride = opal_triangles->vertex_stride;
				triangles->maxVertex = opal_triangles->num_vertices - 1;
				triangles->indexType = vulkan_helperToIndexType(opal_triangles->index_format);

				max_primitive_counts[i] = opal_triangles->num_vertices / 3;

				if (opal_triangles->index_buffer.buffer != OPAL_NULL_HANDLE)
					max_primitive_counts[i] = opal_triangles->num_indices / 3;

				if (opal_triangles->transform_buffer.buffer != OPAL_NULL_HANDLE)
					triangles->transformData.deviceAddress = 1; // Vulkan checks for NULL here, not for a valid address
			}
			else if (opal_geometry->type == OPAL_ACCELERATION_STRUCTURE_GEOMETRY_TYPE_AABBS)
			{
				const Opal_AccelerationStructureGeometryDataAABBs *opal_aabbs = &opal_geometry->data.aabbs;
				VkAccelerationStructureGeometryAabbsDataKHR *aabbs = &geometry->geometry.aabbs;

				aabbs->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
				aabbs->stride = opal_aabbs->stride;

				max_primitive_counts[i] = opal_aabbs->num_entries;
			}
			else
				assert(0);
		}
	}
	else if (desc->type == OPAL_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL)
	{
		VkAccelerationStructureGeometryKHR *geometry = &entries[0];
		const Opal_AccelerationStructureBuildInputTopLevel *input = &desc->input.top_level;

		geometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry->geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;

		VkAccelerationStructureGeometryInstancesDataKHR *instances = &geometry->geometry.instances;
		instances->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		instances->arrayOfPointers = VK_FALSE;

		max_primitive_counts[0] = input->num_instances;
		num_entries = 1;
	}
	else
		assert(0);

	VkAccelerationStructureBuildGeometryInfoKHR build_info = {0};
	build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	build_info.type = vulkan_helperToAccelerationStructureType(desc->type);
	build_info.flags = vulkan_helperToAccelerationStructureBuildFlags(desc->build_flags);
	build_info.mode = vulkan_helperToAccelerationStructureBuildMode(desc->build_mode);
	build_info.geometryCount = num_entries;
	build_info.pGeometries = entries;

	VkAccelerationStructureBuildSizesInfoKHR size_info = {0};
	size_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

	device_ptr->vk.vkGetAccelerationStructureBuildSizesKHR(vulkan_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, max_primitive_counts, &size_info);

	info->acceleration_structure_size = size_info.accelerationStructureSize;
	info->build_scratch_size = size_info.buildScratchSize;
	info->update_scratch_size = size_info.updateScratchSize;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceGetShaderBindingTablePrebuildInfo(Opal_Device this, const Opal_ShaderBindingTableBuildDesc *desc, Opal_ShaderBindingTablePrebuildInfo *info)
{
	assert(this);
	assert(desc);
	assert(info);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	uint32_t handle_size = device_ptr->raytrace_properties.shaderGroupHandleSize;
	uint32_t handle_alignment = device_ptr->raytrace_properties.shaderGroupHandleAlignment;
	uint32_t base_alignment = device_ptr->raytrace_properties.shaderGroupBaseAlignment;

	uint32_t aligned_handle_size = alignUp(handle_size, handle_alignment);

	uint32_t raygen_size = alignUp(desc->num_raygen_indices * aligned_handle_size, base_alignment);
	uint32_t hitgroup_size = alignUp(desc->num_hitgroup_indices * aligned_handle_size, base_alignment);

	// Note: we don't need to align miss part since the buffer offset could be either 0 or already aligned to base_alignment
	uint32_t miss_size = desc->num_miss_indices * aligned_handle_size;

	info->buffer_size = raygen_size + hitgroup_size + miss_size;

	info->base_raygen_offset = 0;
	info->base_hitgroup_offset = raygen_size;
	info->base_miss_offset = raygen_size + hitgroup_size;
	info->aligned_handle_size = aligned_handle_size;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceGetSupportedSurfaceFormats(Opal_Device this, Opal_Surface surface, uint32_t *num_formats, Opal_SurfaceFormat *formats)
{
	assert(this);
	assert(surface);
	assert(num_formats);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Instance *instance_ptr = device_ptr->instance;
	VkPhysicalDevice vulkan_physical_device = device_ptr->physical_device;

	// surface
	Vulkan_Surface *surface_ptr = (Vulkan_Surface *)opal_poolGetElement(&instance_ptr->surfaces, (Opal_PoolHandle)surface);
	assert(surface_ptr);

	VkSurfaceKHR vulkan_surface = surface_ptr->surface;

	uint32_t num_vulkan_formats = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_physical_device, vulkan_surface, &num_vulkan_formats, NULL);

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(VkSurfaceFormatKHR) * (*num_formats));
	VkSurfaceFormatKHR *vulkan_formats = (VkSurfaceFormatKHR *)(device_ptr->bump.data);

	vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_physical_device, vulkan_surface, &num_vulkan_formats, vulkan_formats);

	static VkFormat allowed_formats[] =
	{
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_FORMAT_B8G8R8A8_SRGB,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		// TODO: add rgb10a2 support
	};
	static uint32_t num_allowed_formats = sizeof(allowed_formats) / sizeof(VkFormat);

	static VkColorSpaceKHR allowed_color_spaces[] =
	{
		VK_COLORSPACE_SRGB_NONLINEAR_KHR,
		// TODO: add more color spaces
		// TODO: add hdr10 color space
	};
	static uint32_t num_allowed_color_spaces = sizeof(allowed_color_spaces) / sizeof(VkColorSpaceKHR);

	uint32_t num_supported_formats = 0;
	for (uint32_t i = 0; i < num_vulkan_formats; ++i)
	{
		VkFormat format = vulkan_formats[i].format;
		VkColorSpaceKHR color_space = vulkan_formats[i].colorSpace;

		VkBool32 format_allowed = VK_FALSE;
		for (uint32_t j = 0; j < num_allowed_formats; ++j)
		{
			if (allowed_formats[j] != format)
				continue;

			format_allowed = VK_TRUE;
			break; 
		}

		if (format_allowed == VK_FALSE)
			continue;

		VkBool32 color_space_allowed = VK_FALSE;
		for (uint32_t j = 0; j < num_allowed_color_spaces; ++j)
		{
			if (allowed_color_spaces[j] != color_space)
				continue;

			color_space_allowed = VK_TRUE;
			break;
		}
		
		if (color_space_allowed == VK_FALSE)
			continue;

		if (formats)
		{
			formats[num_supported_formats].texture_format = vulkan_helperFromImageFormat(format);
			formats[num_supported_formats].color_space = vulkan_helperFromColorSpace(color_space);
		}

		num_supported_formats++;
	}

	*num_formats = num_supported_formats;
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceGetSupportedPresentModes(Opal_Device this, Opal_Surface surface, uint32_t *num_present_modes, Opal_PresentMode *present_modes)
{
	assert(this);
	assert(surface);
	assert(num_present_modes);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Instance *instance_ptr = device_ptr->instance;
	VkPhysicalDevice vulkan_physical_device = device_ptr->physical_device;

	// surface
	Vulkan_Surface *surface_ptr = (Vulkan_Surface *)opal_poolGetElement(&instance_ptr->surfaces, (Opal_PoolHandle)surface);
	assert(surface_ptr);

	VkSurfaceKHR vulkan_surface = surface_ptr->surface;

	uint32_t num_vulkan_modes = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_physical_device, vulkan_surface, &num_vulkan_modes, NULL);

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(VkPresentModeKHR) * (*num_present_modes));
	VkPresentModeKHR *vulkan_modes = (VkPresentModeKHR *)(device_ptr->bump.data);

	vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_physical_device, vulkan_surface, &num_vulkan_modes, vulkan_modes);

	static VkPresentModeKHR allowed_present_modes[] =
	{
		VK_PRESENT_MODE_IMMEDIATE_KHR,
		VK_PRESENT_MODE_MAILBOX_KHR,
		VK_PRESENT_MODE_FIFO_KHR,
	};
	static uint32_t num_allowed_present_modes = sizeof(allowed_present_modes) / sizeof(VkPresentModeKHR);

	uint32_t num_supported_modes = 0;
	for (uint32_t i = 0; i < num_vulkan_modes; ++i)
	{
		VkPresentModeKHR mode = vulkan_modes[i];

		VkBool32 mode_allowed = VK_FALSE;
		for (uint32_t j = 0; j < num_allowed_present_modes; ++j)
		{
			if (allowed_present_modes[j] != mode)
				continue;

			mode_allowed = VK_TRUE;
			break;
		}

		if (mode_allowed == VK_FALSE)
			continue;

		if (present_modes)
			present_modes[num_supported_modes] = vulkan_helperFromPresentMode(mode);

		num_supported_modes++;
	}

	*num_present_modes = num_supported_modes;
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceGetPreferredSurfaceFormat(Opal_Device this, Opal_Surface surface, Opal_SurfaceFormat *format)
{
	assert(this);
	assert(surface);
	assert(format);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	uint32_t num_formats = 0;
	vulkan_deviceGetSupportedSurfaceFormats(this, surface, &num_formats, NULL);

	if (num_formats == 0)
		return OPAL_SURFACE_NOT_DRAWABLE;

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(Opal_SurfaceFormat) * num_formats);
	Opal_SurfaceFormat *formats = (Opal_SurfaceFormat *)(device_ptr->bump.data);

	vulkan_deviceGetSupportedSurfaceFormats(this, surface, &num_formats, formats);

	Opal_TextureFormat optimal_format = OPAL_TEXTURE_FORMAT_BGRA8_UNORM;
	Opal_ColorSpace optimal_color_space = OPAL_COLOR_SPACE_SRGB;

	*format = formats[0];
	for (uint32_t i = 0; i < num_formats; ++i)
	{
		if (formats[i].texture_format == optimal_format && formats[i].color_space == optimal_color_space)
		{
			format->texture_format = optimal_format;
			format->color_space = optimal_color_space;
			break;
		}
	}

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceGetPreferredSurfacePresentMode(Opal_Device this, Opal_Surface surface, Opal_PresentMode *present_mode)
{
	assert(this);
	assert(surface);
	assert(present_mode);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	uint32_t num_present_modes = 0;
	vulkan_deviceGetSupportedPresentModes(this, surface, &num_present_modes, NULL);

	if (num_present_modes == 0)
		return OPAL_SURFACE_NOT_PRESENTABLE;

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(Opal_PresentMode) * num_present_modes);
	Opal_PresentMode *present_modes = (Opal_PresentMode *)(device_ptr->bump.data);

	vulkan_deviceGetSupportedPresentModes(this, surface, &num_present_modes, present_modes);

	// TODO: it's probably a good idea to search for mailbox for high performance device,
	//       but for low power device fifo will drain less battery by adding latency
	Opal_PresentMode optimal_present_mode = OPAL_PRESENT_MODE_MAILBOX;

	*present_mode = present_modes[0];
	for (uint32_t i = 0; i < num_present_modes; ++i)
	{
		if (present_modes[i] == optimal_present_mode)
		{
			*present_mode = optimal_present_mode;
			break;
		}
	}

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateSemaphore(Opal_Device this, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore)
{
	assert(this);
	assert(desc);
	assert(semaphore);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	VkSemaphore vulkan_semaphore = VK_NULL_HANDLE;

	VkSemaphoreTypeCreateInfoKHR semaphore_type_info = {0};
	semaphore_type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
	semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
	semaphore_type_info.initialValue = desc->initial_value;

	VkSemaphoreCreateInfo semaphore_info = {0};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = &semaphore_type_info;

	VkResult vulkan_result = device_ptr->vk.vkCreateSemaphore(vulkan_device, &semaphore_info, NULL, &vulkan_semaphore);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_Semaphore result = {0};
	result.semaphore = vulkan_semaphore;

	*semaphore = (Opal_Semaphore)opal_poolAddElement(&device_ptr->semaphores, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateBuffer(Opal_Device this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
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

	VkBuffer vulkan_buffer = VK_NULL_HANDLE;

	VkBufferCreateInfo buffer_info = {0};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = desc->size;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_info.usage = vulkan_helperToBufferUsage(desc->usage);
	
	// TODO: ideally, we should check buffer_device_address feature availability and skip this if it's not present
	buffer_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;

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
		// create buffer
		VkResult result = device_ptr->vk.vkCreateBuffer(vulkan_device, &buffer_info, NULL, &vulkan_buffer);
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
		device_ptr->vk.vkGetBufferMemoryRequirements2(vulkan_device, &memory_info, &memory_requirements);

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

		Opal_Result opal_result = vulkan_deviceAllocateMemory(device_ptr, &allocation_desc, &allocation);
		if (opal_result != OPAL_SUCCESS)
		{
			device_ptr->vk.vkDestroyBuffer(vulkan_device, vulkan_buffer, NULL);
			return opal_result;
		}

		// bind memory
		result = device_ptr->vk.vkBindBufferMemory(vulkan_device, vulkan_buffer, allocation.memory, allocation.offset);
		if (result != VK_SUCCESS)
		{
			device_ptr->vk.vkDestroyBuffer(vulkan_device, vulkan_buffer, NULL);
			vulkan_allocatorFreeMemory(device_ptr, allocation);
			return OPAL_VULKAN_ERROR;
		}
	}

	// TODO: ideally, we should check buffer_device_address feature availability and skip this if it's not present
	VkBufferDeviceAddressInfoKHR buffer_address_info = {0};
	buffer_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
	buffer_address_info.buffer = vulkan_buffer;

	VkDeviceAddress device_address = device_ptr->vk.vkGetBufferDeviceAddressKHR(vulkan_device, &buffer_address_info);

	// create opal struct
	Vulkan_Buffer result = {0};
	result.buffer = vulkan_buffer;
#if OPAL_HAS_VMA
	result.vma_allocation = vma_allocation;
#endif
	result.allocation = allocation;
	result.device_address = device_address;

	*buffer = (Opal_Buffer)opal_poolAddElement(&device_ptr->buffers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	assert(this);
	assert(desc);
	assert(texture);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

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
	image_info.samples = vulkan_helperToSamples(desc->samples);
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
		// create image
		VkResult result = device_ptr->vk.vkCreateImage(vulkan_device, &image_info, NULL, &vulkan_image);
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
		device_ptr->vk.vkGetImageMemoryRequirements2(vulkan_device, &memory_info, &memory_requirements);

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

		Opal_Result opal_result = vulkan_deviceAllocateMemory(device_ptr, &allocation_desc, &allocation);
		if (opal_result != OPAL_SUCCESS)
		{
			device_ptr->vk.vkDestroyImage(vulkan_device, vulkan_image, NULL);
			return opal_result;
		}

		// bind memory
		result = device_ptr->vk.vkBindImageMemory(vulkan_device, vulkan_image, allocation.memory, allocation.offset);
		if (result != VK_SUCCESS)
		{
			device_ptr->vk.vkDestroyImage(vulkan_device, vulkan_image, NULL);
			vulkan_allocatorFreeMemory(device_ptr, allocation);
			return OPAL_VULKAN_ERROR;
		}
	}

	// create opal struct
	Vulkan_Image result = {0};
	result.image = vulkan_image;
	result.format = desc->format;
	result.width = desc->width;
	result.height = desc->height;
	result.depth = desc->depth;
	result.aspect_mask = vulkan_helperToImageAspectMask(desc->format);
#if OPAL_HAS_VMA
	result.vma_allocation = vma_allocation;
#endif
	result.allocation = allocation;

	*texture = (Opal_Texture)opal_poolAddElement(&device_ptr->images, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateTextureView(Opal_Device this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	assert(this);
	assert(desc);
	assert(texture_view);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	Vulkan_Image *image_ptr = (Vulkan_Image *)opal_poolGetElement(&device_ptr->images, (Opal_PoolHandle)desc->texture);
	assert(image_ptr);

	VkImageView vulkan_image_view = VK_NULL_HANDLE;

	VkImageViewCreateInfo image_view_info = {0};
	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.image = image_ptr->image;
	image_view_info.viewType = vulkan_helperToImageViewType(desc->type);
	image_view_info.format = vulkan_helperToImageFormat(image_ptr->format);
	image_view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	image_view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	image_view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	image_view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	image_view_info.subresourceRange.aspectMask = vulkan_helperToImageAspectMask(image_ptr->format);
	image_view_info.subresourceRange.baseArrayLayer = desc->base_layer;
	image_view_info.subresourceRange.layerCount = desc->layer_count;
	image_view_info.subresourceRange.baseMipLevel = desc->base_mip;
	image_view_info.subresourceRange.levelCount = desc->mip_count;

	VkResult vulkan_result = device_ptr->vk.vkCreateImageView(vulkan_device, &image_view_info, NULL, &vulkan_image_view);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_ImageView result = {0};
	result.image_view = vulkan_image_view;
	result.image = image_ptr->image;

	// FIXME: base_mip should log2 sizes
	result.width = image_ptr->width;
	result.height = image_ptr->height;
	result.depth = image_ptr->depth;

	result.base_mip = desc->base_mip;
	result.num_mips = desc->mip_count;
	result.base_layer = desc->base_layer;
	result.num_layers = desc->layer_count;
	result.aspect_mask = image_view_info.subresourceRange.aspectMask;

	*texture_view = (Opal_TextureView)opal_poolAddElement(&device_ptr->image_views, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateSampler(Opal_Device this, const Opal_SamplerDesc *desc, Opal_Sampler *sampler)
{
	assert(this);
	assert(desc);
	assert(sampler);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	VkSampler vulkan_sampler = VK_NULL_HANDLE;

	VkSamplerCreateInfo sampler_info = {0};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = vulkan_helperToFilter(desc->mag_filter);
	sampler_info.minFilter = vulkan_helperToFilter(desc->min_filter);
	sampler_info.mipmapMode = vulkan_helperToSamplerMipmapMode(desc->mip_filter);
	sampler_info.addressModeU = vulkan_helperToSamplerAddressMode(desc->address_mode_u);
	sampler_info.addressModeV = vulkan_helperToSamplerAddressMode(desc->address_mode_v);
	sampler_info.addressModeW = vulkan_helperToSamplerAddressMode(desc->address_mode_w);
	sampler_info.anisotropyEnable = desc->max_anisotropy > 0;
	sampler_info.maxAnisotropy = (float)desc->max_anisotropy;
	sampler_info.compareEnable = desc->compare_enable;
	sampler_info.compareOp = vulkan_helperToCompareOp(desc->compare_op);
	sampler_info.minLod = desc->min_lod;
	sampler_info.maxLod = desc->max_lod;

	VkResult vulkan_result = device_ptr->vk.vkCreateSampler(vulkan_device, &sampler_info, NULL, &vulkan_sampler);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_Sampler result = {0};
	result.sampler = vulkan_sampler;

	*sampler = (Opal_Sampler)opal_poolAddElement(&device_ptr->samplers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateAccelerationStructure(Opal_Device this, const Opal_AccelerationStructureDesc *desc, Opal_AccelerationStructure *acceleration_structure)
{
	assert(this);
	assert(desc);
	assert(acceleration_structure);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	Vulkan_Buffer *buffer_ptr = opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)desc->buffer.buffer);
	assert(buffer_ptr);

	VkAccelerationStructureKHR vulkan_acceleration_structure = VK_NULL_HANDLE;

	VkAccelerationStructureCreateInfoKHR acceleration_structure_info = {0};
	acceleration_structure_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	acceleration_structure_info.buffer = buffer_ptr->buffer;
	acceleration_structure_info.offset = desc->buffer.offset;
	acceleration_structure_info.size = desc->buffer.size;
	acceleration_structure_info.type = vulkan_helperToAccelerationStructureType(desc->type);

	VkResult vulkan_result = device_ptr->vk.vkCreateAccelerationStructureKHR(vulkan_device, &acceleration_structure_info, NULL, &vulkan_acceleration_structure);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	static VkQueryType query_types[] =
	{
		VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR,
		VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR,
		VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
	};

	VkQueryPool query_pools[] = {VK_NULL_HANDLE,VK_NULL_HANDLE,VK_NULL_HANDLE};

	VkBool32 success = VK_TRUE;
	for (uint32_t i = 0; i < 3; ++i)
	{
		VkQueryPoolCreateInfo query_info = {0};
		query_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		query_info.queryType = query_types[i];
		query_info.queryCount = 1;

		vulkan_result = device_ptr->vk.vkCreateQueryPool(vulkan_device, &query_info, NULL, &query_pools[i]);
		if (vulkan_result != VK_SUCCESS)
		{
			success = VK_FALSE;
			break;
		}
	}

	if (success != VK_TRUE)
	{
		for (uint32_t i = 0; i < 3; ++i)
			device_ptr->vk.vkDestroyQueryPool(vulkan_device, query_pools[i], NULL);

		device_ptr->vk.vkDestroyAccelerationStructureKHR(vulkan_device, vulkan_acceleration_structure, NULL);
		return OPAL_VULKAN_ERROR;
	}

	VkAccelerationStructureDeviceAddressInfoKHR address_info = {0};
	address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	address_info.accelerationStructure = vulkan_acceleration_structure;
	
	Vulkan_AccelerationStructure result = {0};
	result.acceleration_structure = vulkan_acceleration_structure;
	result.device_address = device_ptr->vk.vkGetAccelerationStructureDeviceAddressKHR(vulkan_device, &address_info);
	result.size_pool = query_pools[0];
	result.serialization_size_pool = query_pools[1];
	result.compacted_size_pool = query_pools[2];

	*acceleration_structure = (Opal_Sampler)opal_poolAddElement(&device_ptr->acceleration_structures, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateCommandAllocator(Opal_Device this, Opal_Queue queue, Opal_CommandAllocator *command_allocator)
{
	assert(this);
	assert(command_allocator);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	Vulkan_Queue *queue_ptr = (Vulkan_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queue);
	assert(queue_ptr);

	VkCommandPool vulkan_command_pool = VK_NULL_HANDLE;

	VkCommandPoolCreateInfo command_pool_info = {0};
	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.queueFamilyIndex = queue_ptr->family_index;

	VkResult vulkan_result = device_ptr->vk.vkCreateCommandPool(vulkan_device, &command_pool_info, NULL, &vulkan_command_pool);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_CommandAllocator result = {0};
	result.pool = vulkan_command_pool;

	*command_allocator = (Opal_CommandAllocator)opal_poolAddElement(&device_ptr->command_allocators, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateCommandBuffer(Opal_Device this, Opal_CommandAllocator command_allcoator, Opal_CommandBuffer *command_buffer)
{
	assert(this);
	assert(command_allcoator);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;
	VkCommandBuffer vulkan_command_buffer = VK_NULL_HANDLE;

	Vulkan_CommandAllocator *command_allocator_ptr = (Vulkan_CommandAllocator *)opal_poolGetElement(&device_ptr->command_allocators, (Opal_PoolHandle)command_allcoator);
	assert(command_allocator_ptr);

	VkCommandBufferAllocateInfo command_buffer_info = {0};
	command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_info.commandPool = command_allocator_ptr->pool;
	command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_info.commandBufferCount = 1;

	VkResult vulkan_result = device_ptr->vk.vkAllocateCommandBuffers(vulkan_device, &command_buffer_info, &vulkan_command_buffer);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_CommandBuffer result = {0};
	result.command_buffer = vulkan_command_buffer;
	result.pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;

	// TODO: add handle to Vulkan_CommandAllocator instance

	*command_buffer = (Opal_CommandBuffer)opal_poolAddElement(&device_ptr->command_buffers, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateShader(Opal_Device this, const Opal_ShaderDesc *desc, Opal_Shader *shader)
{
	assert(this);
	assert(desc);
	assert(shader);
	assert(desc->type == OPAL_SHADER_SOURCE_TYPE_SPIRV_BINARY);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	VkShaderModule vulkan_shader = VK_NULL_HANDLE;

	VkShaderModuleCreateInfo shader_info = {0};
	shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_info.pCode = desc->data;
	shader_info.codeSize = desc->size;

	VkResult vulkan_result = device_ptr->vk.vkCreateShaderModule(vulkan_device, &shader_info, NULL, &vulkan_shader);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_Shader result = {0};
	result.shader = vulkan_shader;

	*shader = (Opal_Shader)opal_poolAddElement(&device_ptr->shaders, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateDescriptorHeap(Opal_Device this, const Opal_DescriptorHeapDesc *desc, Opal_DescriptorHeap *descriptor_heap)
{
	assert(this);
	assert(desc);
	assert(descriptor_heap);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	VkBuffer vulkan_buffer = VK_NULL_HANDLE;

	uint32_t num_descriptors = desc->num_resource_descriptors + desc->num_sampler_descriptors;
	// TODO: check current buffer size agains samplerDescriptorBufferAddressSpaceSize or resourceDescriptorBufferAddressSpaceSize
	//       depending on descriptor heap type

	VkBufferCreateInfo buffer_info = {0};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = alignUpul(num_descriptors * device_ptr->max_descriptor_size, device_ptr->descriptor_buffer_properties.descriptorBufferOffsetAlignment);
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_info.usage = 0;

	if (desc->num_resource_descriptors > 0)
		buffer_info.usage |= VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

	if (desc->num_sampler_descriptors > 0)
		buffer_info.usage |= VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

	// TODO: ideally, we should check buffer_device_address feature availability and skip this if it's not present
	buffer_info.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;

	Vulkan_Allocation allocation = {0};
	uint8_t *buffer_ptr = NULL;

#ifdef OPAL_HAS_VMA
	VmaAllocation vma_allocation = VK_NULL_HANDLE;

	if (device_ptr->use_vma > 0)
	{
		VmaAllocationCreateInfo allocation_info = {0};
		allocation_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		allocation_info.preferredFlags = ~(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
		allocation_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		VkResult result = vmaCreateBuffer(device_ptr->vma_allocator, &buffer_info, &allocation_info, &vulkan_buffer, &vma_allocation, NULL);
		if (result != VK_SUCCESS)
			return OPAL_VULKAN_ERROR;

		buffer_ptr = vma_allocation.pMappedData;
	}
	else
#endif
	{
		// create buffer
		VkResult result = device_ptr->vk.vkCreateBuffer(vulkan_device, &buffer_info, NULL, &vulkan_buffer);
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
		device_ptr->vk.vkGetBufferMemoryRequirements2(vulkan_device, &memory_info, &memory_requirements);

		// fill allocation info
		Vulkan_AllocationDesc allocation_desc = {0};
		allocation_desc.size = memory_requirements.memoryRequirements.size;
		allocation_desc.alignment = memory_requirements.memoryRequirements.alignment;
		allocation_desc.memory_type_bits = memory_requirements.memoryRequirements.memoryTypeBits;
		allocation_desc.required_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		allocation_desc.preferred_flags = 0;
		allocation_desc.not_preferred_flags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		allocation_desc.resource_type = VULKAN_RESOURCE_TYPE_LINEAR;
		allocation_desc.hint = OPAL_ALLOCATION_HINT_AUTO;
		allocation_desc.prefers_dedicated = dedicated_requirements.prefersDedicatedAllocation;
		allocation_desc.requires_dedicated = dedicated_requirements.requiresDedicatedAllocation;

		Opal_Result opal_result = vulkan_deviceAllocateMemory(device_ptr, &allocation_desc, &allocation);
		if (opal_result != OPAL_SUCCESS)
		{
			device_ptr->vk.vkDestroyBuffer(vulkan_device, vulkan_buffer, NULL);
			return opal_result;
		}

		// bind memory
		result = device_ptr->vk.vkBindBufferMemory(vulkan_device, vulkan_buffer, allocation.memory, allocation.offset);
		if (result != VK_SUCCESS)
		{
			device_ptr->vk.vkDestroyBuffer(vulkan_device, vulkan_buffer, NULL);
			vulkan_allocatorFreeMemory(device_ptr, allocation);
			return OPAL_VULKAN_ERROR;
		}

		// map memory
		uint8_t *block_ptr = NULL;
		opal_result = vulkan_allocatorMapMemory(device_ptr, allocation, &block_ptr);
		if (opal_result != OPAL_SUCCESS)
		{
			device_ptr->vk.vkDestroyBuffer(vulkan_device, vulkan_buffer, NULL);
			vulkan_allocatorFreeMemory(device_ptr, allocation);
			return opal_result;
		}

		buffer_ptr = block_ptr + allocation.offset;
	}

	// TODO: ideally, we should check buffer_device_address feature availability and skip this if it's not present
	VkBufferDeviceAddressInfoKHR buffer_address_info = {0};
	buffer_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
	buffer_address_info.buffer = vulkan_buffer;

	VkDeviceAddress device_address = device_ptr->vk.vkGetBufferDeviceAddressKHR(vulkan_device, &buffer_address_info);

	// create opal struct
	Vulkan_DescriptorHeap result = {0};
	result.buffer = vulkan_buffer;
	result.usage = buffer_info.usage;
	result.buffer_ptr = buffer_ptr;
#if OPAL_HAS_VMA
	result.vma_allocation = vma_allocation;
#endif
	result.allocation = allocation;
	result.device_address = device_address;

	uint32_t num_blocks = (uint32_t)(buffer_info.size / device_ptr->descriptor_buffer_properties.descriptorBufferOffsetAlignment);
	Opal_Result opal_result = opal_heapInitialize(&result.heap, num_blocks, num_blocks);
	assert(opal_result == OPAL_SUCCESS);

	*descriptor_heap = (Opal_DescriptorHeap)opal_poolAddElement(&device_ptr->descriptor_heaps, &result);
	return opal_result;
}

static Opal_Result vulkan_deviceCreateDescriptorSetLayout(Opal_Device this, uint32_t num_entries, const Opal_DescriptorSetLayoutEntry *entries, Opal_DescriptorSetLayout *descriptor_set_layout)
{
	assert(this);
	assert(num_entries == 0 || entries);
	assert(descriptor_set_layout);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	VkDescriptorSetLayout vulkan_set_layout = VK_NULL_HANDLE;

	VkDescriptorSetLayoutCreateInfo set_layout_info = {0};
	set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

	uint32_t num_static_entries = 0;
	uint32_t num_dynamic_entries = 0;

	for (uint32_t i = 0; i < num_entries; ++i)
	{
		const Opal_DescriptorSetLayoutEntry *entry = &entries[i];

		switch (entry->type)
		{
			case OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC:
			{
				num_dynamic_entries++;
			}
			break;

			default:
			{
				num_static_entries++;
			}
			break;
		}
	}

	if (num_dynamic_entries > 0)
		set_layout_info.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

	assert(num_static_entries + num_dynamic_entries == num_entries);

	uint32_t static_entry_index = 0;
	uint32_t dynamic_entry_index = 0;

	VkDescriptorSetLayoutBinding *vulkan_entries = NULL;
	if (num_entries > 0)
	{
		vulkan_entries = (VkDescriptorSetLayoutBinding *)malloc(sizeof(VkDescriptorSetLayoutBinding) * num_entries);
		memset(vulkan_entries, 0, sizeof(VkDescriptorSetLayoutBinding) * num_entries);
	}

	for (uint32_t i = 0; i < num_entries; ++i)
	{
		const Opal_DescriptorSetLayoutEntry *entry = &entries[i];
		VkDescriptorSetLayoutBinding *vulkan_entry = NULL;
		switch (entry->type)
		{
			case OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			case OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC:
			{
				vulkan_entry = &vulkan_entries[num_static_entries + dynamic_entry_index];
				dynamic_entry_index++;
			}
			break;

			default:
			{
				vulkan_entry = &vulkan_entries[static_entry_index++];
			}
			break;
		}

		vulkan_entry->binding = entry->binding;
		vulkan_entry->descriptorCount = 1;
		vulkan_entry->descriptorType = vulkan_helperToDescriptorType(entry->type);
		vulkan_entry->pImmutableSamplers = NULL;
		vulkan_entry->stageFlags = vulkan_helperToShaderStageFlags(entry->visibility);
	}

	assert(static_entry_index == num_static_entries);
	assert(dynamic_entry_index == num_dynamic_entries);

	set_layout_info.pBindings = vulkan_entries;
	set_layout_info.bindingCount = num_entries;

	VkResult vulkan_result = device_ptr->vk.vkCreateDescriptorSetLayout(vulkan_device, &set_layout_info, NULL, &vulkan_set_layout);

	if (vulkan_result != VK_SUCCESS)
	{
		free(vulkan_entries);
		return OPAL_VULKAN_ERROR;
	}

	VkDeviceSize *vulkan_offsets = NULL;
	if (num_entries > 0)
	{
		assert(vulkan_entries);
		vulkan_offsets = (VkDeviceSize *)malloc(sizeof(VkDeviceSize) * num_entries);
	}

	for (uint32_t i = 0; i < num_entries; ++i)
		device_ptr->vk.vkGetDescriptorSetLayoutBindingOffsetEXT(vulkan_device, vulkan_set_layout, vulkan_entries[i].binding, &vulkan_offsets[i]);

	VkDeviceSize size = 0;
	uint32_t num_blocks = 0;

	if (num_entries > 0)
	{
		device_ptr->vk.vkGetDescriptorSetLayoutSizeEXT(vulkan_device, vulkan_set_layout, &size);

		VkDeviceSize alignment = device_ptr->descriptor_buffer_properties.descriptorBufferOffsetAlignment;
		num_blocks = (uint32_t)(alignUpul(size, alignment) / alignment);
	}

	Vulkan_DescriptorSetLayout result = {0};
	result.layout = vulkan_set_layout;
	result.num_blocks = num_blocks;
	result.num_entries = num_entries;
	result.entries = vulkan_entries;
	result.offsets = vulkan_offsets;
	result.num_static_descriptors = num_static_entries;
	result.num_dynamic_descriptors = num_dynamic_entries;

	*descriptor_set_layout = (Opal_DescriptorSetLayout)opal_poolAddElement(&device_ptr->descriptor_set_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_descriptor_set_layouts, const Opal_DescriptorSetLayout *descriptor_set_layouts, Opal_PipelineLayout *pipeline_layout)
{
	assert(this);
	assert(pipeline_layout);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	VkDescriptorSetLayout *set_layouts = NULL;

	uint32_t num_dynamic_descriptors = 0;
	if (num_descriptor_set_layouts > 0)
	{
		assert(descriptor_set_layouts);

		opal_bumpReset(&device_ptr->bump);
		opal_bumpAlloc(&device_ptr->bump, sizeof(VkDescriptorSetLayout) * num_descriptor_set_layouts);
		set_layouts = (VkDescriptorSetLayout *)device_ptr->bump.data;

		for (uint32_t i = 0; i < num_descriptor_set_layouts; ++i)
		{
			Vulkan_DescriptorSetLayout *descriptor_set_layout_ptr = (Vulkan_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_layouts[i]);
			assert(descriptor_set_layout_ptr);

			set_layouts[i] = descriptor_set_layout_ptr->layout;
			num_dynamic_descriptors += descriptor_set_layout_ptr->num_dynamic_descriptors;
		}
	}

	VkPipelineLayout vulkan_pipeline_layout = VK_NULL_HANDLE;

	VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = num_descriptor_set_layouts;
	pipeline_layout_info.pSetLayouts = set_layouts;

	VkResult vulkan_result = device_ptr->vk.vkCreatePipelineLayout(vulkan_device, &pipeline_layout_info, NULL, &vulkan_pipeline_layout);

	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_PipelineLayout result = {0};
	result.layout = vulkan_pipeline_layout;
	result.num_dynamic_descriptors = num_dynamic_descriptors;

	*pipeline_layout = (Opal_PipelineLayout)opal_poolAddElement(&device_ptr->pipeline_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	assert(this);
	assert(desc);
	assert(desc->vertex_function.shader);
	assert(desc->vertex_function.name);
	assert(desc->fragment_function.shader);
	assert(desc->fragment_function.name);
	assert(desc->pipeline_layout);
	assert(pipeline);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	// pipeline layout
	Vulkan_PipelineLayout *pipeline_layout_ptr = (Vulkan_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)desc->pipeline_layout);
	assert(pipeline_layout_ptr);

	VkPipelineLayout vulkan_pipeline_layout = pipeline_layout_ptr->layout;
	VkPipeline vulkan_pipeline = VK_NULL_HANDLE;
	VkPipelineCache vulkan_pipeline_cache = VK_NULL_HANDLE;

	// shaders
	Opal_Shader shaders[5] =
	{
		desc->vertex_function.shader,
		desc->tessellation_control_function.shader,
		desc->tessellation_evaluation_function.shader,
		desc->geometry_function.shader,
		desc->fragment_function.shader,
	};

	const char *names[5] =
	{
		desc->vertex_function.name,
		desc->tessellation_control_function.name,
		desc->tessellation_evaluation_function.name,
		desc->geometry_function.name,
		desc->fragment_function.name,
	};

	VkShaderStageFlagBits shader_stages[5] =
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		VK_SHADER_STAGE_GEOMETRY_BIT,
		VK_SHADER_STAGE_FRAGMENT_BIT,
	};

	VkPipelineShaderStageCreateInfo shader_infos[5];
	uint32_t num_shaders = 0;

	for (uint32_t i = 0; i < 5; ++i)
	{
		Vulkan_Shader *shader_ptr = (Vulkan_Shader *)opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)shaders[i]);
		if (shader_ptr == NULL)
			continue;

		shader_infos[num_shaders].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_infos[num_shaders].pNext = NULL;
		shader_infos[num_shaders].flags = 0;
		shader_infos[num_shaders].stage = shader_stages[i];
		shader_infos[num_shaders].module = shader_ptr->shader;
		shader_infos[num_shaders].pName = names[i];
		shader_infos[num_shaders].pSpecializationInfo = NULL;
		num_shaders++;
	}

	// vertex input state
	VkPipelineVertexInputStateCreateInfo vertex_input = {0};
	vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	uint32_t num_vertex_attributes = 0;
	for (uint32_t i = 0; i < desc->num_vertex_streams; ++i)
		num_vertex_attributes += desc->vertex_streams[i].num_vertex_attributes;

	opal_bumpReset(&device_ptr->bump);
	uint32_t vertex_streams_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkVertexInputBindingDescription) * desc->num_vertex_streams);
	uint32_t vertex_attributes_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkVertexInputAttributeDescription) * num_vertex_attributes);

	VkVertexInputBindingDescription *vertex_streams = (VkVertexInputBindingDescription *)(device_ptr->bump.data + vertex_streams_offset);
	VkVertexInputAttributeDescription *vertex_attributes = (VkVertexInputAttributeDescription *)(device_ptr->bump.data + vertex_attributes_offset);

	uint32_t num_attributes = 0;
	for (uint32_t i = 0; i < desc->num_vertex_streams; ++i)
	{
		const Opal_VertexStream *vertex_stream = &desc->vertex_streams[i];
		for (uint32_t j = 0; j < vertex_stream->num_vertex_attributes; ++j)
		{
			const Opal_VertexAttribute *vertex_attribute = &vertex_stream->attributes[j];

			vertex_attributes[num_attributes].location = num_attributes;
			vertex_attributes[num_attributes].binding = i;
			vertex_attributes[num_attributes].format = vulkan_helperToVertexFormat(vertex_attribute->format);
			vertex_attributes[num_attributes].offset = vertex_attribute->offset;
			num_attributes++;
		}

		vertex_streams[i].binding = i;
		vertex_streams[i].stride = vertex_stream->stride;
		vertex_streams[i].inputRate = vulkan_helperToVertexInputRate(vertex_stream->rate);
	}

	vertex_input.vertexBindingDescriptionCount = desc->num_vertex_streams;
	vertex_input.pVertexBindingDescriptions = vertex_streams;
	vertex_input.vertexAttributeDescriptionCount = num_attributes;
	vertex_input.pVertexAttributeDescriptions = vertex_attributes;

	// input assembly state
	VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = vulkan_helperToPrimitiveTopology(desc->primitive_type);

	if (desc->primitive_type == OPAL_PRIMITIVE_TYPE_LINE_STRIP || desc->primitive_type == OPAL_PRIMITIVE_TYPE_TRIANGLE_STRIP)
		input_assembly.primitiveRestartEnable = VK_TRUE;

	// viewport state
	VkViewport default_viewport = {0};
	VkRect2D default_scissor = {0};

	VkPipelineViewportStateCreateInfo viewport = {0};
	viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport.viewportCount = 1;
	viewport.pViewports = &default_viewport;
	viewport.scissorCount = 1;
	viewport.pScissors = &default_scissor;

	// tessellation state
	VkPipelineTessellationStateCreateInfo tessellation = {0};
	tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tessellation.patchControlPoints = vulkan_helperToPatchControlPoints(desc->primitive_type);

	// rasterization state
	VkPipelineRasterizationStateCreateInfo rasterization = {0};
	rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization.cullMode = vulkan_helperToCullMode(desc->cull_mode);
	rasterization.frontFace = vulkan_helperToFrontFace(desc->front_face);
	rasterization.lineWidth = 1.0f;

	// multisample state
	VkPipelineMultisampleStateCreateInfo multisample = {0};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.rasterizationSamples = vulkan_helperToSamples(desc->rasterization_samples);

	// depstencil state
	VkPipelineDepthStencilStateCreateInfo depth_stencil = {0};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = desc->depth_enable;
	depth_stencil.depthWriteEnable = desc->depth_write;
	depth_stencil.depthCompareOp = vulkan_helperToCompareOp(desc->depth_compare_op);
	depth_stencil.stencilTestEnable = desc->stencil_enable;
	depth_stencil.front.failOp = vulkan_helperToStencilOp(desc->stencil_front.fail_op);
	depth_stencil.front.depthFailOp = vulkan_helperToStencilOp(desc->stencil_front.depth_fail_op);
	depth_stencil.front.passOp = vulkan_helperToStencilOp(desc->stencil_front.pass_op);
	depth_stencil.front.compareOp = vulkan_helperToCompareOp(desc->stencil_front.compare_op);
	depth_stencil.front.compareMask = desc->stencil_read_mask;
	depth_stencil.front.writeMask = desc->stencil_write_mask;
	depth_stencil.back.failOp = vulkan_helperToStencilOp(desc->stencil_back.fail_op);
	depth_stencil.back.depthFailOp = vulkan_helperToStencilOp(desc->stencil_back.depth_fail_op);
	depth_stencil.back.passOp = vulkan_helperToStencilOp(desc->stencil_back.pass_op);
	depth_stencil.back.compareOp = vulkan_helperToCompareOp(desc->stencil_back.compare_op);
	depth_stencil.back.compareMask = desc->stencil_read_mask;
	depth_stencil.back.writeMask = desc->stencil_write_mask;

	// colorblend state
	VkPipelineColorBlendAttachmentState blend_states[8];
	for (uint32_t i = 0; i < desc->num_color_attachments; ++i)
	{
		VkPipelineColorBlendAttachmentState *blend_state = &blend_states[i];
		const Opal_BlendState *opal_blend_state = &desc->color_blend_states[i];

		blend_state->blendEnable = opal_blend_state->enable;
		blend_state->srcColorBlendFactor = vulkan_helperToBlendFactor(opal_blend_state->src_color);
		blend_state->dstColorBlendFactor = vulkan_helperToBlendFactor(opal_blend_state->dst_color);
		blend_state->colorBlendOp = vulkan_helperToBlendOp(opal_blend_state->color_op);
		blend_state->srcAlphaBlendFactor = vulkan_helperToBlendFactor(opal_blend_state->src_alpha);
		blend_state->dstAlphaBlendFactor = vulkan_helperToBlendFactor(opal_blend_state->dst_alpha);
		blend_state->alphaBlendOp = vulkan_helperToBlendOp(opal_blend_state->alpha_op);
		blend_state->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	}

	VkPipelineColorBlendStateCreateInfo blend = {0};
	blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend.attachmentCount = desc->num_color_attachments;
	blend.pAttachments = blend_states;

	// dynamic state
	VkDynamicState dynamic_states[2] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamic = {0};
	dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic.dynamicStateCount = 2;
	dynamic.pDynamicStates = dynamic_states;

	// dynamic rendering extension
	VkFormat color_attachment_formats[8];

	for (uint32_t i = 0; i < desc->num_color_attachments; ++i)
		color_attachment_formats[i] = vulkan_helperToImageFormat(desc->color_attachment_formats[i]);

	VkPipelineRenderingCreateInfoKHR dynamic_rendering_info = {0};
	dynamic_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	dynamic_rendering_info.colorAttachmentCount = desc->num_color_attachments;
	dynamic_rendering_info.pColorAttachmentFormats = color_attachment_formats;

	if (desc->depth_stencil_attachment_format)
	{
		dynamic_rendering_info.depthAttachmentFormat = vulkan_helperToImageFormat(*desc->depth_stencil_attachment_format);
		dynamic_rendering_info.stencilAttachmentFormat = vulkan_helperToImageFormat(*desc->depth_stencil_attachment_format);
	}

	// pipeline
	VkGraphicsPipelineCreateInfo pipeline_info = {0};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
	pipeline_info.stageCount = num_shaders;
	pipeline_info.pStages = shader_infos;
	pipeline_info.pVertexInputState = &vertex_input;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pTessellationState = &tessellation;
	pipeline_info.pViewportState = &viewport;
	pipeline_info.pRasterizationState = &rasterization;
	pipeline_info.pMultisampleState = &multisample;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &blend;
	pipeline_info.pDynamicState = &dynamic;
	pipeline_info.layout = vulkan_pipeline_layout;
	pipeline_info.pNext = &dynamic_rendering_info;

	VkResult vulkan_result = device_ptr->vk.vkCreateGraphicsPipelines(vulkan_device, vulkan_pipeline_cache, 1, &pipeline_info, NULL, &vulkan_pipeline);

	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_Pipeline result = {0};
	result.pipeline = vulkan_pipeline;
	result.bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;

	*pipeline = (Opal_Pipeline)opal_poolAddElement(&device_ptr->pipelines, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateMeshletPipeline(Opal_Device this, const Opal_MeshletPipelineDesc *desc, Opal_Pipeline *pipeline)
{
	assert(this);
	assert(desc);
	assert(pipeline);

	OPAL_UNUSED(this);
	OPAL_UNUSED(desc);
	OPAL_UNUSED(pipeline);
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCreateComputePipeline(Opal_Device this, const Opal_ComputePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	assert(this);
	assert(desc);
	assert(desc->compute_function.shader);
	assert(desc->compute_function.name);
	assert(desc->pipeline_layout);
	assert(pipeline);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	// pipeline layout
	Vulkan_PipelineLayout *pipeline_layout_ptr = (Vulkan_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)desc->pipeline_layout);
	assert(pipeline_layout_ptr);

	VkPipelineLayout vulkan_pipeline_layout = pipeline_layout_ptr->layout;
	VkPipeline vulkan_pipeline = VK_NULL_HANDLE;
	VkPipelineCache vulkan_pipeline_cache = VK_NULL_HANDLE;

	// shader
	Vulkan_Shader *shader_ptr = (Vulkan_Shader *)opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)desc->compute_function.shader);
	assert(shader_ptr);

	// pipeline
	VkComputePipelineCreateInfo pipeline_info = {0};
	pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_info.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
	pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipeline_info.stage.pNext = NULL;
	pipeline_info.stage.flags = 0;
	pipeline_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	pipeline_info.stage.module = shader_ptr->shader;
	pipeline_info.stage.pName = desc->compute_function.name;
	pipeline_info.stage.pSpecializationInfo = NULL;
	pipeline_info.layout = vulkan_pipeline_layout;

	VkResult vulkan_result = device_ptr->vk.vkCreateComputePipelines(vulkan_device, vulkan_pipeline_cache, 1, &pipeline_info, NULL, &vulkan_pipeline);

	// error handling & result
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_Pipeline result = {0};
	result.pipeline = vulkan_pipeline;
	result.bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;

	*pipeline = (Opal_Pipeline)opal_poolAddElement(&device_ptr->pipelines, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateRaytracePipeline(Opal_Device this, const Opal_RaytracePipelineDesc *desc, Opal_Pipeline *pipeline)
{
	// TODO: add support for desc->max_ray_payload_size & desc->max_hit_attribute_size

	assert(this);
	assert(desc);
	assert(pipeline);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	// pipeline layout
	Vulkan_PipelineLayout *pipeline_layout_ptr = (Vulkan_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)desc->pipeline_layout);
	assert(pipeline_layout_ptr);

	VkPipelineLayout vulkan_pipeline_layout = pipeline_layout_ptr->layout;
	VkPipeline vulkan_pipeline = VK_NULL_HANDLE;
	VkPipelineCache vulkan_pipeline_cache = VK_NULL_HANDLE;

	uint32_t max_shader_modules = desc->num_raygen_functions + desc->num_hitgroup_functions * 3 + desc->num_miss_functions; 
	uint32_t max_shader_groups = desc->num_raygen_functions + desc->num_hitgroup_functions + desc->num_miss_functions;

	opal_bumpReset(&device_ptr->bump);
	uint32_t shader_stages_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkPipelineShaderStageCreateInfo) * max_shader_modules);
	uint32_t shader_groups_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkRayTracingShaderGroupCreateInfoKHR) * max_shader_groups);

	VkPipelineShaderStageCreateInfo *shader_stages = (VkPipelineShaderStageCreateInfo *)(device_ptr->bump.data + shader_stages_offset);
	memset(shader_stages, 0, sizeof(VkPipelineShaderStageCreateInfo) * max_shader_modules);

	VkRayTracingShaderGroupCreateInfoKHR *shader_groups = (VkRayTracingShaderGroupCreateInfoKHR *)(device_ptr->bump.data + shader_groups_offset);
	memset(shader_groups, 0, sizeof(VkRayTracingShaderGroupCreateInfoKHR) * max_shader_groups);

	// TODO: add hashmap lookup to gather unique shader stages, now let's create shader stages with duplicates
	uint32_t current_shader_offset = 0;
	uint32_t current_group_offset = 0;

	for (uint32_t i = 0; i < desc->num_raygen_functions; ++i)
	{
		Vulkan_Shader *shader_ptr = opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)desc->raygen_functions[i].shader);
		assert(shader_ptr);

		VkPipelineShaderStageCreateInfo *stage = &shader_stages[current_shader_offset];
		stage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage->stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		stage->module = shader_ptr->shader;
		stage->pName = desc->raygen_functions[i].name;

		VkRayTracingShaderGroupCreateInfoKHR *group = &shader_groups[current_group_offset];
		group->sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		group->type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group->generalShader = current_shader_offset;
		group->closestHitShader = VK_SHADER_UNUSED_KHR;
		group->anyHitShader = VK_SHADER_UNUSED_KHR;
		group->intersectionShader = VK_SHADER_UNUSED_KHR;

		current_shader_offset++;
		current_group_offset++;
	}

	for (uint32_t i = 0; i < desc->num_hitgroup_functions; ++i)
	{
		Vulkan_Shader *closesthit_shader_ptr = NULL;
		Vulkan_Shader *anyhit_shader_ptr = NULL;
		Vulkan_Shader *intersection_shader_ptr = NULL;
		
		closesthit_shader_ptr = opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)desc->hitgroup_functions[i].closesthit_function.shader);
		anyhit_shader_ptr = opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)desc->hitgroup_functions[i].anyhit_function.shader);
		intersection_shader_ptr = opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)desc->hitgroup_functions[i].intersection_function.shader);

		VkRayTracingShaderGroupCreateInfoKHR *group = &shader_groups[current_group_offset];
		group->sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		group->type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		group->generalShader = VK_SHADER_UNUSED_KHR;
		group->closestHitShader = VK_SHADER_UNUSED_KHR;
		group->anyHitShader = VK_SHADER_UNUSED_KHR;
		group->intersectionShader = VK_SHADER_UNUSED_KHR;

		if (closesthit_shader_ptr)
		{
			const char *name = desc->hitgroup_functions[i].closesthit_function.name;
			assert(name);

			VkPipelineShaderStageCreateInfo *stage = &shader_stages[current_shader_offset];
			stage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage->stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			stage->module = closesthit_shader_ptr->shader;
			stage->pName = name;

			group->closestHitShader = current_shader_offset++;
		}

		if (anyhit_shader_ptr)
		{
			const char *name = desc->hitgroup_functions[i].anyhit_function.name;
			assert(name);

			VkPipelineShaderStageCreateInfo *stage = &shader_stages[current_shader_offset];
			stage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage->stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			stage->module = anyhit_shader_ptr->shader;
			stage->pName = name;

			group->anyHitShader = current_shader_offset++;
		}

		if (intersection_shader_ptr)
		{
			const char *name = desc->hitgroup_functions[i].intersection_function.name;
			assert(name);

			VkPipelineShaderStageCreateInfo *stage = &shader_stages[current_shader_offset];
			stage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage->stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
			stage->module = intersection_shader_ptr->shader;
			stage->pName = name;

			group->intersectionShader = current_shader_offset++;
			group->type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
		}

		current_group_offset++;
	}

	for (uint32_t i = 0; i < desc->num_miss_functions; ++i)
	{
		Vulkan_Shader *shader_ptr = opal_poolGetElement(&device_ptr->shaders, (Opal_PoolHandle)desc->miss_functions[i].shader);
		assert(shader_ptr);

		VkPipelineShaderStageCreateInfo *stage = &shader_stages[current_shader_offset];
		stage->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage->stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		stage->module = shader_ptr->shader;
		stage->pName = desc->miss_functions[i].name;

		VkRayTracingShaderGroupCreateInfoKHR *group = &shader_groups[current_group_offset];
		group->sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		group->type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group->generalShader = current_shader_offset;
		group->closestHitShader = VK_SHADER_UNUSED_KHR;
		group->anyHitShader = VK_SHADER_UNUSED_KHR;
		group->intersectionShader = VK_SHADER_UNUSED_KHR;

		current_shader_offset++;
		current_group_offset++;
	}

	assert(current_shader_offset <= max_shader_modules);
	assert(current_group_offset <= max_shader_groups);

	// pipeline
	VkRayTracingPipelineCreateInfoKHR pipeline_info = {0};
	pipeline_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	pipeline_info.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
	pipeline_info.stageCount = current_shader_offset;
	pipeline_info.pStages = shader_stages;
	pipeline_info.groupCount = current_group_offset;
	pipeline_info.pGroups = shader_groups;
	pipeline_info.maxPipelineRayRecursionDepth = desc->max_recursion_depth;
	pipeline_info.layout = vulkan_pipeline_layout;

	VkResult vulkan_result = device_ptr->vk.vkCreateRayTracingPipelinesKHR(vulkan_device, VK_NULL_HANDLE, vulkan_pipeline_cache, 1, &pipeline_info, NULL, &vulkan_pipeline);

	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_Pipeline result = {0};
	result.pipeline = vulkan_pipeline;
	result.bind_point = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
	result.num_raygen_groups = desc->num_raygen_functions;
	result.num_hitgroup_groups = desc->num_hitgroup_functions;
	result.num_miss_groups = desc->num_miss_functions;

	*pipeline = (Opal_Pipeline)opal_poolAddElement(&device_ptr->pipelines, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateSwapchain(Opal_Device this, const Opal_SwapchainDesc *desc, Opal_Swapchain *swapchain)
{
	assert(this);
	assert(desc);
	assert(desc->surface);
	assert(swapchain);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Instance *instance_ptr = device_ptr->instance;
	VkPhysicalDevice vulkan_physical_device = device_ptr->physical_device;
	VkDevice vulkan_device = device_ptr->device;

	// surface
	Vulkan_Surface *surface_ptr = (Vulkan_Surface *)opal_poolGetElement(&instance_ptr->surfaces, (Opal_PoolHandle)desc->surface);
	assert(surface_ptr);

	VkSurfaceKHR vulkan_surface = surface_ptr->surface;
	VkSwapchainKHR vulkan_swapchain = VK_NULL_HANDLE;

	// surface capabilities
	VkSurfaceCapabilitiesKHR surface_capabilities = {0};
	VkResult vulkan_result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_physical_device, vulkan_surface, &surface_capabilities);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	uint32_t num_images = surface_capabilities.minImageCount + 1;

	if (surface_capabilities.maxImageCount > 0)
		num_images = min(num_images, surface_capabilities.maxImageCount);

	VkExtent2D extent = surface_capabilities.currentExtent;

	if (extent.width == 0xFFFFFFFF || extent.height == 0xFFFFFFFF)
		extent = surface_capabilities.minImageExtent;

	// surface present queue
	uint32_t present_queue_family = VK_QUEUE_FAMILY_IGNORED;
	VkBool32 present_supported = VK_FALSE;
	Opal_Queue present_queue = OPAL_NULL_HANDLE;

	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
	{
		uint32_t queue_family = device_ptr->device_engines_info.queue_families[i];
		Opal_Queue *queues = device_ptr->queue_handles[i];

		vulkan_result = vkGetPhysicalDeviceSurfaceSupportKHR(vulkan_physical_device, queue_family, vulkan_surface, &present_supported);
		if (vulkan_result != VK_SUCCESS)
			continue;

		if (present_supported)
		{
			present_queue_family = queue_family;
			present_queue = queues[0];
			break;
		}
	}

	if (present_supported == VK_FALSE)
		return OPAL_SWAPCHAIN_PRESENT_NOT_SUPPORTED;

	// surface present mode
	uint32_t num_present_modes = 0;
	vulkan_result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_physical_device, vulkan_surface, &num_present_modes, NULL);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(VkPresentModeKHR) * num_present_modes);

	VkPresentModeKHR *present_modes = (VkPresentModeKHR *)device_ptr->bump.data;
	vulkan_result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_physical_device, vulkan_surface, &num_present_modes, present_modes);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	VkPresentModeKHR wanted_present_mode = vulkan_helperToPresentMode(desc->mode);
	VkBool32 found_present_mode = VK_FALSE;

	for (uint32_t i = 0; i < num_present_modes; ++i)
	{
		if (present_modes[i] == wanted_present_mode)
		{
			found_present_mode = VK_TRUE;
			break;
		}
	}

	if (found_present_mode == VK_FALSE)
		return OPAL_SWAPCHAIN_PRESENT_MODE_NOT_SUPPORTED;

	// surface formats
	uint32_t num_surface_formats = 0;
	vulkan_result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_physical_device, vulkan_surface, &num_surface_formats, NULL);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(VkSurfaceFormatKHR) * num_surface_formats);

	VkSurfaceFormatKHR *surface_formats = (VkSurfaceFormatKHR *)device_ptr->bump.data;
	vulkan_result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_physical_device, vulkan_surface, &num_surface_formats, surface_formats);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	VkFormat wanted_format = vulkan_helperToImageFormat(desc->format.texture_format);
	VkColorSpaceKHR wanted_color_space = vulkan_helperToColorSpace(desc->format.color_space);

	VkBool32 found_format = VK_FALSE;
	VkBool32 found_color_space = VK_FALSE;

	for (uint32_t i = 0; i < num_surface_formats; ++i)
	{
		if (surface_formats[i].format == wanted_format)
			found_format = VK_TRUE;

		if (surface_formats[i].colorSpace == wanted_color_space)
			found_color_space = VK_TRUE;
		
		if (found_format && found_color_space)
			break;
	}

	if (found_format == VK_FALSE)
		return OPAL_SWAPCHAIN_FORMAT_NOT_SUPPORTED;

	if (found_color_space == VK_FALSE)
		return OPAL_SWAPCHAIN_COLOR_SPACE_NOT_SUPPORTED;

	// swap chain
	VkSwapchainCreateInfoKHR swapchain_info = {0};
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.surface = vulkan_surface;
	swapchain_info.minImageCount = num_images;
	swapchain_info.imageFormat = wanted_format;
	swapchain_info.imageColorSpace = wanted_color_space;
	swapchain_info.imageExtent = extent;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageUsage = vulkan_helperToImageUsage(desc->usage, desc->format.texture_format);
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_info.presentMode = wanted_present_mode;

	vulkan_result = device_ptr->vk.vkCreateSwapchainKHR(vulkan_device, &swapchain_info, NULL, &vulkan_swapchain);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	// images
	opal_bumpReset(&device_ptr->bump);
	opal_bumpAlloc(&device_ptr->bump, sizeof(VkImage) * num_images);

	VkImage *vulkan_images = (VkImage *)device_ptr->bump.data;
	device_ptr->vk.vkGetSwapchainImagesKHR(vulkan_device, vulkan_swapchain, &num_images, vulkan_images);
	if (vulkan_result != VK_SUCCESS)
	{
		device_ptr->vk.vkDestroySwapchainKHR(vulkan_device, vulkan_swapchain, NULL);
		return OPAL_VULKAN_ERROR;
	}

	// semaphores & image views
	VkSemaphore *vulkan_acquire_semaphores = (VkSemaphore *)malloc(sizeof(VkSemaphore) * num_images);
	memset(vulkan_acquire_semaphores, 0, sizeof(VkSemaphore) * num_images);

	VkSemaphore *vulkan_present_semaphores = (VkSemaphore *)malloc(sizeof(VkSemaphore) * num_images);
	memset(vulkan_present_semaphores, 0, sizeof(VkSemaphore) * num_images);

	Opal_TextureView *texture_views = (Opal_TextureView *)malloc(sizeof(Opal_TextureView) * num_images);
	memset(texture_views, OPAL_NULL_HANDLE, sizeof(Opal_TextureView) * num_images);

	VkSemaphoreCreateInfo semaphore_info = {0};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkBool32 success = VK_TRUE;
	for (uint32_t i = 0; i < num_images; ++i)
	{
		vulkan_result = device_ptr->vk.vkCreateSemaphore(vulkan_device, &semaphore_info, NULL, &vulkan_acquire_semaphores[i]);
		if (vulkan_result != VK_SUCCESS)
		{
			success = VK_FALSE;
			break;
		}

		vulkan_result = device_ptr->vk.vkCreateSemaphore(vulkan_device, &semaphore_info, NULL, &vulkan_present_semaphores[i]);
		if (vulkan_result != VK_SUCCESS)
		{
			success = VK_FALSE;
			break;
		}

		VkImageView vulkan_image_view = VK_NULL_HANDLE;

		VkImageViewCreateInfo image_view_info = {0};
		image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_info.image = vulkan_images[i];
		image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_info.format = wanted_format;
		image_view_info.components.r = VK_COMPONENT_SWIZZLE_R;
		image_view_info.components.g = VK_COMPONENT_SWIZZLE_G;
		image_view_info.components.b = VK_COMPONENT_SWIZZLE_B;
		image_view_info.components.a = VK_COMPONENT_SWIZZLE_A;
		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_info.subresourceRange.baseArrayLayer = 0;
		image_view_info.subresourceRange.layerCount = 1;
		image_view_info.subresourceRange.baseMipLevel = 0;
		image_view_info.subresourceRange.levelCount = 1;

		vulkan_result = device_ptr->vk.vkCreateImageView(vulkan_device, &image_view_info, NULL, &vulkan_image_view);
		if (vulkan_result != VK_SUCCESS)
		{
			success = VK_FALSE;
			break;
		}

		Vulkan_ImageView result = {0};
		result.image_view = vulkan_image_view;
		result.image = vulkan_images[i];
		result.width = extent.width;
		result.height = extent.height;
		result.depth = 1;
		result.base_mip = 0;
		result.num_mips = 1;
		result.base_layer = 0;
		result.num_layers = 1;
		result.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;

		texture_views[i] = (Opal_TextureView)opal_poolAddElement(&device_ptr->image_views, &result);
	}

	if (success != VK_TRUE)
	{
		for (uint32_t i = 0; i < num_images; ++i)
		{
			device_ptr->vk.vkDestroySemaphore(vulkan_device, vulkan_acquire_semaphores[i], NULL);
			device_ptr->vk.vkDestroySemaphore(vulkan_device, vulkan_present_semaphores[i], NULL);
			vulkan_deviceDestroyTextureView(this, texture_views[i]);
		}

		free(vulkan_acquire_semaphores);
		free(vulkan_present_semaphores);
		free(texture_views);

		device_ptr->vk.vkDestroySwapchainKHR(vulkan_device, vulkan_swapchain, NULL);
		return OPAL_VULKAN_ERROR;
	}

	Vulkan_Swapchain result = {0};
	result.swapchain = vulkan_swapchain;
	result.present_queue = present_queue;
	result.texture_views = texture_views;
	result.acquire_semaphores = vulkan_acquire_semaphores;
	result.present_semaphores = vulkan_present_semaphores;
	result.num_images = num_images;
	result.current_image = 0;
	result.current_semaphore = 0;

	*swapchain = (Opal_Swapchain)opal_poolAddElement(&device_ptr->swapchains, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroySemaphore(Opal_Device this, Opal_Semaphore semaphore)
{
	assert(this);
	assert(semaphore);

	Opal_PoolHandle handle = (Opal_PoolHandle)semaphore;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Semaphore *semaphore_ptr = (Vulkan_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, handle);
	assert(semaphore_ptr);

	opal_poolRemoveElement(&device_ptr->semaphores, handle);

	vulkan_destroySemaphore(device_ptr, semaphore_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer)
{
	assert(this);
	assert(buffer);

	Opal_PoolHandle handle = (Opal_PoolHandle)buffer;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, handle);
	assert(buffer_ptr);

	opal_poolRemoveElement(&device_ptr->buffers, handle);

	vulkan_destroyBuffer(device_ptr, buffer_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroyTexture(Opal_Device this, Opal_Texture texture)
{
	assert(this);
	assert(texture);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)texture;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Image *image_ptr = (Vulkan_Image *)opal_poolGetElement(&device_ptr->images, handle);
	assert(image_ptr);

	opal_poolRemoveElement(&device_ptr->images, handle);

	vulkan_destroyTexture(device_ptr, image_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	assert(this);
	assert(texture_view);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)texture_view;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_ImageView *image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, handle);
	assert(image_view_ptr);

	opal_poolRemoveElement(&device_ptr->image_views, handle);

	vulkan_destroyTextureView(device_ptr, image_view_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroySampler(Opal_Device this, Opal_Sampler sampler)
{
	assert(this);
	assert(sampler);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)sampler;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Sampler *sampler_ptr = (Vulkan_Sampler *)opal_poolGetElement(&device_ptr->samplers, handle);
	assert(sampler_ptr);

	opal_poolRemoveElement(&device_ptr->samplers, handle);

	vulkan_destroySampler(device_ptr, sampler_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroyAccelerationStructure(Opal_Device this, Opal_AccelerationStructure acceleration_structure)
{
	assert(this);
	assert(acceleration_structure);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)acceleration_structure;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_AccelerationStructure *acceleration_structure_ptr = (Vulkan_AccelerationStructure *)opal_poolGetElement(&device_ptr->acceleration_structures, handle);
	assert(acceleration_structure_ptr);

	opal_poolRemoveElement(&device_ptr->acceleration_structures, handle);

	vulkan_destroyAccelerationStructure(device_ptr, acceleration_structure_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroyCommandAllocator(Opal_Device this, Opal_CommandAllocator command_allocator)
{
	assert(this);
	assert(command_allocator);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)command_allocator;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_CommandAllocator *command_allocator_ptr = (Vulkan_CommandAllocator *)opal_poolGetElement(&device_ptr->command_allocators, handle);
	assert(command_allocator_ptr);

	// TODO: fix memory leak caused by orphaned Vulkan_CommandBuffer instances

	opal_poolRemoveElement(&device_ptr->command_allocators, handle);

	vulkan_destroyCommandAllocator(device_ptr, command_allocator_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroyCommandBuffer(Opal_Device this, Opal_CommandAllocator command_allocator, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_allocator);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	Vulkan_CommandAllocator *command_allocator_ptr = (Vulkan_CommandAllocator *)opal_poolGetElement(&device_ptr->command_allocators, (Opal_PoolHandle)command_allocator);
	assert(command_allocator_ptr);

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	device_ptr->vk.vkFreeCommandBuffers(vulkan_device, command_allocator_ptr->pool, 1, &command_buffer_ptr->command_buffer);

	// TODO: remove handle from Vulkan_CommandAllocator instance

	opal_poolRemoveElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	assert(this);
	assert(shader);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)shader;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Shader *shader_ptr = (Vulkan_Shader *)opal_poolGetElement(&device_ptr->shaders, handle);
	assert(shader_ptr);

	opal_poolRemoveElement(&device_ptr->shaders, handle);

	vulkan_destroyShader(device_ptr, shader_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroyDescriptorHeap(Opal_Device this, Opal_DescriptorHeap descriptor_heap)
{
	assert(this);
	assert(descriptor_heap);

	Opal_PoolHandle handle = (Opal_PoolHandle)descriptor_heap;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_DescriptorHeap *descriptor_heap_ptr = (Vulkan_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, handle);
	assert(descriptor_heap_ptr);

	opal_poolRemoveElement(&device_ptr->descriptor_heaps, handle);

	vulkan_destroyDescriptorHeap(device_ptr, descriptor_heap_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroyDescriptorSetLayout(Opal_Device this, Opal_DescriptorSetLayout descriptor_set_layout)
{
	assert(this);
	assert(descriptor_set_layout);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)descriptor_set_layout;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_DescriptorSetLayout *descriptor_set_layout_ptr = (Vulkan_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, handle);
	assert(descriptor_set_layout_ptr);

	opal_poolRemoveElement(&device_ptr->descriptor_set_layouts, handle);

	vulkan_destroyDescriptorSetLayout(device_ptr, descriptor_set_layout_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	assert(this);
	assert(pipeline_layout);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)pipeline_layout;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_PipelineLayout *pipeline_layout_ptr = (Vulkan_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, handle);
	assert(pipeline_layout_ptr);

	opal_poolRemoveElement(&device_ptr->pipeline_layouts, handle);

	vulkan_destroyPipelineLayout(device_ptr, pipeline_layout_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroyPipeline(Opal_Device this, Opal_Pipeline pipeline)
{
	assert(this);
	assert(pipeline);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)pipeline;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Pipeline *pipeline_ptr = (Vulkan_Pipeline *)opal_poolGetElement(&device_ptr->pipelines, handle);
	assert(pipeline_ptr);

	opal_poolRemoveElement(&device_ptr->pipelines, handle);

	vulkan_destroyPipeline(device_ptr, pipeline_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroySwapchain(Opal_Device this, Opal_Swapchain swapchain)
{
	assert(this);
	assert(swapchain);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)swapchain;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Swapchain *swapchain_ptr = (Vulkan_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, handle);
	assert(swapchain_ptr);

	for (uint32_t i = 0; i < swapchain_ptr->num_images; ++i)
		vulkan_deviceDestroyTextureView(this, swapchain_ptr->texture_views[i]);

	opal_poolRemoveElement(&device_ptr->swapchains, handle);

	vulkan_destroySwapchain(device_ptr, swapchain_ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroy(Opal_Device this)
{
	assert(this);

	Vulkan_Device *ptr = (Vulkan_Device *)this;

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->swapchains);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_Swapchain *swapchain_ptr = (Vulkan_Swapchain *)opal_poolGetElementByIndex(&ptr->swapchains, head);
			vulkan_destroySwapchain(ptr, swapchain_ptr);

			head = opal_poolGetNextIndex(&ptr->swapchains, head);
		}


		opal_poolShutdown(&ptr->swapchains);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->pipelines);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_Pipeline *pipeline_ptr = (Vulkan_Pipeline *)opal_poolGetElementByIndex(&ptr->pipelines, head);
			vulkan_destroyPipeline(ptr, pipeline_ptr);

			head = opal_poolGetNextIndex(&ptr->pipelines, head);
		}

		opal_poolShutdown(&ptr->pipelines);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->pipeline_layouts);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_PipelineLayout *pipeline_layout_ptr = (Vulkan_PipelineLayout *)opal_poolGetElementByIndex(&ptr->pipeline_layouts, head);
			vulkan_destroyPipelineLayout(ptr, pipeline_layout_ptr);

			head = opal_poolGetNextIndex(&ptr->pipeline_layouts, head);
		}

		opal_poolShutdown(&ptr->pipeline_layouts);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->descriptor_set_layouts);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_DescriptorSetLayout *descriptor_set_layout_ptr = (Vulkan_DescriptorSetLayout *)opal_poolGetElementByIndex(&ptr->descriptor_set_layouts, head);
			vulkan_destroyDescriptorSetLayout(ptr, descriptor_set_layout_ptr);

			head = opal_poolGetNextIndex(&ptr->descriptor_set_layouts, head);
		}

		opal_poolShutdown(&ptr->descriptor_set_layouts);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->descriptor_heaps);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_DescriptorHeap *descriptor_heap_ptr = (Vulkan_DescriptorHeap *)opal_poolGetElementByIndex(&ptr->descriptor_heaps, head);
			vulkan_destroyDescriptorHeap(ptr, descriptor_heap_ptr);

			head = opal_poolGetNextIndex(&ptr->descriptor_heaps, head);
		}

		opal_poolShutdown(&ptr->descriptor_heaps);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->shaders);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_Shader *shader_ptr = (Vulkan_Shader *)opal_poolGetElementByIndex(&ptr->shaders, head);
			vulkan_destroyShader(ptr, shader_ptr);

			head = opal_poolGetNextIndex(&ptr->shaders, head);
		}

		opal_poolShutdown(&ptr->shaders);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->command_allocators);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_CommandAllocator *command_allocator_ptr = (Vulkan_CommandAllocator *)opal_poolGetElementByIndex(&ptr->command_allocators, head);
			vulkan_destroyCommandAllocator(ptr, command_allocator_ptr);

			head = opal_poolGetNextIndex(&ptr->command_allocators, head);
		}

		opal_poolShutdown(&ptr->command_allocators);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->acceleration_structures);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_AccelerationStructure *acceleration_structure_ptr = (Vulkan_AccelerationStructure *)opal_poolGetElementByIndex(&ptr->acceleration_structures, head);
			vulkan_destroyAccelerationStructure(ptr, acceleration_structure_ptr);

			head = opal_poolGetNextIndex(&ptr->acceleration_structures, head);
		}

		opal_poolShutdown(&ptr->acceleration_structures);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->samplers);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_Sampler *sampler_ptr = (Vulkan_Sampler *)opal_poolGetElementByIndex(&ptr->samplers, head);
			vulkan_destroySampler(ptr, sampler_ptr);

			head = opal_poolGetNextIndex(&ptr->samplers, head);
		}

		opal_poolShutdown(&ptr->samplers);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->image_views);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_ImageView *image_view_ptr = (Vulkan_ImageView *)opal_poolGetElementByIndex(&ptr->image_views, head);
			vulkan_destroyTextureView(ptr, image_view_ptr);

			head = opal_poolGetNextIndex(&ptr->image_views, head);
		}

		opal_poolShutdown(&ptr->image_views);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->images);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_Image *image_ptr = (Vulkan_Image *)opal_poolGetElementByIndex(&ptr->images, head);
			vulkan_destroyTexture(ptr, image_ptr);

			head = opal_poolGetNextIndex(&ptr->images, head);
		}

		opal_poolShutdown(&ptr->images);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->buffers);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElementByIndex(&ptr->buffers, head);
			vulkan_destroyBuffer(ptr, buffer_ptr);

			head = opal_poolGetNextIndex(&ptr->buffers, head);
		}

		opal_poolShutdown(&ptr->buffers);
	}

	{
		uint32_t head = opal_poolGetHeadIndex(&ptr->semaphores);
		while (head != OPAL_POOL_HANDLE_NULL)
		{
			Vulkan_Semaphore *semaphore_ptr = (Vulkan_Semaphore *)opal_poolGetElementByIndex(&ptr->semaphores, head);
			vulkan_destroySemaphore(ptr, semaphore_ptr);

			head = opal_poolGetNextIndex(&ptr->semaphores, head);
		}

		opal_poolShutdown(&ptr->semaphores);
	}

	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
		free(ptr->queue_handles[i]);

	opal_poolShutdown(&ptr->queues);
	opal_poolShutdown(&ptr->descriptor_sets);
	opal_poolShutdown(&ptr->command_buffers);

	opal_bumpShutdown(&ptr->bump);

#ifdef OPAL_HAS_VMA
	if (ptr->use_vma > 0)
	{
		vmaDestroyAllocator(ptr->vma_allocator);
	}
	else
#endif
	{
		Opal_Result result = vulkan_allocatorShutdown(ptr);
		assert(result == OPAL_SUCCESS);

		OPAL_UNUSED(result);
	}

	vkDestroyDevice(ptr->device, NULL);

	free(ptr);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceBuildShaderBindingTable(Opal_Device this, const Opal_ShaderBindingTableBuildDesc *desc)
{
	assert(this);
	assert(desc);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	uint32_t handle_size = device_ptr->raytrace_properties.shaderGroupHandleSize;
	uint32_t handle_alignment = device_ptr->raytrace_properties.shaderGroupHandleAlignment;
	uint32_t base_alignment = device_ptr->raytrace_properties.shaderGroupBaseAlignment;

	uint32_t aligned_handle_size = alignUp(handle_size, handle_alignment);

	assert(isAlignedul(desc->sbt.offset, base_alignment));

	uint8_t *handles_dst_data = NULL;
	Opal_Result result = vulkan_deviceMapBuffer(this, desc->sbt.buffer, &handles_dst_data);
	if (result != OPAL_SUCCESS)
		return result;

	handles_dst_data += desc->sbt.offset;

	Vulkan_Pipeline *pipeline_ptr = (Vulkan_Pipeline *)opal_poolGetElement(&device_ptr->pipelines, (Opal_PoolHandle)desc->pipeline);
	assert(pipeline_ptr);
	assert(pipeline_ptr->bind_point == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

	uint32_t num_raygen_groups = pipeline_ptr->num_raygen_groups;
	uint32_t num_hitgroup_group = pipeline_ptr->num_hitgroup_groups;
	uint32_t num_miss_group = pipeline_ptr->num_miss_groups;

	uint32_t num_groups = num_raygen_groups + num_hitgroup_group + num_miss_group;

	opal_bumpReset(&device_ptr->bump);
	uint32_t group_handles_offset = opal_bumpAlloc(&device_ptr->bump, handle_size * num_groups);

	uint8_t *handles_src_data = device_ptr->bump.data + group_handles_offset;
	VkResult vulkan_result = device_ptr->vk.vkGetRayTracingShaderGroupHandlesKHR(vulkan_device, pipeline_ptr->pipeline, 0, num_groups, handle_size * num_groups, handles_src_data);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	const uint8_t *raygen_src_data = handles_src_data;
	const uint8_t *hitgroup_src_data = handles_src_data + num_raygen_groups * handle_size;
	const uint8_t *miss_src_data = hitgroup_src_data + num_hitgroup_group * handle_size;

	uint8_t *raygen_dst_data = handles_dst_data;
	uint8_t *hitgroup_dst_data = handles_dst_data + alignUp(desc->num_raygen_indices * aligned_handle_size, base_alignment);
	uint8_t *miss_dst_data = hitgroup_dst_data + alignUp(desc->num_hitgroup_indices * aligned_handle_size, base_alignment);

	for (uint32_t i = 0; i < desc->num_raygen_indices; ++i)
	{
		uint32_t offset = desc->raygen_indices[i] * handle_size;
		memcpy(raygen_dst_data, raygen_src_data + offset, handle_size);
		raygen_dst_data += aligned_handle_size;
	}

	for (uint32_t i = 0; i < desc->num_hitgroup_indices; ++i)
	{
		uint32_t offset = desc->hitgroup_indices[i] * handle_size;
		memcpy(hitgroup_dst_data, hitgroup_src_data + offset, handle_size);
		hitgroup_dst_data += aligned_handle_size;
	}

	for (uint32_t i = 0; i < desc->num_miss_indices; ++i)
	{
		uint32_t offset = desc->miss_indices[i] * handle_size;
		memcpy(miss_dst_data, miss_src_data + offset, handle_size);
		miss_dst_data += aligned_handle_size;
	}

	return vulkan_deviceUnmapBuffer(this, desc->sbt.buffer);
}

static Opal_Result vulkan_deviceBuildAccelerationStructureInstanceBuffer(Opal_Device this, const Opal_AccelerationStructureInstanceBufferBuildDesc *desc)
{
	assert(this);
	assert(desc);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	uint8_t *dst_data;
	Opal_Result result = vulkan_deviceMapBuffer(this, desc->buffer.buffer, &dst_data);
	if (result != OPAL_SUCCESS)
		return result;

	dst_data += desc->buffer.offset;

	for (uint32_t i = 0; i < desc->num_instances; ++i)
	{
		const Opal_AccelerationStructureInstance *opal_instance = &desc->instances[i];
		VkAccelerationStructureInstanceKHR *vulkan_instance = (VkAccelerationStructureInstanceKHR *)dst_data;

		Vulkan_AccelerationStructure *blas_ptr = (Vulkan_AccelerationStructure *)opal_poolGetElement(&device_ptr->acceleration_structures, (Opal_PoolHandle)opal_instance->blas);
		assert(blas_ptr);

		memcpy(&vulkan_instance->transform, opal_instance->transform, sizeof(VkTransformMatrixKHR));
		vulkan_instance->instanceCustomIndex = opal_instance->custom_index;
		vulkan_instance->mask = opal_instance->mask;
		vulkan_instance->instanceShaderBindingTableRecordOffset = opal_instance->sbt_hitgroup_index_offset;
		vulkan_instance->flags = vulkan_helperToAccelerationStructureGeometryInstanceFlags(opal_instance->flags);
		vulkan_instance->accelerationStructureReference = blas_ptr->device_address;

		dst_data += sizeof(VkAccelerationStructureInstanceKHR);
	}

	return vulkan_deviceUnmapBuffer(this, desc->buffer.buffer);
}

static Opal_Result vulkan_deviceResetCommandAllocator(Opal_Device this, Opal_CommandAllocator command_allocator)
{
	assert(this);
	assert(command_allocator);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	Vulkan_CommandAllocator *command_allocator_ptr = (Vulkan_CommandAllocator *)opal_poolGetElement(&device_ptr->command_allocators, (Opal_PoolHandle)command_allocator);
	assert(command_allocator_ptr);

	VkResult vulkan_result = device_ptr->vk.vkResetCommandPool(vulkan_device, command_allocator_ptr->pool, 0);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceAllocateDescriptorSet(Opal_Device this, const Opal_DescriptorSetAllocationDesc *desc, Opal_DescriptorSet *descriptor_set)
{
	assert(this);
	assert(desc);
	assert(descriptor_set);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDescriptorSet vulkan_descriptor_set = VK_NULL_HANDLE;

	Vulkan_DescriptorHeap *descriptor_heap_ptr = (Vulkan_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)desc->heap);
	assert(descriptor_heap_ptr);

	Vulkan_DescriptorSetLayout *descriptor_set_layout_ptr = (Vulkan_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)desc->layout);
	assert(descriptor_set_layout_ptr);

	Vulkan_DescriptorSet result = {0};

	uint32_t num_static_descriptors = descriptor_set_layout_ptr->num_static_descriptors;
	if (num_static_descriptors > 0)
	{
		result.num_static_descriptors = num_static_descriptors;

		Opal_Result opal_result = opal_heapAlloc(&descriptor_heap_ptr->heap, descriptor_set_layout_ptr->num_blocks, &result.allocation);
		if (opal_result != OPAL_SUCCESS)
			return opal_result;
	}

	uint32_t num_dynamic_descriptors = descriptor_set_layout_ptr->num_dynamic_descriptors;
	if (num_dynamic_descriptors > 0)
	{
		result.num_dynamic_descriptors = num_dynamic_descriptors;
		result.dynamic_descriptors = (Opal_DescriptorSetEntry *)malloc(sizeof(Opal_DescriptorSetEntry) * num_dynamic_descriptors);
	}

	result.set = vulkan_descriptor_set;
	result.layout = desc->layout;
	result.heap = desc->heap;

	*descriptor_set = (Opal_DescriptorSet)opal_poolAddElement(&device_ptr->descriptor_sets, &result);

	if (desc->num_entries == 0)
		return OPAL_SUCCESS;

	assert(desc->entries);

	Opal_Result opal_result = vulkan_deviceUpdateDescriptorSet(this, *descriptor_set, desc->num_entries, desc->entries);
	if (opal_result != OPAL_SUCCESS)
	{
		vulkan_deviceFreeDescriptorSet(this, *descriptor_set);
		*descriptor_set = OPAL_NULL_HANDLE;
	}

	return opal_result;
}

static Opal_Result vulkan_deviceFreeDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set)
{
	assert(this);
	assert(descriptor_set);
 
 	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_DescriptorSet *descriptor_set_ptr = (Vulkan_DescriptorSet *)opal_poolGetElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	assert(descriptor_set_ptr);

	Vulkan_DescriptorHeap *descriptor_heap_ptr = (Vulkan_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)descriptor_set_ptr->heap);
	assert(descriptor_heap_ptr);

	if (descriptor_set_ptr->num_static_descriptors > 0)
	{
		Opal_Result opal_result = opal_heapFree(&descriptor_heap_ptr->heap, descriptor_set_ptr->allocation);
		assert(opal_result == OPAL_SUCCESS);

		descriptor_set_ptr->num_static_descriptors = 0;
	}

	if (descriptor_set_ptr->num_dynamic_descriptors > 0)
	{
		free(descriptor_set_ptr->dynamic_descriptors);
		descriptor_set_ptr->dynamic_descriptors = NULL;
		descriptor_set_ptr->num_dynamic_descriptors = 0;
	}

	opal_poolRemoveElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr)
{
	assert(this);
	assert(buffer);
	assert(ptr);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer);
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

	uint8_t *block_memory = NULL;
	Opal_Result result = vulkan_allocatorMapMemory(device_ptr, buffer_ptr->allocation, &block_memory);

	if (result != OPAL_SUCCESS)
		return result;

	buffer_ptr->map_count++;
	*ptr = block_memory + buffer_ptr->allocation.offset;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceUnmapBuffer(Opal_Device this, Opal_Buffer buffer)
{
	assert(this);
	assert(buffer);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer);
	assert(buffer_ptr);

#if OPAL_HAS_VMA
	if (device_ptr->use_vma > 0)
	{
		vmaUnmapMemory(device_ptr->vma_allocator, buffer_ptr->vma_allocation);
		return OPAL_SUCCESS;
	}
#endif

	assert(buffer_ptr->map_count > 0);

	buffer_ptr->map_count--;
	return vulkan_allocatorUnmapMemory(device_ptr, buffer_ptr->allocation);
}

static Opal_Result vulkan_deviceWriteBuffer(Opal_Device this, Opal_Buffer buffer, uint64_t offset, const void *data, uint64_t size)
{
	OPAL_UNUSED(this);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(offset);
	OPAL_UNUSED(data);
	OPAL_UNUSED(size);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceUpdateDescriptorSet(Opal_Device this, Opal_DescriptorSet descriptor_set, uint32_t num_entries, const Opal_DescriptorSetEntry *entries)
{
	assert(this);
	assert(descriptor_set);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	Vulkan_DescriptorSet *descriptor_set_ptr = (Vulkan_DescriptorSet *)opal_poolGetElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	assert(descriptor_set_ptr);

	Vulkan_DescriptorSetLayout *descriptor_set_layout_ptr = (Vulkan_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_ptr->layout);
	assert(descriptor_set_layout_ptr);

	Vulkan_DescriptorHeap *descriptor_heap_ptr = (Vulkan_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)descriptor_set_ptr->heap);
	assert(descriptor_heap_ptr);

	typedef struct DescriptorIndices_t
	{
		uint32_t layout;
		uint32_t entry;
	} DescriptorIndices;

	opal_bumpReset(&device_ptr->bump);
	uint32_t static_indices_ptr_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(DescriptorIndices *) * num_entries);
	uint32_t dynamic_indices_ptr_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(DescriptorIndices *) * num_entries);

	uint32_t num_static_indices = 0;
	uint32_t num_dynamic_indices = 0;
	DescriptorIndices *static_layout_indices = (DescriptorIndices *)(device_ptr->bump.data + static_indices_ptr_offset);
	DescriptorIndices *dynamic_layout_indices = (DescriptorIndices *)(device_ptr->bump.data + dynamic_indices_ptr_offset);

	// TODO: replace naive O(N) layout <-> binding search loop by O(1) hashmap lookup
	for (uint32_t i = 0; i < num_entries; ++i)
	{
		const Opal_DescriptorSetEntry *entry = &entries[i];

		uint32_t index = UINT32_MAX;
		for (uint32_t j = 0; j < descriptor_set_layout_ptr->num_entries; ++j)
		{
			const VkDescriptorSetLayoutBinding *info = &descriptor_set_layout_ptr->entries[j];
			if (info->binding == entry->binding)
			{
				index = j;
				break;
			}
		}

		assert(index != UINT32_MAX);
		if (index < descriptor_set_layout_ptr->num_static_descriptors)
		{
			static_layout_indices[num_static_indices].entry = i;
			static_layout_indices[num_static_indices].layout = index;
			num_static_indices++;
		}
		else
		{
			dynamic_layout_indices[num_dynamic_indices].entry = i;
			dynamic_layout_indices[num_dynamic_indices].layout = index;
			num_dynamic_indices++;
		}
	}

	uint8_t *base_ptr = descriptor_heap_ptr->buffer_ptr + descriptor_set_ptr->allocation.offset * device_ptr->descriptor_buffer_properties.descriptorBufferOffsetAlignment;

	for (uint32_t i = 0; i < num_static_indices; ++i)
	{
		uint32_t layout_index = static_layout_indices[i].layout;
		uint32_t entry_index = static_layout_indices[i].entry;

		const VkDescriptorSetLayoutBinding *info = &descriptor_set_layout_ptr->entries[layout_index];
		assert(info->descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
		assert(info->descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);

		const Opal_DescriptorSetEntry *entry = &entries[entry_index];
		assert(info->binding == entry->binding);

		uint8_t *descriptor_ptr = base_ptr + descriptor_set_layout_ptr->offsets[layout_index];

		VkDescriptorImageInfo image_info = {0};

		VkDescriptorAddressInfoEXT address_info = {0};
		address_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;

		VkDescriptorGetInfoEXT descriptor_info = {0};
		descriptor_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
		descriptor_info.type = info->descriptorType;
		size_t descriptor_size = 0;

		switch (info->descriptorType)
		{
			case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			{
				Opal_TextureView data = entry->data.texture_view;

				Vulkan_ImageView *image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, (Opal_PoolHandle)data);
				assert(image_view_ptr);

				image_info.imageView = image_view_ptr->image_view;
				image_info.sampler = NULL;
				image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				descriptor_info.data.pSampledImage = &image_info;
				descriptor_size = device_ptr->descriptor_buffer_properties.sampledImageDescriptorSize;
			}
			break;

			case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			{
				Opal_TextureView data = entry->data.texture_view;

				Vulkan_ImageView *image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, (Opal_PoolHandle)data);
				assert(image_view_ptr);

				image_info.imageView = image_view_ptr->image_view;
				image_info.sampler = NULL;
				image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

				descriptor_info.data.pStorageImage = &image_info;
				descriptor_size = device_ptr->descriptor_buffer_properties.storageImageDescriptorSize;
			}
			break;

			case VK_DESCRIPTOR_TYPE_SAMPLER:
			{
				Opal_Sampler data = entry->data.sampler;

				Vulkan_Sampler *sampler_ptr = (Vulkan_Sampler *)opal_poolGetElement(&device_ptr->samplers, (Opal_PoolHandle)data);
				assert(sampler_ptr);

				descriptor_info.data.pSampler = &sampler_ptr->sampler;
				descriptor_size = device_ptr->descriptor_buffer_properties.samplerDescriptorSize;
			}
			break;

			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			{
				Opal_StorageBufferView data = entry->data.storage_buffer_view;

				Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)data.buffer);
				assert(buffer_ptr);

				address_info.address = buffer_ptr->device_address + data.offset;
				address_info.range = data.element_size * data.num_elements;
				address_info.format = VK_FORMAT_UNDEFINED;

				descriptor_info.data.pStorageBuffer = &address_info;
				descriptor_size = device_ptr->descriptor_buffer_properties.storageBufferDescriptorSize;
			}
			break;

			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			{
				Opal_BufferView data = entry->data.buffer_view;

				Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)data.buffer);
				assert(buffer_ptr);

				address_info.address = buffer_ptr->device_address + data.offset;
				address_info.range = data.size;
				address_info.format = VK_FORMAT_UNDEFINED;

				descriptor_info.data.pUniformBuffer = &address_info;
				descriptor_size = device_ptr->descriptor_buffer_properties.uniformBufferDescriptorSize;
			}
			break;

			case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
			{
				Opal_AccelerationStructure data = entry->data.acceleration_structure;

				Vulkan_AccelerationStructure *acceleration_structure_ptr = (Vulkan_AccelerationStructure *)opal_poolGetElement(&device_ptr->acceleration_structures, (Opal_PoolHandle)data);
				assert(acceleration_structure_ptr);

				descriptor_info.data.accelerationStructure = acceleration_structure_ptr->device_address;
				descriptor_size = device_ptr->descriptor_buffer_properties.accelerationStructureDescriptorSize;
			}
			break;
		}

		assert(descriptor_size > 0);
		device_ptr->vk.vkGetDescriptorEXT(vulkan_device, &descriptor_info, descriptor_size, descriptor_ptr);
	}

	for (uint32_t i = 0; i < num_dynamic_indices; ++i)
	{
		uint32_t layout_index = dynamic_layout_indices[i].layout;
		uint32_t entry_index = dynamic_layout_indices[i].entry;

		const VkDescriptorSetLayoutBinding *info = &descriptor_set_layout_ptr->entries[layout_index];
		const Opal_DescriptorSetEntry *entry = &entries[entry_index];
		assert(info->binding == entry->binding);

		memcpy(&descriptor_set_ptr->dynamic_descriptors[i], entry, sizeof(Opal_DescriptorSetEntry));
	}

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceBeginCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	VkCommandBufferBeginInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkResult result = device_ptr->vk.vkBeginCommandBuffer(command_buffer_ptr->command_buffer, &info);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	VkResult result = device_ptr->vk.vkEndCommandBuffer(command_buffer_ptr->command_buffer);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceQuerySemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t *value)
{
	assert(this);
	assert(semaphore);
	assert(value);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_Semaphore *semaphore_ptr = (Vulkan_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);

	VkResult result = device_ptr->vk.vkGetSemaphoreCounterValueKHR(device_ptr->device, semaphore_ptr->semaphore, value);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceSignalSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value)
{
	assert(this);
	assert(semaphore);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_Semaphore *semaphore_ptr = (Vulkan_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);

	VkSemaphoreSignalInfoKHR signal_info = {0};
	signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO_KHR;
	signal_info.semaphore = semaphore_ptr->semaphore;
	signal_info.value = value;

	VkResult result = device_ptr->vk.vkSignalSemaphoreKHR(device_ptr->device, &signal_info);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceWaitSemaphore(Opal_Device this, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds)
{
	assert(this);
	assert(semaphore);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_Semaphore *semaphore_ptr = (Vulkan_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)semaphore);
	assert(semaphore_ptr);

	VkSemaphoreWaitInfoKHR wait_info = {0};
	wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO_KHR;
	wait_info.semaphoreCount = 1;
	wait_info.pSemaphores = &semaphore_ptr->semaphore;
	wait_info.pValues = &value;

	VkResult result = device_ptr->vk.vkWaitSemaphoresKHR(device_ptr->device, &wait_info, timeout_milliseconds * 1000000);
	if (result == VK_TIMEOUT)
		return OPAL_WAIT_TIMEOUT;

	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceWaitQueue(Opal_Device this, Opal_Queue queue)
{
	assert(this);
	assert(queue);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_Queue *queue_ptr = (Vulkan_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queue);
	assert(queue_ptr);

	VkResult result = device_ptr->vk.vkQueueWaitIdle(queue_ptr->queue);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceWaitIdle(Opal_Device this)
{
	assert(this);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	VkResult result = device_ptr->vk.vkDeviceWaitIdle(vulkan_device);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceSubmit(Opal_Device this, Opal_Queue queue, const Opal_SubmitDesc *desc)
{
	assert(this);
	assert(desc);
 	assert(desc->num_command_buffers > 0);
	assert(desc->command_buffers);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_Queue *queue_ptr = (Vulkan_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)queue);
	assert(queue_ptr);

	uint32_t wait_semaphores_offset = 0;
	uint32_t wait_masks_offset = 0;
	uint32_t wait_values_offset = 0;
	uint32_t submit_command_buffers_offset = 0;
	uint32_t signal_semaphores_offset = 0;
	uint32_t signal_values_offset = 0;

	opal_bumpReset(&device_ptr->bump);

	uint32_t num_wait_objects = desc->num_wait_semaphores + desc->num_wait_swapchains;
	if (num_wait_objects > 0)
	{
		wait_semaphores_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkSemaphore) * num_wait_objects);
		wait_values_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(uint64_t) * num_wait_objects);
		wait_masks_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkPipelineStageFlags) * num_wait_objects);
	}

	if (desc->num_command_buffers > 0)
		submit_command_buffers_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkCommandBuffer) * desc->num_command_buffers);

	uint32_t num_signal_objects = desc->num_signal_semaphores + desc->num_signal_swapchains;
	if (num_signal_objects > 0)
	{
		signal_semaphores_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkSemaphore) * num_signal_objects);
		signal_values_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(uint64_t) * num_signal_objects);
	}

	VkSemaphore *wait_semaphores = (VkSemaphore *)(device_ptr->bump.data + wait_semaphores_offset);
	uint64_t *wait_values = (uint64_t *)(device_ptr->bump.data + wait_values_offset);
	VkPipelineStageFlags *wait_masks = (VkPipelineStageFlags *)(device_ptr->bump.data + wait_masks_offset);
	VkCommandBuffer *submit_command_buffers = (VkCommandBuffer *)(device_ptr->bump.data + submit_command_buffers_offset);
	VkSemaphore *signal_semaphores = (VkSemaphore *)(device_ptr->bump.data + signal_semaphores_offset);
	uint64_t *signal_values = (uint64_t *)(device_ptr->bump.data + signal_values_offset);

	uint32_t current_wait_object = 0;
	for (uint32_t i = 0; i < desc->num_wait_semaphores; ++i)
	{
		Vulkan_Semaphore *semaphore_ptr = (Vulkan_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)desc->wait_semaphores[i]);
		assert(semaphore_ptr);

		wait_semaphores[current_wait_object] = semaphore_ptr->semaphore;
		wait_values[current_wait_object] = desc->wait_values[i];
		wait_masks[current_wait_object] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		current_wait_object++;
	}

	for (uint32_t i = 0; i < desc->num_wait_swapchains; ++i)
	{
		Vulkan_Swapchain *swapchain_ptr = (Vulkan_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)desc->wait_swapchains[i]);
		assert(swapchain_ptr);

		wait_semaphores[current_wait_object] = swapchain_ptr->acquire_semaphores[swapchain_ptr->current_semaphore];
		wait_values[current_wait_object] = 0;
		wait_masks[current_wait_object] = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		current_wait_object++;
	}

	for (uint32_t i = 0; i < desc->num_command_buffers; ++i)
	{
		Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)desc->command_buffers[i]);
		assert(command_buffer_ptr);

		submit_command_buffers[i] = command_buffer_ptr->command_buffer;
	}

	uint32_t current_signal_object = 0;
	for (uint32_t i = 0; i < desc->num_signal_semaphores; ++i)
	{
		Vulkan_Semaphore *semaphore_ptr = (Vulkan_Semaphore *)opal_poolGetElement(&device_ptr->semaphores, (Opal_PoolHandle)desc->signal_semaphores[i]);
		assert(semaphore_ptr);

		signal_semaphores[current_signal_object] = semaphore_ptr->semaphore;
		signal_values[current_signal_object] = desc->signal_values[i];
		current_signal_object++;
	}

	for (uint32_t i = 0; i < desc->num_signal_swapchains; ++i)
	{
		Vulkan_Swapchain *swapchain_ptr = (Vulkan_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)desc->signal_swapchains[i]);
		assert(swapchain_ptr);

		signal_semaphores[current_signal_object] = swapchain_ptr->present_semaphores[swapchain_ptr->current_semaphore];
		signal_values[current_signal_object] = 0;
		current_signal_object++;
	}

	VkTimelineSemaphoreSubmitInfo timeline_submit_info = {0};
	timeline_submit_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO_KHR;
	timeline_submit_info.waitSemaphoreValueCount = num_wait_objects;
	timeline_submit_info.pWaitSemaphoreValues = wait_values;
	timeline_submit_info.signalSemaphoreValueCount = num_signal_objects;
	timeline_submit_info.pSignalSemaphoreValues = signal_values;

	VkSubmitInfo submit_info = {0};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = &timeline_submit_info;
	submit_info.waitSemaphoreCount = num_wait_objects;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_masks;
	submit_info.commandBufferCount = desc->num_command_buffers;
	submit_info.pCommandBuffers = submit_command_buffers;
	submit_info.signalSemaphoreCount = num_signal_objects;
	submit_info.pSignalSemaphores = signal_semaphores;

	VkResult result = device_ptr->vk.vkQueueSubmit(queue_ptr->queue, 1, &submit_info, VK_NULL_HANDLE);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceAcquire(Opal_Device this, Opal_Swapchain swapchain, Opal_TextureView *texture_view)
{
	assert(this);
 	assert(swapchain);
	assert(texture_view);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	Vulkan_Swapchain *swapchain_ptr = (Vulkan_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)swapchain);
	assert(swapchain_ptr);

	uint32_t semaphore_index = swapchain_ptr->current_semaphore + 1;
	semaphore_index %= swapchain_ptr->num_images;

	VkSemaphore vulkan_semaphore = swapchain_ptr->acquire_semaphores[semaphore_index];
	uint64_t timeout = 0;

	VkResult result = device_ptr->vk.vkAcquireNextImageKHR(vulkan_device, swapchain_ptr->swapchain, timeout, vulkan_semaphore, VK_NULL_HANDLE, &swapchain_ptr->current_image);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	assert(swapchain_ptr->current_image < swapchain_ptr->num_images);
	*texture_view = swapchain_ptr->texture_views[swapchain_ptr->current_image];

	swapchain_ptr->current_semaphore = semaphore_index;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_devicePresent(Opal_Device this, Opal_Swapchain swapchain)
{
	assert(this);
 	assert(swapchain);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_Swapchain *swapchain_ptr = (Vulkan_Swapchain *)opal_poolGetElement(&device_ptr->swapchains, (Opal_PoolHandle)swapchain);
	assert(swapchain_ptr);

	Vulkan_Queue *queue_ptr = (Vulkan_Queue *)opal_poolGetElement(&device_ptr->queues, (Opal_PoolHandle)swapchain_ptr->present_queue);
	assert(queue_ptr);

	uint32_t semaphore_index = swapchain_ptr->current_semaphore;
	VkSemaphore vulkan_semaphore = swapchain_ptr->present_semaphores[semaphore_index];

	VkPresentInfoKHR present_info = {0};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.swapchainCount = 1;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &vulkan_semaphore;
	present_info.pSwapchains = &swapchain_ptr->swapchain;
	present_info.pImageIndices = &swapchain_ptr->current_image;

	VkResult result = device_ptr->vk.vkQueuePresentKHR(queue_ptr->queue, &present_info);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depth_stencil_attachment)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	VkRenderingAttachmentInfo *vulkan_color_attachments = NULL;
	VkRenderingAttachmentInfo *vulkan_depth_stencil_attachment = NULL;

	uint32_t width = 0;
	uint32_t height = 0;

	opal_bumpReset(&device_ptr->bump);

	uint32_t color_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkRenderingAttachmentInfo) * num_color_attachments);
	uint32_t depth_stencil_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkRenderingAttachmentInfo));

	if (num_color_attachments > 0)
	{
		vulkan_color_attachments = (VkRenderingAttachmentInfo *)(device_ptr->bump.data + color_offset);
		memset(vulkan_color_attachments, 0, sizeof(VkRenderingAttachmentInfo) * num_color_attachments);

		for (uint32_t i = 0; i < num_color_attachments; ++i)
		{
			const Opal_FramebufferAttachment *opal_attachment = &color_attachments[i];
			VkRenderingAttachmentInfo *attachment = &vulkan_color_attachments[i];

			Vulkan_ImageView *image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, (Opal_PoolHandle)opal_attachment->texture_view);
			assert(image_view_ptr);

			Vulkan_ImageView *resolve_image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, (Opal_PoolHandle)opal_attachment->resolve_texture_view);

			width = max(width, image_view_ptr->width);
			height = max(height, image_view_ptr->height);

			attachment->sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			attachment->imageView = image_view_ptr->image_view;
			attachment->imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			if (resolve_image_view_ptr)
			{
				attachment->resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
				attachment->resolveImageView = resolve_image_view_ptr->image_view;
				attachment->resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			attachment->loadOp = vulkan_helperToLoadOp(opal_attachment->load_op);
			attachment->storeOp = vulkan_helperToStoreOp(opal_attachment->store_op);

			assert(sizeof(VkClearValue) == sizeof(Opal_ClearValue));
			memcpy(&attachment->clearValue, &opal_attachment->clear_value, sizeof(VkClearValue));
		}
	}

	if (depth_stencil_attachment)
	{
		vulkan_depth_stencil_attachment = (VkRenderingAttachmentInfo *)(device_ptr->bump.data + depth_stencil_offset);
		memset(vulkan_depth_stencil_attachment, 0, sizeof(VkRenderingAttachmentInfo));

		vulkan_depth_stencil_attachment->sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		Vulkan_ImageView *image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, (Opal_PoolHandle)depth_stencil_attachment->texture_view);
		assert(image_view_ptr);

		Vulkan_ImageView *resolve_image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, (Opal_PoolHandle)depth_stencil_attachment->resolve_texture_view);

		width = max(width, image_view_ptr->width);
		height = max(height, image_view_ptr->height);

		vulkan_depth_stencil_attachment->sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		vulkan_depth_stencil_attachment->imageView = image_view_ptr->image_view;
		vulkan_depth_stencil_attachment->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		if (resolve_image_view_ptr)
		{
			vulkan_depth_stencil_attachment->resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
			vulkan_depth_stencil_attachment->resolveImageView = resolve_image_view_ptr->image_view;
			vulkan_depth_stencil_attachment->resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		vulkan_depth_stencil_attachment->loadOp = vulkan_helperToLoadOp(depth_stencil_attachment->load_op);
		vulkan_depth_stencil_attachment->storeOp = vulkan_helperToStoreOp(depth_stencil_attachment->store_op);

		assert(sizeof(VkClearValue) == sizeof(Opal_ClearValue));
		memcpy(&vulkan_depth_stencil_attachment->clearValue, &depth_stencil_attachment->clear_value, sizeof(VkClearValue));
	}

	command_buffer_ptr->pipeline_bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkRenderingInfo rendering_info = {0};
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.renderArea.extent.width = width;
	rendering_info.renderArea.extent.height = height;
	rendering_info.layerCount = 1;
	rendering_info.colorAttachmentCount = num_color_attachments;
	rendering_info.pColorAttachments = vulkan_color_attachments;
	rendering_info.pDepthAttachment = vulkan_depth_stencil_attachment;
	rendering_info.pStencilAttachment = vulkan_depth_stencil_attachment;

	device_ptr->vk.vkCmdBeginRenderingKHR(command_buffer_ptr->command_buffer, &rendering_info);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	command_buffer_ptr->pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;

	device_ptr->vk.vkCmdEndRenderingKHR(command_buffer_ptr->command_buffer);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdBeginComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	command_buffer_ptr->pipeline_bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdEndComputePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	command_buffer_ptr->pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdBeginRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	command_buffer_ptr->pipeline_bind_point = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdEndRaytracePass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	command_buffer_ptr->pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdBeginCopyPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	command_buffer_ptr->pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdEndCopyPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	command_buffer_ptr->pipeline_bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdSetDescriptorHeap(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_DescriptorHeap descriptor_heap)
{
	assert(this);
	assert(command_buffer);
	assert(descriptor_heap);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	Vulkan_DescriptorHeap *descriptor_heap_ptr = (Vulkan_DescriptorHeap *)opal_poolGetElement(&device_ptr->descriptor_heaps, (Opal_PoolHandle)descriptor_heap);
	assert(descriptor_heap_ptr);

	VkDescriptorBufferBindingInfoEXT info = {0};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
	info.address = descriptor_heap_ptr->device_address;
	info.usage = descriptor_heap_ptr->usage;

	device_ptr->vk.vkCmdBindDescriptorBuffersEXT(command_buffer_ptr->command_buffer, 1, &info);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdSetPipelineLayout(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	command_buffer_ptr->pipeline_layout = pipeline_layout;
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdSetPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Pipeline pipeline)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	Vulkan_Pipeline *pipeline_ptr = (Vulkan_Pipeline *)opal_poolGetElement(&device_ptr->pipelines, (Opal_PoolHandle)pipeline);
	assert(pipeline_ptr);
	assert(command_buffer_ptr->pipeline_bind_point == pipeline_ptr->bind_point);

	device_ptr->vk.vkCmdBindPipeline(command_buffer_ptr->command_buffer, command_buffer_ptr->pipeline_bind_point, pipeline_ptr->pipeline);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdSetDescriptorSet(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets)
{
	assert(this);
	assert(command_buffer);
	assert(descriptor_set);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);
	assert(command_buffer_ptr->pipeline_layout);

	Vulkan_PipelineLayout *pipeline_layout_ptr = (Vulkan_PipelineLayout *)opal_poolGetElement(&device_ptr->pipeline_layouts, (Opal_PoolHandle)command_buffer_ptr->pipeline_layout);
	assert(pipeline_layout_ptr);

	Vulkan_DescriptorSet *descriptor_set_ptr = (Vulkan_DescriptorSet *)opal_poolGetElement(&device_ptr->descriptor_sets, (Opal_PoolHandle)descriptor_set);
	assert(descriptor_set_ptr);

	Vulkan_DescriptorSetLayout *descriptor_set_layout_ptr = (Vulkan_DescriptorSetLayout *)opal_poolGetElement(&device_ptr->descriptor_set_layouts, (Opal_PoolHandle)descriptor_set_ptr->layout);
	assert(descriptor_set_layout_ptr);

	VkCommandBuffer vulkan_command_buffer = command_buffer_ptr->command_buffer;
	VkPipelineBindPoint vulkan_bind_point = command_buffer_ptr->pipeline_bind_point;
	VkPipelineLayout vulkan_pipeline_layout = pipeline_layout_ptr->layout;

	if (descriptor_set_ptr->num_static_descriptors > 0)
	{
		uint32_t buffer_index = 0;
		VkDeviceSize buffer_offset = descriptor_set_ptr->allocation.offset * device_ptr->descriptor_buffer_properties.descriptorBufferOffsetAlignment;

		device_ptr->vk.vkCmdSetDescriptorBufferOffsetsEXT(vulkan_command_buffer, vulkan_bind_point, vulkan_pipeline_layout, index, 1, &buffer_index, &buffer_offset);
	}

	if (num_dynamic_offsets > 0)
	{
		assert(dynamic_offsets);
		assert(pipeline_layout_ptr->num_dynamic_descriptors == num_dynamic_offsets);
		assert(descriptor_set_layout_ptr->num_dynamic_descriptors == descriptor_set_ptr->num_dynamic_descriptors);

		uint32_t offset = descriptor_set_layout_ptr->num_static_descriptors;

		for (uint32_t i = 0; i < num_dynamic_offsets; ++i)
		{
			const VkDescriptorSetLayoutBinding *info = &descriptor_set_layout_ptr->entries[offset + i];
			const Opal_DescriptorSetEntry *data = &descriptor_set_ptr->dynamic_descriptors[i];

			VkDescriptorBufferInfo buffer_info = {0};
			switch (info->descriptorType)
			{
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				{
					Opal_BufferView buffer_view = data->data.buffer_view;
					Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer_view.buffer);
					assert(buffer_ptr);

					buffer_info.buffer = buffer_ptr->buffer;
					buffer_info.offset = buffer_view.offset + dynamic_offsets[i];
					buffer_info.range = buffer_view.size;
				}
				break;

				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				{
					Opal_StorageBufferView buffer_view = data->data.storage_buffer_view;
					Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer_view.buffer);
					assert(buffer_ptr);

					buffer_info.buffer = buffer_ptr->buffer;
					buffer_info.offset = buffer_view.offset + dynamic_offsets[i];
					buffer_info.range = buffer_view.element_size * buffer_view.num_elements;
				}
				break;

				default: assert(0); break;
			}

			VkWriteDescriptorSet descriptor_write = {0};
			descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write.dstBinding = info->binding;
			descriptor_write.dstSet = descriptor_set_ptr->set;
			descriptor_write.descriptorCount = info->descriptorCount;
			descriptor_write.descriptorType = info->descriptorType;
			descriptor_write.pBufferInfo = &buffer_info;

			device_ptr->vk.vkCmdPushDescriptorSetKHR(vulkan_command_buffer, vulkan_bind_point, vulkan_pipeline_layout, index, 1, &descriptor_write);
		}
	}

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_VertexBufferView *vertex_buffers)
{
	assert(this);
	assert(command_buffer);
	assert(num_vertex_buffers > 0);
	assert(vertex_buffers);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	opal_bumpReset(&device_ptr->bump);
	uint32_t buffers_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkBuffer) * num_vertex_buffers);
	uint32_t offsets_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkDeviceSize) * num_vertex_buffers);

	VkBuffer *buffers = (VkBuffer *)(device_ptr->bump.data + buffers_offset);
	VkDeviceSize *offsets = (VkDeviceSize *)(device_ptr->bump.data + offsets_offset);

	for (uint32_t i = 0; i < num_vertex_buffers; ++i)
	{
		Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)vertex_buffers[i].buffer);
		assert(buffer_ptr);

		buffers[i] = buffer_ptr->buffer;
		offsets[i] = vertex_buffers[i].offset;
	}
	
	device_ptr->vk.vkCmdBindVertexBuffers(command_buffer_ptr->command_buffer, 0, num_vertex_buffers, buffers, offsets);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_IndexBufferView index_buffer)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)index_buffer.buffer);
	assert(buffer_ptr);

	VkIndexType index_type = vulkan_helperToIndexType(index_buffer.format);

	device_ptr->vk.vkCmdBindIndexBuffer(command_buffer_ptr->command_buffer, buffer_ptr->buffer, index_buffer.offset, index_type);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdSetViewport(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Viewport viewport)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	VkViewport vulkan_viewport = {0};
	vulkan_viewport.x = viewport.x;
	vulkan_viewport.y = viewport.y;
	vulkan_viewport.width = viewport.width;
	vulkan_viewport.height = viewport.height;
	vulkan_viewport.minDepth = viewport.min_depth;
	vulkan_viewport.maxDepth = viewport.max_depth;

	device_ptr->vk.vkCmdSetViewport(command_buffer_ptr->command_buffer, 0, 1, &vulkan_viewport);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdSetScissor(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	VkRect2D rect = {0};
	rect.offset.x = x;
	rect.offset.y = y;
	rect.extent.width = width;
	rect.extent.height = height;

	device_ptr->vk.vkCmdSetScissor(command_buffer_ptr->command_buffer, 0, 1, &rect);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdDraw(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertices, uint32_t num_instances, uint32_t base_vertex, uint32_t base_instance)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	device_ptr->vk.vkCmdDraw(command_buffer_ptr->command_buffer, num_vertices, num_instances, base_vertex, base_instance);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdDrawIndexed(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t num_instances, uint32_t base_index, int32_t vertex_offset, uint32_t base_instance)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	device_ptr->vk.vkCmdDrawIndexed(command_buffer_ptr->command_buffer, num_indices, num_instances, base_index, vertex_offset, base_instance);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdMeshletDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	device_ptr->vk.vkCmdDrawMeshTasksEXT(command_buffer_ptr->command_buffer, num_groups_x, num_groups_y, num_groups_z);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdComputeDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	device_ptr->vk.vkCmdDispatch(command_buffer_ptr->command_buffer, num_groups_x, num_groups_y, num_groups_z);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdRaytraceDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView raygen_entry, Opal_BufferView hitgroup_entry, Opal_BufferView miss_entry, uint32_t width, uint32_t height, uint32_t depth)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	uint32_t base_alignment = device_ptr->raytrace_properties.shaderGroupBaseAlignment;
	uint32_t handle_size = device_ptr->raytrace_properties.shaderGroupHandleSize;
	uint32_t handle_alignment = device_ptr->raytrace_properties.shaderGroupHandleAlignment;

	uint32_t aligned_handle_size = alignUp(handle_size, handle_alignment);

	VkStridedDeviceAddressRegionKHR vulkan_raygen_entry = {0};
	VkStridedDeviceAddressRegionKHR vulkan_miss_entry = {0};
	VkStridedDeviceAddressRegionKHR vulkan_hitgroup_entry = {0};
	VkStridedDeviceAddressRegionKHR vulkan_callable_entry = {0};

	Vulkan_Buffer *raygen_sbt_buffer_ptr = opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)raygen_entry.buffer);
	if (raygen_sbt_buffer_ptr)
	{
		assert(isAlignedul(raygen_entry.offset, base_alignment));

		vulkan_raygen_entry.deviceAddress = raygen_sbt_buffer_ptr->device_address + raygen_entry.offset;
		vulkan_raygen_entry.size = aligned_handle_size;
		vulkan_raygen_entry.stride = aligned_handle_size;
	}

	Vulkan_Buffer *miss_sbt_buffer_ptr = opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)miss_entry.buffer);
	if (miss_sbt_buffer_ptr)
	{
		assert(isAlignedul(miss_entry.offset, base_alignment));

		vulkan_miss_entry.deviceAddress = miss_sbt_buffer_ptr->device_address + miss_entry.offset;
		vulkan_miss_entry.size = aligned_handle_size;
		vulkan_miss_entry.stride = aligned_handle_size;
	}

	Vulkan_Buffer *hitgroup_sbt_buffer_ptr = opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)hitgroup_entry.buffer);
	if (hitgroup_sbt_buffer_ptr)
	{
		assert(isAlignedul(hitgroup_entry.offset, base_alignment));

		vulkan_hitgroup_entry.deviceAddress = hitgroup_sbt_buffer_ptr->device_address + hitgroup_entry.offset;
		vulkan_hitgroup_entry.size = aligned_handle_size;
		vulkan_hitgroup_entry.stride = aligned_handle_size;
	}

	OPAL_UNUSED(base_alignment);

	device_ptr->vk.vkCmdTraceRaysKHR(command_buffer_ptr->command_buffer, &vulkan_raygen_entry, &vulkan_miss_entry, &vulkan_hitgroup_entry, &vulkan_callable_entry, width, height, depth);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdBuildAccelerationStructures(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_build_descs, const Opal_AccelerationStructureBuildDesc *descs)
{
	assert(this);
	assert(command_buffer);
	assert(num_build_descs > 0);
	assert(descs);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	VkCommandBuffer vulkan_command_buffer = command_buffer_ptr->command_buffer;

	uint32_t num_entries = 0;

	for (uint32_t i = 0; i < num_build_descs; ++i)
	{
		const Opal_AccelerationStructureBuildDesc *desc = &descs[i];
		uint32_t num_desc_entries = 1;
		if (desc->type == OPAL_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
			num_desc_entries = desc->input.bottom_level.num_geometries;

		num_entries += num_desc_entries;
	}

	opal_bumpReset(&device_ptr->bump);
	uint32_t entries_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkAccelerationStructureGeometryKHR) * num_entries);
	uint32_t build_ranges_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkAccelerationStructureBuildRangeInfoKHR) * num_entries);
	uint32_t build_ranges_ptrs_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkAccelerationStructureBuildRangeInfoKHR*) * num_build_descs);
	uint32_t build_infos_offset = opal_bumpAlloc(&device_ptr->bump, sizeof(VkAccelerationStructureBuildGeometryInfoKHR) * num_build_descs);

	VkAccelerationStructureGeometryKHR *entries = (VkAccelerationStructureGeometryKHR *)(device_ptr->bump.data + entries_offset);
	memset(entries, 0, sizeof(VkAccelerationStructureGeometryKHR) * num_entries);

	VkAccelerationStructureBuildRangeInfoKHR *build_ranges = (VkAccelerationStructureBuildRangeInfoKHR *)(device_ptr->bump.data + build_ranges_offset);
	memset(build_ranges, 0, sizeof(VkAccelerationStructureBuildRangeInfoKHR) * num_entries);

	VkAccelerationStructureBuildRangeInfoKHR **build_ranges_ptrs = (VkAccelerationStructureBuildRangeInfoKHR **)(device_ptr->bump.data + build_ranges_ptrs_offset);
	memset(build_ranges, 0, sizeof(VkAccelerationStructureBuildRangeInfoKHR *) * num_build_descs);

	VkAccelerationStructureBuildGeometryInfoKHR *build_infos = (VkAccelerationStructureBuildGeometryInfoKHR *)(device_ptr->bump.data + build_infos_offset);
	memset(build_ranges, 0, sizeof(VkAccelerationStructureBuildGeometryInfoKHR) * num_build_descs);

	uint32_t current_entry = 0;
	uint32_t current_build_range = 0;

	for (uint32_t i = 0; i < num_build_descs; ++i)
	{
		const Opal_AccelerationStructureBuildDesc *desc = &descs[i];

		const Vulkan_AccelerationStructure *dst_acceleration_structure_ptr = opal_poolGetElement(&device_ptr->acceleration_structures, (Opal_PoolHandle)desc->dst_acceleration_structure);
		assert(dst_acceleration_structure_ptr);

		const Vulkan_Buffer *scratch_buffer_ptr = opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)desc->scratch_buffer.buffer);
		assert(scratch_buffer_ptr);

		VkAccelerationStructureBuildGeometryInfoKHR *build_info = &build_infos[i];
		build_info->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		build_info->type = vulkan_helperToAccelerationStructureType(desc->type);
		build_info->flags = vulkan_helperToAccelerationStructureBuildFlags(desc->build_flags);
		build_info->mode = vulkan_helperToAccelerationStructureBuildMode(desc->build_mode);
		build_info->dstAccelerationStructure = dst_acceleration_structure_ptr->acceleration_structure;

		if (desc->src_acceleration_structure)
		{
			const Vulkan_AccelerationStructure *src_acceleration_structure_ptr = opal_poolGetElement(&device_ptr->acceleration_structures, (Opal_PoolHandle)desc->src_acceleration_structure);
			assert(src_acceleration_structure_ptr);

			build_info->srcAccelerationStructure = src_acceleration_structure_ptr->acceleration_structure;
		}

		build_info->pGeometries = &entries[current_entry];
		build_info->geometryCount = 1;
		if (desc->type == OPAL_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
			build_info->geometryCount = desc->input.bottom_level.num_geometries;

		build_info->scratchData.deviceAddress = scratch_buffer_ptr->device_address + desc->scratch_buffer.offset;

		build_ranges_ptrs[i] = &build_ranges[current_build_range];

		if (desc->type == OPAL_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
		{
			for (uint32_t j = 0; j < build_info->geometryCount; ++j)
			{
				const Opal_AccelerationStructureGeometry *opal_geometry = &desc->input.bottom_level.geometries[j];
				VkAccelerationStructureGeometryKHR *geometry = &entries[current_entry];
				VkAccelerationStructureBuildRangeInfoKHR *build_range = &build_ranges[current_build_range];

				geometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
				geometry->geometryType = vulkan_helperToAccelerationStructureGeometryType(opal_geometry->type);
				geometry->flags = vulkan_helperToAccelerationStructureGeometryFlags(opal_geometry->flags);

				if (opal_geometry->type == OPAL_ACCELERATION_STRUCTURE_GEOMETRY_TYPE_TRIANGLES)
				{
					const Opal_AccelerationStructureGeometryDataTriangles *opal_triangles = &opal_geometry->data.triangles;
					VkAccelerationStructureGeometryTrianglesDataKHR *triangles = &geometry->geometry.triangles;

					const Vulkan_Buffer *vertex_buffer_ptr = opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)opal_triangles->vertex_buffer.buffer);
					assert(vertex_buffer_ptr);

					triangles->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
					triangles->vertexFormat = vulkan_helperToVertexFormat(opal_triangles->vertex_format);
					triangles->vertexData.deviceAddress = vertex_buffer_ptr->device_address + opal_triangles->vertex_buffer.offset;
					triangles->vertexStride = opal_triangles->vertex_stride;
					triangles->maxVertex = opal_triangles->num_vertices - 1;
					triangles->indexType = vulkan_helperToIndexType(opal_triangles->index_format);

					build_range->primitiveCount = opal_triangles->num_vertices / 3;

					if (opal_triangles->index_buffer.buffer != OPAL_NULL_HANDLE)
					{
						const Vulkan_Buffer *index_buffer_ptr = opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)opal_triangles->index_buffer.buffer);
						assert(index_buffer_ptr);

						triangles->indexData.deviceAddress = index_buffer_ptr->device_address + opal_triangles->index_buffer.offset;

						build_range->primitiveCount = opal_triangles->num_indices / 3;
					}

					if (opal_triangles->transform_buffer.buffer != OPAL_NULL_HANDLE)
					{
						const Vulkan_Buffer *transform_buffer_ptr = opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)opal_triangles->transform_buffer.buffer);
						assert(transform_buffer_ptr);

						triangles->transformData.deviceAddress = transform_buffer_ptr->device_address + opal_triangles->transform_buffer.offset;
					}
				}
				else if (opal_geometry->type == OPAL_ACCELERATION_STRUCTURE_GEOMETRY_TYPE_AABBS)
				{
					const Opal_AccelerationStructureGeometryDataAABBs *opal_aabbs = &opal_geometry->data.aabbs;
					VkAccelerationStructureGeometryAabbsDataKHR *aabbs = &geometry->geometry.aabbs;

					Vulkan_Buffer *entries_buffer_ptr = opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)opal_aabbs->entries_buffer.buffer);
					assert(entries_buffer_ptr);

					aabbs->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
					aabbs->stride = opal_aabbs->stride;
					aabbs->data.deviceAddress = entries_buffer_ptr->device_address + opal_aabbs->entries_buffer.offset;

					build_range->primitiveCount = opal_aabbs->num_entries;
				}
				else
					assert(0);

				current_entry++;
				current_build_range++;
			}
		}
		else if (desc->type == OPAL_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL)
		{
			VkAccelerationStructureGeometryKHR *geometry = &entries[current_entry];
			VkAccelerationStructureBuildRangeInfoKHR *build_range = &build_ranges[current_build_range];
			const Opal_AccelerationStructureBuildInputTopLevel *input = &desc->input.top_level;

			Vulkan_Buffer *instances_buffer_ptr = opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)input->instance_buffer.buffer);
			assert(instances_buffer_ptr);

			geometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			geometry->geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;

			VkAccelerationStructureGeometryInstancesDataKHR *instances = &geometry->geometry.instances;
			instances->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
			instances->arrayOfPointers = VK_FALSE;
			instances->data.deviceAddress = instances_buffer_ptr->device_address + input->instance_buffer.offset;

			build_range->primitiveCount = input->num_instances;

			current_entry++;
			current_build_range++;
		}
		else
			assert(0);
	}

	device_ptr->vk.vkCmdBuildAccelerationStructuresKHR(vulkan_command_buffer, num_build_descs, build_infos, build_ranges_ptrs);

	for (uint32_t i = 0; i < num_build_descs; ++i)
	{
		const Opal_AccelerationStructureBuildDesc *desc = &descs[i];

		Vulkan_AccelerationStructure *dst_acceleration_structure_ptr = opal_poolGetElement(&device_ptr->acceleration_structures, (Opal_PoolHandle)desc->dst_acceleration_structure);
		assert(dst_acceleration_structure_ptr);

		dst_acceleration_structure_ptr->allow_compaction = (desc->build_flags & OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_ALLOW_COMPACTION) != 0;

		VkAccelerationStructureKHR acceleration_structure = dst_acceleration_structure_ptr->acceleration_structure;

		VkQueryPool size_pool = dst_acceleration_structure_ptr->size_pool;
		VkQueryPool serialization_size_pool = dst_acceleration_structure_ptr->serialization_size_pool;
		VkQueryPool compacted_size_pool = dst_acceleration_structure_ptr->compacted_size_pool;

		device_ptr->vk.vkCmdResetQueryPool(vulkan_command_buffer, size_pool, 0, 1);
		device_ptr->vk.vkCmdResetQueryPool(vulkan_command_buffer, serialization_size_pool, 0, 1);
		device_ptr->vk.vkCmdResetQueryPool(vulkan_command_buffer, compacted_size_pool, 0, 1);

		device_ptr->vk.vkCmdWriteAccelerationStructuresPropertiesKHR(vulkan_command_buffer, 1, &acceleration_structure, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR, size_pool, 0);
		device_ptr->vk.vkCmdWriteAccelerationStructuresPropertiesKHR(vulkan_command_buffer, 1, &acceleration_structure, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR, serialization_size_pool, 0);

		if (dst_acceleration_structure_ptr->allow_compaction)
			device_ptr->vk.vkCmdWriteAccelerationStructuresPropertiesKHR(vulkan_command_buffer, 1, &acceleration_structure, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, compacted_size_pool, 0);
	}

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdCopyAccelerationStructure(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_AccelerationStructure src, Opal_AccelerationStructure dst, Opal_AccelerationStructureCopyMode mode)
{
	assert(this);
	assert(command_buffer);
	assert(src);
	assert(dst);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	const Vulkan_AccelerationStructure *src_ptr = opal_poolGetElement(&device_ptr->acceleration_structures, (Opal_PoolHandle)src);
	assert(src_ptr);

	const Vulkan_AccelerationStructure *dst_ptr = opal_poolGetElement(&device_ptr->acceleration_structures, (Opal_PoolHandle)dst);
	assert(dst_ptr);

	VkCopyAccelerationStructureInfoKHR copy_info = {0};
	copy_info.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR;
	copy_info.src = src_ptr->acceleration_structure;
	copy_info.dst = dst_ptr->acceleration_structure;
	copy_info.mode = vulkan_helperToAccelerationStructureCopyMode(mode);

	device_ptr->vk.vkCmdCopyAccelerationStructureKHR(command_buffer_ptr->command_buffer, &copy_info);

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdCopyAccelerationStructuresPostbuildInfo(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_src_acceleration_structures, const Opal_AccelerationStructure *src_acceleration_structures, Opal_BufferView dst_buffer)
{
	assert(this);
	assert(command_buffer);
	assert(num_src_acceleration_structures > 0);
	assert(src_acceleration_structures);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	VkCommandBuffer vulkan_command_buffer = command_buffer_ptr->command_buffer;

	Vulkan_Buffer *dst_buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)dst_buffer.buffer);
	assert(dst_buffer_ptr);

	VkBuffer vulkan_buffer = dst_buffer_ptr->buffer;

	VkDeviceSize stride = sizeof(Opal_AccelerationStructurePostbuildInfo);
	VkQueryResultFlagBits query_result_flags = VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT;

	VkDeviceSize size_offset = offsetof(Opal_AccelerationStructurePostbuildInfo, acceleration_structure_size);
	VkDeviceSize compacted_size_offset = offsetof(Opal_AccelerationStructurePostbuildInfo, compacted_buffer_size);
	VkDeviceSize serialization_size_offset = offsetof(Opal_AccelerationStructurePostbuildInfo, serialization_buffer_size);

	for (uint32_t i = 0; i < num_src_acceleration_structures; ++i)
	{
		VkDeviceSize base_offset = dst_buffer.offset + stride * i;

		Vulkan_AccelerationStructure *acceleration_structure_ptr = opal_poolGetElement(&device_ptr->acceleration_structures, (Opal_PoolHandle)src_acceleration_structures[i]);
		assert(acceleration_structure_ptr);

		VkQueryPool size_pool = acceleration_structure_ptr->size_pool;
		VkQueryPool serialization_size_pool = acceleration_structure_ptr->serialization_size_pool;
		VkQueryPool compacted_size_pool = acceleration_structure_ptr->compacted_size_pool;

		device_ptr->vk.vkCmdCopyQueryPoolResults(vulkan_command_buffer, size_pool, 0, 1, vulkan_buffer, base_offset + size_offset, 0, query_result_flags);
		device_ptr->vk.vkCmdCopyQueryPoolResults(vulkan_command_buffer, serialization_size_pool, 0, 1, vulkan_buffer, base_offset + serialization_size_offset, 0, query_result_flags);

		if (acceleration_structure_ptr->allow_compaction)
			device_ptr->vk.vkCmdCopyQueryPoolResults(vulkan_command_buffer, compacted_size_pool, 0, 1, vulkan_buffer, base_offset + compacted_size_offset, 0, query_result_flags);
	}

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdCopyBufferToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_Buffer src_buffer, uint64_t src_offset, Opal_Buffer dst_buffer, uint64_t dst_offset, uint64_t size)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	Vulkan_Buffer *src_buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)src_buffer);
	assert(src_buffer_ptr);

	Vulkan_Buffer *dst_buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)dst_buffer);
	assert(dst_buffer_ptr);

	VkBufferCopy copy_region = {0};
	copy_region.srcOffset = src_offset;
	copy_region.dstOffset = dst_offset;
	copy_region.size = size;

	device_ptr->vk.vkCmdCopyBuffer(command_buffer_ptr->command_buffer, src_buffer_ptr->buffer, dst_buffer_ptr->buffer, 1, &copy_region);

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferTextureRegion src, Opal_TextureRegion dst, Opal_Extent3D size)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	Vulkan_Buffer *src_buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)src.buffer);
	assert(src_buffer_ptr);

	Vulkan_ImageView *dst_image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, (Opal_PoolHandle)dst.texture_view);
	assert(dst_image_view_ptr);

	VkBufferImageCopy copy_region = {0};
	copy_region.bufferOffset = src.offset;
	copy_region.imageExtent.width = size.width;
	copy_region.imageExtent.height = size.height;
	copy_region.imageExtent.depth = size.depth;
	copy_region.imageOffset.x = dst.offset.x;
	copy_region.imageOffset.z = dst.offset.y;
	copy_region.imageOffset.y = dst.offset.z;
	copy_region.imageSubresource.aspectMask = dst_image_view_ptr->aspect_mask;
	copy_region.imageSubresource.mipLevel = dst_image_view_ptr->base_mip;
	copy_region.imageSubresource.baseArrayLayer = dst_image_view_ptr->base_layer;
	copy_region.imageSubresource.layerCount = dst_image_view_ptr->num_layers;

	device_ptr->vk.vkCmdCopyBufferToImage(command_buffer_ptr->command_buffer, src_buffer_ptr->buffer, dst_image_view_ptr->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdCopyTextureToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferTextureRegion dst, Opal_Extent3D size)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	Vulkan_ImageView *src_image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, (Opal_PoolHandle)src.texture_view);
	assert(src_image_view_ptr);

	Vulkan_Buffer *dst_buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)dst.buffer);
	assert(dst_buffer_ptr);

	VkBufferImageCopy copy_region = {0};
	copy_region.bufferOffset = dst.offset;
	copy_region.imageExtent.width = size.width;
	copy_region.imageExtent.height = size.height;
	copy_region.imageExtent.depth = size.depth;
	copy_region.imageOffset.x = src.offset.x;
	copy_region.imageOffset.z = src.offset.y;
	copy_region.imageOffset.y = src.offset.z;
	copy_region.imageSubresource.aspectMask = src_image_view_ptr->aspect_mask;
	copy_region.imageSubresource.mipLevel = src_image_view_ptr->base_mip;
	copy_region.imageSubresource.baseArrayLayer = src_image_view_ptr->base_layer;
	copy_region.imageSubresource.layerCount = src_image_view_ptr->num_layers;

	device_ptr->vk.vkCmdCopyImageToBuffer(command_buffer_ptr->command_buffer, src_image_view_ptr->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_buffer_ptr->buffer, 1, &copy_region);

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdCopyTextureToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_TextureRegion dst, Opal_Extent3D size)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	Vulkan_ImageView *src_image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, (Opal_PoolHandle)src.texture_view);
	assert(src_image_view_ptr);

	Vulkan_ImageView *dst_image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, (Opal_PoolHandle)dst.texture_view);
	assert(dst_image_view_ptr);

	VkImageCopy copy_region = {0};
	copy_region.extent.width = size.width;
	copy_region.extent.height = size.height;
	copy_region.extent.depth = size.depth;

	copy_region.srcOffset.x = src.offset.x;
	copy_region.srcOffset.z = src.offset.y;
	copy_region.srcOffset.y = src.offset.z;

	copy_region.srcSubresource.aspectMask = src_image_view_ptr->aspect_mask;
	copy_region.srcSubresource.mipLevel = src_image_view_ptr->base_mip;
	copy_region.srcSubresource.baseArrayLayer = src_image_view_ptr->base_layer;
	copy_region.srcSubresource.layerCount = src_image_view_ptr->num_layers;

	copy_region.dstOffset.x = dst.offset.x;
	copy_region.dstOffset.z = dst.offset.y;
	copy_region.dstOffset.y = dst.offset.z;

	copy_region.dstSubresource.aspectMask = dst_image_view_ptr->aspect_mask;
	copy_region.dstSubresource.mipLevel = dst_image_view_ptr->base_mip;
	copy_region.dstSubresource.baseArrayLayer = dst_image_view_ptr->base_layer;
	copy_region.dstSubresource.layerCount = dst_image_view_ptr->num_layers;

	device_ptr->vk.vkCmdCopyImage(command_buffer_ptr->command_buffer, src_image_view_ptr->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image_view_ptr->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdBufferTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	Vulkan_Buffer *buffer_ptr = (Vulkan_Buffer *)opal_poolGetElement(&device_ptr->buffers, (Opal_PoolHandle)buffer.buffer);
	assert(buffer_ptr);

	VkPipelineStageFlags wait_stage = vulkan_helperToPipelineWaitStage(state_before);
	VkPipelineStageFlags block_stage = vulkan_helperToPipelineBlockStage(state_after);

	VkBufferMemoryBarrier buffer_barrier = {0};
	buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	buffer_barrier.srcAccessMask = vulkan_helperToFlushAccessMask(state_before);
	buffer_barrier.dstAccessMask = vulkan_helperToInvalidateAccessMask(state_after);
	buffer_barrier.buffer = buffer_ptr->buffer;
	buffer_barrier.offset = buffer.offset;
	buffer_barrier.size = buffer.size;

	device_ptr->vk.vkCmdPipelineBarrier(command_buffer_ptr->command_buffer, wait_stage, block_stage, 0, 0, NULL, 1, &buffer_barrier, 0, NULL);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdBufferQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	assert(this);
	assert(command_buffer);
	assert(buffer.buffer);
	assert(queue);

	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdBufferQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	assert(this);
	assert(command_buffer);
	assert(buffer.buffer);
	assert(queue);

	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(buffer);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdTextureTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	assert(this);
	assert(command_buffer);
 
	Vulkan_Device *device_ptr = (Vulkan_Device *)this;

	Vulkan_CommandBuffer *command_buffer_ptr = (Vulkan_CommandBuffer *)opal_poolGetElement(&device_ptr->command_buffers, (Opal_PoolHandle)command_buffer);
	assert(command_buffer_ptr);

	Vulkan_ImageView *image_view_ptr = (Vulkan_ImageView *)opal_poolGetElement(&device_ptr->image_views, (Opal_PoolHandle)texture_view);
	assert(image_view_ptr);

	VkPipelineStageFlags wait_stage = vulkan_helperToPipelineWaitStage(state_before);
	VkPipelineStageFlags block_stage = vulkan_helperToPipelineBlockStage(state_after);

	VkImageMemoryBarrier image_barrier = {0};
	image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_barrier.srcAccessMask = vulkan_helperToFlushAccessMask(state_before);
	image_barrier.dstAccessMask = vulkan_helperToInvalidateAccessMask(state_after);
	image_barrier.oldLayout = vulkan_helperToImageLayoutTransition(state_before, image_view_ptr->aspect_mask);
	image_barrier.newLayout = vulkan_helperToImageLayoutTransition(state_after, image_view_ptr->aspect_mask);
	image_barrier.image = image_view_ptr->image;
	image_barrier.subresourceRange.aspectMask = image_view_ptr->aspect_mask;
	image_barrier.subresourceRange.baseMipLevel = image_view_ptr->base_mip;
	image_barrier.subresourceRange.levelCount = image_view_ptr->num_mips;
	image_barrier.subresourceRange.baseArrayLayer = image_view_ptr->base_layer;
	image_barrier.subresourceRange.layerCount = image_view_ptr->num_layers;

	device_ptr->vk.vkCmdPipelineBarrier(command_buffer_ptr->command_buffer, wait_stage, block_stage, 0, 0, NULL, 0, NULL, 1, &image_barrier);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCmdTextureQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	assert(this);
	assert(command_buffer);
	assert(texture_view);
	assert(queue);

	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdTextureQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	assert(this);
	assert(command_buffer);
	assert(texture_view);
	assert(queue);

	OPAL_UNUSED(this);
	OPAL_UNUSED(command_buffer);
	OPAL_UNUSED(texture_view);
	OPAL_UNUSED(queue);

	return OPAL_NOT_SUPPORTED;
}

/*
 */
static Opal_DeviceTable device_vtbl =
{
	vulkan_deviceGetInfo,
	vulkan_deviceGetQueue,
	vulkan_deviceGetAccelerationStructurePrebuildInfo,
	vulkan_deviceGetShaderBindingTablePrebuildInfo,

	vulkan_deviceGetSupportedSurfaceFormats,
	vulkan_deviceGetSupportedPresentModes,
	vulkan_deviceGetPreferredSurfaceFormat,
	vulkan_deviceGetPreferredSurfacePresentMode,

	vulkan_deviceCreateSemaphore,
	vulkan_deviceCreateBuffer,
	vulkan_deviceCreateTexture,
	vulkan_deviceCreateTextureView,
	vulkan_deviceCreateSampler,
	vulkan_deviceCreateAccelerationStructure,
	vulkan_deviceCreateCommandAllocator,
	vulkan_deviceCreateCommandBuffer,
	vulkan_deviceCreateShader,
	vulkan_deviceCreateDescriptorHeap,
	vulkan_deviceCreateDescriptorSetLayout,
	vulkan_deviceCreatePipelineLayout,
	vulkan_deviceCreateGraphicsPipeline,
	vulkan_deviceCreateMeshletPipeline,
	vulkan_deviceCreateComputePipeline,
	vulkan_deviceCreateRaytracePipeline,
	vulkan_deviceCreateSwapchain,

	vulkan_deviceDestroySemaphore,
	vulkan_deviceDestroyBuffer,
	vulkan_deviceDestroyTexture,
	vulkan_deviceDestroyTextureView,
	vulkan_deviceDestroySampler,
	vulkan_deviceDestroyAccelerationStructure,
	vulkan_deviceDestroyCommandAllocator,
	vulkan_deviceDestroyCommandBuffer,
	vulkan_deviceDestroyShader,
	vulkan_deviceDestroyDescriptorHeap,
	vulkan_deviceDestroyDescriptorSetLayout,
	vulkan_deviceDestroyPipelineLayout,
	vulkan_deviceDestroyPipeline,
	vulkan_deviceDestroySwapchain,
	vulkan_deviceDestroy,

	vulkan_deviceBuildShaderBindingTable,
	vulkan_deviceBuildAccelerationStructureInstanceBuffer,
	vulkan_deviceResetCommandAllocator,
	vulkan_deviceAllocateDescriptorSet,
	vulkan_deviceFreeDescriptorSet,
	vulkan_deviceMapBuffer,
	vulkan_deviceUnmapBuffer,
	vulkan_deviceWriteBuffer,
	vulkan_deviceUpdateDescriptorSet,
	vulkan_deviceBeginCommandBuffer,
	vulkan_deviceEndCommandBuffer,
	vulkan_deviceQuerySemaphore,
	vulkan_deviceSignalSemaphore,
	vulkan_deviceWaitSemaphore,
	vulkan_deviceWaitQueue,
	vulkan_deviceWaitIdle,
	vulkan_deviceSubmit,
	vulkan_deviceAcquire,
	vulkan_devicePresent,

	vulkan_deviceCmdBeginGraphicsPass,
	vulkan_deviceCmdEndGraphicsPass,
	vulkan_deviceCmdBeginComputePass,
	vulkan_deviceCmdEndComputePass,
	vulkan_deviceCmdBeginRaytracePass,
	vulkan_deviceCmdEndRaytracePass,
	vulkan_deviceCmdBeginCopyPass,
	vulkan_deviceCmdEndCopyPass,
	vulkan_deviceCmdSetDescriptorHeap,
	vulkan_deviceCmdSetPipelineLayout,
	vulkan_deviceCmdSetPipeline,
	vulkan_deviceCmdSetDescriptorSet,
	vulkan_deviceCmdSetVertexBuffers,
	vulkan_deviceCmdSetIndexBuffer,
	vulkan_deviceCmdSetViewport,
	vulkan_deviceCmdSetScissor,
	vulkan_deviceCmdDraw,
	vulkan_deviceCmdDrawIndexed,
	vulkan_deviceCmdMeshletDispatch,
	vulkan_deviceCmdComputeDispatch,
	vulkan_deviceCmdRaytraceDispatch,
	vulkan_deviceCmdBuildAccelerationStructures,
	vulkan_deviceCmdCopyAccelerationStructure,
	vulkan_deviceCmdCopyAccelerationStructuresPostbuildInfo,
	vulkan_deviceCmdCopyBufferToBuffer,
	vulkan_deviceCmdCopyBufferToTexture,
	vulkan_deviceCmdCopyTextureToBuffer,
	vulkan_deviceCmdCopyTextureToTexture,
	vulkan_deviceCmdBufferTransitionBarrier,
	vulkan_deviceCmdBufferQueueGrabBarrier,
	vulkan_deviceCmdBufferQueueReleaseBarrier,
	vulkan_deviceCmdTextureTransitionBarrier,
	vulkan_deviceCmdTextureQueueGrabBarrier,
	vulkan_deviceCmdTextureQueueReleaseBarrier,
};

/*
 */
Opal_Result vulkan_deviceInitialize(Vulkan_Device *device_ptr, Vulkan_Instance *instance_ptr, VkPhysicalDevice physical_device, VkDevice device)
{
	assert(device_ptr);
	assert(instance_ptr);
	assert(physical_device != VK_NULL_HANDLE);
	assert(device != VK_NULL_HANDLE);

	// opal vtable
	device_ptr->vtbl = &device_vtbl;

	// vulkan vtable
	volkLoadDeviceTable(&device_ptr->vk, device);

	// data
	device_ptr->instance = instance_ptr;
	device_ptr->physical_device = physical_device;
	device_ptr->device = device;

	// device properties
	memset(&device_ptr->raytrace_properties, 0, sizeof(VkPhysicalDeviceRayTracingPipelinePropertiesKHR));
	device_ptr->raytrace_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

	memset(&device_ptr->descriptor_buffer_properties, 0, sizeof(VkPhysicalDeviceDescriptorBufferPropertiesEXT));
	device_ptr->descriptor_buffer_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;

	VkPhysicalDeviceProperties2 properties = {0};
	properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties.pNext = &device_ptr->raytrace_properties;

	device_ptr->raytrace_properties.pNext = &device_ptr->descriptor_buffer_properties;

	vkGetPhysicalDeviceProperties2(physical_device, &properties);

	// max descriptor size
	size_t descriptor_sizes[] =
	{
		device_ptr->descriptor_buffer_properties.bufferCaptureReplayDescriptorDataSize,
		device_ptr->descriptor_buffer_properties.imageCaptureReplayDescriptorDataSize,
		device_ptr->descriptor_buffer_properties.imageViewCaptureReplayDescriptorDataSize,
		device_ptr->descriptor_buffer_properties.samplerCaptureReplayDescriptorDataSize,
		device_ptr->descriptor_buffer_properties.accelerationStructureCaptureReplayDescriptorDataSize,
		device_ptr->descriptor_buffer_properties.samplerDescriptorSize,
		device_ptr->descriptor_buffer_properties.combinedImageSamplerDescriptorSize,
		device_ptr->descriptor_buffer_properties.sampledImageDescriptorSize,
		device_ptr->descriptor_buffer_properties.storageImageDescriptorSize,
		device_ptr->descriptor_buffer_properties.uniformTexelBufferDescriptorSize,
		device_ptr->descriptor_buffer_properties.robustUniformTexelBufferDescriptorSize,
		device_ptr->descriptor_buffer_properties.storageTexelBufferDescriptorSize,
		device_ptr->descriptor_buffer_properties.robustStorageTexelBufferDescriptorSize,
		device_ptr->descriptor_buffer_properties.uniformBufferDescriptorSize,
		device_ptr->descriptor_buffer_properties.robustUniformBufferDescriptorSize,
		device_ptr->descriptor_buffer_properties.storageBufferDescriptorSize,
		device_ptr->descriptor_buffer_properties.robustStorageBufferDescriptorSize,
		device_ptr->descriptor_buffer_properties.inputAttachmentDescriptorSize,
		device_ptr->descriptor_buffer_properties.accelerationStructureDescriptorSize,
	};

	device_ptr->max_descriptor_size = descriptor_sizes[0];
	for (uint32_t i = 1; i < sizeof(descriptor_sizes) / sizeof(size_t); ++i)
		device_ptr->max_descriptor_size = max(device_ptr->max_descriptor_size, descriptor_sizes[i]);

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
		vulkan_functions.vkAllocateMemory = device_ptr->vk.vkAllocateMemory;
		vulkan_functions.vkFreeMemory = device_ptr->vk.vkFreeMemory;
		vulkan_functions.vkMapMemory = device_ptr->vk.vkMapMemory;
		vulkan_functions.vkUnmapMemory = device_ptr->vk.vkUnmapMemory;
		vulkan_functions.vkFlushMappedMemoryRanges = device_ptr->vk.vkFlushMappedMemoryRanges;
		vulkan_functions.vkInvalidateMappedMemoryRanges = device_ptr->vk.vkInvalidateMappedMemoryRanges;
		vulkan_functions.vkBindBufferMemory = device_ptr->vk.vkBindBufferMemory;
		vulkan_functions.vkBindImageMemory = device_ptr->vk.vkBindImageMemory;
		vulkan_functions.vkGetBufferMemoryRequirements = device_ptr->vk.vkGetBufferMemoryRequirements;
		vulkan_functions.vkGetImageMemoryRequirements = device_ptr->vk.vkGetImageMemoryRequirements;
		vulkan_functions.vkCreateBuffer = device_ptr->vk.vkCreateBuffer;
		vulkan_functions.vkDestroyBuffer = device_ptr->vk.vkDestroyBuffer;
		vulkan_functions.vkCreateImage = device_ptr->vk.vkCreateImage;
		vulkan_functions.vkDestroyImage = device_ptr->vk.vkDestroyImage;
		vulkan_functions.vkCmdCopyBuffer = device_ptr->vk.vkCmdCopyBuffer;
		vulkan_functions.vkGetBufferMemoryRequirements2KHR = device_ptr->vk.vkGetBufferMemoryRequirements2;
		vulkan_functions.vkGetImageMemoryRequirements2KHR = device_ptr->vk.vkGetImageMemoryRequirements2;
		vulkan_functions.vkBindBufferMemory2KHR = device_ptr->vk.vkBindBufferMemory2;
		vulkan_functions.vkBindImageMemory2KHR = device_ptr->vk.vkBindImageMemory2;
		vulkan_functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
		vulkan_functions.vkGetDeviceBufferMemoryRequirements = device_ptr->vk.vkGetDeviceBufferMemoryRequirements;
		vulkan_functions.vkGetDeviceImageMemoryRequirements = device_ptr->vk.vkGetDeviceImageMemoryRequirements;

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
		uint32_t buffer_image_granularity = (uint32_t)properties.properties.limits.bufferImageGranularity;

		Opal_Result result = vulkan_allocatorInitialize(device_ptr, instance_ptr->heap_size, instance_ptr->max_heap_allocations, instance_ptr->max_heaps, buffer_image_granularity);
		assert(result == OPAL_SUCCESS);

		OPAL_UNUSED(result);
	}

	// bump
	opal_bumpInitialize(&device_ptr->bump, 256);

	// pools
	opal_poolInitialize(&device_ptr->queues, sizeof(Vulkan_Queue), 32);
	opal_poolInitialize(&device_ptr->semaphores, sizeof(Vulkan_Semaphore), 32);
	opal_poolInitialize(&device_ptr->buffers, sizeof(Vulkan_Buffer), 32);
	opal_poolInitialize(&device_ptr->images, sizeof(Vulkan_Image), 32);
	opal_poolInitialize(&device_ptr->image_views, sizeof(Vulkan_ImageView), 32);
	opal_poolInitialize(&device_ptr->samplers, sizeof(Vulkan_Sampler), 32);
	opal_poolInitialize(&device_ptr->acceleration_structures, sizeof(Vulkan_AccelerationStructure), 32);
	opal_poolInitialize(&device_ptr->command_allocators, sizeof(Vulkan_CommandAllocator), 32);
	opal_poolInitialize(&device_ptr->command_buffers, sizeof(Vulkan_CommandBuffer), 32);
	opal_poolInitialize(&device_ptr->shaders, sizeof(Vulkan_Shader), 32);
	opal_poolInitialize(&device_ptr->descriptor_heaps, sizeof(Vulkan_DescriptorHeap), 32);
	opal_poolInitialize(&device_ptr->descriptor_set_layouts, sizeof(Vulkan_DescriptorSetLayout), 32);
	opal_poolInitialize(&device_ptr->descriptor_sets, sizeof(Vulkan_DescriptorSet), 32);
	opal_poolInitialize(&device_ptr->pipeline_layouts, sizeof(Vulkan_PipelineLayout), 32);
	opal_poolInitialize(&device_ptr->pipelines, sizeof(Vulkan_Pipeline), 32);
	opal_poolInitialize(&device_ptr->swapchains, sizeof(Vulkan_Swapchain), 32);

	// queues
	const Vulkan_DeviceEnginesInfo *engines_info = &device_ptr->device_engines_info;

	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
	{
		uint32_t queue_count = engines_info->queue_counts[i];
		uint32_t queue_family = engines_info->queue_families[i];

		Vulkan_Queue queue = {0};
		queue.family_index = queue_family;

		Opal_Queue *queue_handles = (Opal_Queue *)malloc(sizeof(Opal_Queue) * queue_count);

		for (uint32_t j = 0; j < queue_count; j++)
		{
			device_ptr->vk.vkGetDeviceQueue(device, queue_family, j, &queue.queue);
			queue_handles[j] = (Opal_Queue)opal_poolAddElement(&device_ptr->queues, &queue);
		}

		device_ptr->queue_handles[i] = queue_handles;
	}

	return OPAL_SUCCESS;
}

Opal_Result vulkan_deviceAllocateMemory(Vulkan_Device *device_ptr, const Vulkan_AllocationDesc *desc, Vulkan_Allocation *allocation)
{
	assert(device_ptr);

	Vulkan_Allocator *allocator = &device_ptr->allocator;
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
			opal_result = vulkan_allocatorAllocateMemory(device_ptr, desc, memory_type, 0, allocation);

		if (opal_result == OPAL_NO_MEMORY)
			opal_result = vulkan_allocatorAllocateMemory(device_ptr, desc, memory_type, 1, allocation);

		if (opal_result == OPAL_SUCCESS)
			return OPAL_SUCCESS;

		memory_type_bits &= ~(1 << memory_type);
	}

	return OPAL_NO_MEMORY;
}
