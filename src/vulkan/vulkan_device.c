#include "vulkan_internal.h"

#include <assert.h>
#include <stdlib.h>

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

	Opal_PoolHandle *queue_handles = ptr->queue_handles[engine_type];
	assert(queue_handles);

	*queue = queue_handles[index];
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceDestroy(Opal_Device this)
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
		Opal_Result result = vulkan_allocatorShutdown(ptr);
		assert(result == OPAL_SUCCESS);
	}

	// TODO: proper cleanup for all previously created buffers & images
	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
	{
		vkDestroyCommandPool(ptr->device, ptr->command_pools[i], NULL);
		free(ptr->queue_handles[i]);
	}

	opal_poolShutdown(&ptr->swap_chains);
	opal_poolShutdown(&ptr->pipelines);
	opal_poolShutdown(&ptr->pipeline_layouts);
	opal_poolShutdown(&ptr->bindsets);
	opal_poolShutdown(&ptr->bindset_pools);
	opal_poolShutdown(&ptr->bindset_layouts);
	opal_poolShutdown(&ptr->shaders);
	opal_poolShutdown(&ptr->command_buffers);
	opal_poolShutdown(&ptr->samplers);
	opal_poolShutdown(&ptr->image_views);
	opal_poolShutdown(&ptr->images);
	opal_poolShutdown(&ptr->buffers);
	opal_poolShutdown(&ptr->queues);

	vkDestroyDevice(ptr->device, NULL);

	free(ptr);
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

static Opal_Result vulkan_deviceCreateTexture(Opal_Device this, const Opal_TextureDesc *desc, Opal_Texture *texture)
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
	image_info.format = vulkan_helperToFormat(desc->format);
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
		Vulkan_Allocator *allocator = &device_ptr->allocator;

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

	Vulkan_Image *image_ptr = (Vulkan_Image *)opal_poolGetElement(&device_ptr->images, desc->texture);
	assert(image_ptr);

	VkImageView vulkan_image_view = VK_NULL_HANDLE;

	VkImageViewCreateInfo image_view_info = {0};
	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.image = image_ptr->image;
	image_view_info.viewType = vulkan_helperToImageViewType(desc->type);
	image_view_info.format = vulkan_helperToFormat(image_ptr->format);
	image_view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	image_view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	image_view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	image_view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	image_view_info.subresourceRange.aspectMask = vulkan_helperToAspectMask(image_ptr->format);
	image_view_info.subresourceRange.baseArrayLayer = desc->base_layer;
	image_view_info.subresourceRange.layerCount = desc->layer_count;
	image_view_info.subresourceRange.baseMipLevel = desc->base_mip;
	image_view_info.subresourceRange.levelCount = desc->mip_count;

	VkResult vulkan_result = device_ptr->vk.vkCreateImageView(vulkan_device, &image_view_info, NULL, &vulkan_image_view);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_ImageView result = {0};
	result.image_view = vulkan_image_view;

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
	sampler_info.compareEnable = desc->compare_op != OPAL_COMPARE_OP_NEVER;
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

static Opal_Result vulkan_deviceCreateCommandBuffer(Opal_Device this, Opal_DeviceEngineType engine_type, Opal_CommandBuffer *command_buffer)
{
	assert(this);
	assert(command_buffer);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	VkCommandBuffer vulkan_command_buffer = VK_NULL_HANDLE;

	VkCommandBufferAllocateInfo command_buffer_info = {0};
	command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_info.commandPool = device_ptr->command_pools[engine_type];
	command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_info.commandBufferCount = 1;

	VkResult vulkan_result = device_ptr->vk.vkAllocateCommandBuffers(vulkan_device, &command_buffer_info, &vulkan_command_buffer);
	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_CommandBuffer result = {0};
	result.command_buffer = vulkan_command_buffer;

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

static Opal_Result vulkan_deviceCreateBindsetLayout(Opal_Device this, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout)
{
	assert(this);
	assert(num_bindings > 0);
	assert(bindings);
	assert(bindset_layout);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	VkDescriptorSetLayout vulkan_set_layout = VK_NULL_HANDLE;

	VkDescriptorSetLayoutCreateInfo set_layout_info = {0};
	set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout_info.bindingCount = num_bindings;

	VkDescriptorSetLayoutBinding *vulkan_bindings = (VkDescriptorSetLayoutBinding *)malloc(sizeof(VkDescriptorSetLayoutBinding) * num_bindings);
	for (uint32_t i = 0; i < num_bindings; ++i)
	{
		vulkan_bindings[i].binding = bindings[i].binding;
		vulkan_bindings[i].descriptorCount = 1;
		vulkan_bindings[i].descriptorType = vulkan_helperToDescriptorType(bindings[i].type);
		vulkan_bindings[i].pImmutableSamplers = NULL;
		vulkan_bindings[i].stageFlags = vulkan_helperToShaderStageFlags(bindings[i].visibility);
	}

	set_layout_info.pBindings = vulkan_bindings;

	VkResult vulkan_result = device_ptr->vk.vkCreateDescriptorSetLayout(vulkan_device, &set_layout_info, NULL, &vulkan_set_layout);

	if (vulkan_result != VK_SUCCESS)
	{
		free(vulkan_bindings);
		return OPAL_VULKAN_ERROR;
	}

	Vulkan_BindsetLayout result = {0};
	result.layout = vulkan_set_layout;
	result.layout_bindings = vulkan_bindings;
	result.num_layout_bindings = num_bindings;

	*bindset_layout = (Opal_BindsetLayout)opal_poolAddElement(&device_ptr->bindset_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateBindsetPool(Opal_Device this, Opal_BindsetLayout bindset_layout, uint32_t max_bindsets, Opal_BindsetPool *bindset_pool)
{
	assert(this);
	assert(bindset_layout);
	assert(max_bindsets > 0);
	assert(bindset_pool);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	Vulkan_BindsetLayout *bindset_layout_ptr = (Vulkan_BindsetLayout *)opal_poolGetElement(&device_ptr->bindset_layouts, bindset_layout);
	assert(bindset_layout_ptr);
	assert(bindset_layout_ptr->layout_bindings);

	const VkDescriptorSetLayoutBinding *vulkan_layout_bindings = bindset_layout_ptr->layout_bindings;
	uint32_t num_layout_bindings = bindset_layout_ptr->num_layout_bindings;

	VkDescriptorPool vulkan_pool = VK_NULL_HANDLE;

	VkDescriptorPoolCreateInfo pool_info = {0};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.maxSets = max_bindsets;

	VkDescriptorPoolSize *vulkan_pool_sizes = (VkDescriptorPoolSize *)malloc(sizeof(VkDescriptorPoolSize) * num_layout_bindings);
	for (uint32_t i = 0; i < num_layout_bindings; ++i)
	{
		vulkan_pool_sizes[i].descriptorCount = vulkan_layout_bindings[i].descriptorCount;
		vulkan_pool_sizes[i].type = vulkan_layout_bindings[i].descriptorType;
	}

	pool_info.poolSizeCount = num_layout_bindings;
	pool_info.pPoolSizes = vulkan_pool_sizes;

	VkResult vulkan_result = device_ptr->vk.vkCreateDescriptorPool(vulkan_device, &pool_info, NULL, &vulkan_pool);
	free(vulkan_pool_sizes);

	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_BindsetPool result = {0};
	result.pool = vulkan_pool;

	*bindset_pool = (Opal_BindsetPool)opal_poolAddElement(&device_ptr->bindset_pools, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreatePipelineLayout(Opal_Device this, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout)
{
	assert(this);
	assert(pipeline_layout);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	VkDescriptorSetLayout *set_layouts = NULL;

	if (num_bindset_layouts > 0)
	{
		assert(bindset_layouts);
		set_layouts = (VkDescriptorSetLayout *)malloc(sizeof(VkDescriptorSetLayout) * num_bindset_layouts);

		for (uint32_t i = 0; i < num_bindset_layouts; ++i)
		{
			Vulkan_BindsetLayout *bindset_layout_ptr = (Vulkan_BindsetLayout *)opal_poolGetElement(&device_ptr->bindset_layouts, bindset_layouts[i]);
			assert(bindset_layout_ptr);
			assert(bindset_layout_ptr->layout_bindings);

			set_layouts[i] = bindset_layout_ptr->layout;
		}
	}

	VkPipelineLayout vulkan_pipeline_layout = VK_NULL_HANDLE;

	VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = num_bindset_layouts;
	pipeline_layout_info.pSetLayouts = set_layouts;

	VkResult vulkan_result = device_ptr->vk.vkCreatePipelineLayout(vulkan_device, &pipeline_layout_info, NULL, &vulkan_pipeline_layout);
	free(set_layouts);

	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_PipelineLayout result = {0};
	result.layout = vulkan_pipeline_layout;

	*pipeline_layout = (Opal_PipelineLayout)opal_poolAddElement(&device_ptr->pipeline_layouts, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateGraphicsPipeline(Opal_Device this, const Opal_GraphicsPipelineDesc *desc, Opal_GraphicsPipeline *pipeline)
{
	assert(this);
	assert(desc);
	assert(pipeline);
	assert(desc->vertex_shader);
	assert(desc->fragment_shader);

	Vulkan_Device *device_ptr = (Vulkan_Device *)this;
	VkDevice vulkan_device = device_ptr->device;

	// pipeline layout
	VkPipelineLayout vulkan_pipeline_layout = VK_NULL_HANDLE;
	VkPipeline vulkan_pipeline = VK_NULL_HANDLE;
	VkPipelineCache vulkan_pipeline_cache = VK_NULL_HANDLE;

	// shaders
	Opal_Shader shaders[5] =
	{
		desc->vertex_shader,
		desc->tessellation_control_shader,
		desc->tessellation_evaluation_shader,
		desc->geometry_shader,
		desc->fragment_shader
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
		Vulkan_Shader *shader_ptr = opal_poolGetElement(&device_ptr->shaders, shaders[i]);
		if (shader_ptr == NULL)
			continue;

		shader_infos[num_shaders].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_infos[num_shaders].pNext = NULL;
		shader_infos[num_shaders].flags = 0;
		shader_infos[num_shaders].stage = shader_stages[i];
		shader_infos[num_shaders].module = shader_ptr->shader;
		shader_infos[num_shaders].pName = "main";
		shader_infos[num_shaders].pSpecializationInfo = NULL;
		num_shaders++;
	}

	// vertex input state
	VkPipelineVertexInputStateCreateInfo vertex_input = {0};
	vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	uint32_t num_vertex_attributes = 0;
	for (uint32_t i = 0; i < desc->num_vertex_streams; ++i)
		num_vertex_attributes += desc->vertex_streams[i].num_vertex_attributes;

	VkVertexInputBindingDescription *vertex_streams = (VkVertexInputBindingDescription *)malloc(sizeof(VkVertexInputBindingDescription) * desc->num_vertex_streams);
	VkVertexInputAttributeDescription *vertex_attributes = (VkVertexInputAttributeDescription *)malloc(sizeof(VkVertexInputAttributeDescription) * num_vertex_attributes);

	uint32_t num_attributes = 0;
	for (uint32_t i = 0; i < desc->num_vertex_streams; ++i)
	{
		const Opal_VertexStream *vertex_stream = &desc->vertex_streams[i];
		for (uint32_t j = 0; j < vertex_stream->num_vertex_attributes; ++j)
		{
			const Opal_VertexAttribute *vertex_attribute = &vertex_stream->attributes[j];

			vertex_attributes[num_attributes].location = j;
			vertex_attributes[num_attributes].binding = i;
			vertex_attributes[num_attributes].format = vulkan_helperToFormat(vertex_attribute->format);
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

	// multisample state
	VkPipelineMultisampleStateCreateInfo multisample = {0};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.rasterizationSamples = vulkan_helperToSamples(desc->rasterization_samples);

	// depstencil state
	VkPipelineDepthStencilStateCreateInfo depthstencil = {0};
	depthstencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthstencil.depthTestEnable = desc->depth_enable;
	depthstencil.depthWriteEnable = desc->depth_write;
	depthstencil.depthCompareOp = vulkan_helperToCompareOp(desc->depth_compare_op);
	depthstencil.stencilTestEnable = desc->stencil_enable;
	depthstencil.front.failOp = vulkan_helperToStencilOp(desc->stencil_front.fail_op);
	depthstencil.front.depthFailOp = vulkan_helperToStencilOp(desc->stencil_front.depth_fail_op);
	depthstencil.front.passOp = vulkan_helperToStencilOp(desc->stencil_front.pass_op);
	depthstencil.front.compareOp = vulkan_helperToCompareOp(desc->stencil_front.compare_op);
	depthstencil.front.compareMask = desc->stencil_read_mask;
	depthstencil.front.writeMask = desc->stencil_write_mask;
	depthstencil.back.failOp = vulkan_helperToStencilOp(desc->stencil_back.fail_op);
	depthstencil.back.depthFailOp = vulkan_helperToStencilOp(desc->stencil_back.depth_fail_op);
	depthstencil.back.passOp = vulkan_helperToStencilOp(desc->stencil_back.pass_op);
	depthstencil.back.compareOp = vulkan_helperToCompareOp(desc->stencil_back.compare_op);
	depthstencil.back.compareMask = desc->stencil_read_mask;
	depthstencil.back.writeMask = desc->stencil_write_mask;

	// colorblend state
	VkPipelineColorBlendStateCreateInfo blend = {0};
	blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend.attachmentCount = desc->num_color_attachments;

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
		color_attachment_formats[i] = vulkan_helperToFormat(desc->color_attachment_formats[i]);

	VkPipelineRenderingCreateInfoKHR dynamic_rendering_info = {0};
	dynamic_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	dynamic_rendering_info.colorAttachmentCount = desc->num_color_attachments;
	dynamic_rendering_info.pColorAttachmentFormats = color_attachment_formats;

	if (desc->depthstencil_attachment_format)
	{
		dynamic_rendering_info.depthAttachmentFormat = vulkan_helperToFormat(*desc->depthstencil_attachment_format);
		dynamic_rendering_info.stencilAttachmentFormat = vulkan_helperToFormat(*desc->depthstencil_attachment_format);
	}

	// pipeline
	VkGraphicsPipelineCreateInfo pipeline_info = {0};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = num_shaders;
	pipeline_info.pStages = shader_infos;
	pipeline_info.pVertexInputState = &vertex_input;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pTessellationState = &tessellation;
	pipeline_info.pRasterizationState = &rasterization;
	pipeline_info.pMultisampleState = &multisample;
	pipeline_info.pDepthStencilState = &depthstencil;
	pipeline_info.pColorBlendState = &blend;
	pipeline_info.pDynamicState = &dynamic;
	pipeline_info.layout = vulkan_pipeline_layout;
	pipeline_info.pNext = &dynamic_rendering_info;

	VkResult vulkan_result = device_ptr->vk.vkCreateGraphicsPipelines(vulkan_device, vulkan_pipeline_cache, 1, &pipeline_info, NULL, &vulkan_pipeline);

	// error handling & result
	free(vertex_streams);
	free(vertex_attributes);

	if (vulkan_result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	Vulkan_Pipeline result = {0};
	result.pipeline = vulkan_pipeline;

	*pipeline = (Opal_GraphicsPipeline)opal_poolAddElement(&device_ptr->pipelines, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_deviceCreateMeshletPipeline(Opal_Device this, const Opal_MeshletPipelineDesc *desc, Opal_MeshletPipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCreateComputePipeline(Opal_Device this, const Opal_ComputePipelineDesc *desc, Opal_ComputePipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCreateRaytracePipeline(Opal_Device this, const Opal_RaytracePipelineDesc *desc, Opal_RaytracePipeline *pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCreateSwapChain(Opal_Device this, const void *handle, Opal_SwapChain *swap_chain)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroyBuffer(Opal_Device this, Opal_Buffer buffer)
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

	device_ptr->vk.vkDestroyBuffer(device_ptr->device, buffer_ptr->buffer, NULL);
	Opal_Result result = vulkan_allocatorFreeMemory(device_ptr, buffer_ptr->allocation);

	if (buffer_ptr->map_count > 0)
		vulkan_allocatorUnmapMemory(device_ptr, buffer_ptr->allocation);

	return result;
}

static Opal_Result vulkan_deviceDestroyTexture(Opal_Device this, Opal_Texture texture)
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

	device_ptr->vk.vkDestroyImage(device_ptr->device, image_ptr->image, NULL);
	Opal_Result result = vulkan_allocatorFreeMemory(device_ptr, image_ptr->allocation);
	return result;
}

static Opal_Result vulkan_deviceDestroyTextureView(Opal_Device this, Opal_TextureView texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroySampler(Opal_Device this, Opal_Sampler sampler)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroyCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroyShader(Opal_Device this, Opal_Shader shader)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroyBindsetLayout(Opal_Device this, Opal_BindsetLayout bindset_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroyBindsetPool(Opal_Device this, Opal_BindsetPool bindset_pool)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroyPipelineLayout(Opal_Device this, Opal_PipelineLayout pipeline_layout)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroyGraphicsPipeline(Opal_Device this, Opal_GraphicsPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroyMeshletPipeline(Opal_Device this, Opal_MeshletPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroyComputePipeline(Opal_Device this, Opal_ComputePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroyRaytracePipeline(Opal_Device this, Opal_RaytracePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceDestroySwapChain(Opal_Device this, Opal_SwapChain swap_chain)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceAllocateBindset(Opal_Device this, Opal_BindsetPool bindset_pool, uint32_t num_bindings, const Opal_BindsetBinding *bindings, Opal_Bindset *bindset)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceFreeBindset(Opal_Device this, Opal_BindsetPool bindset_pool, Opal_Bindset bindset)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceMapBuffer(Opal_Device this, Opal_Buffer buffer, void **ptr)
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
	return vulkan_allocatorUnmapMemory(device_ptr, buffer_ptr->allocation);
}

static Opal_Result vulkan_deviceUpdateBindset(Opal_Device this, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceBeginCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceEndCommandBuffer(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceWaitCommandBuffers(Opal_Device this, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceWaitIdle(Opal_Device this)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceSubmit(Opal_Device this, Opal_Queue queue, uint32_t num_command_buffer, const Opal_CommandBuffer *command_buffers, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceAcquire(Opal_Device this, Opal_SwapChain swap_chain, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_devicePresent(Opal_Device this, Opal_SwapChain swap_chain, uint32_t num_wait_command_buffers, const Opal_CommandBuffer *wait_command_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdBeginGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_attachments, const Opal_FramebufferAttachment *attachments)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdEndGraphicsPass(Opal_Device this, Opal_CommandBuffer command_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdSetGraphicsPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_GraphicsPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdSetMeshletPipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_MeshletPipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdSetComputePipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_ComputePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdSetRaytracePipeline(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_RaytracePipeline pipeline)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdSetBindsets(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_bindsets, const Opal_Bindset *bindsets)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdSetVertexBuffers(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdSetIndexBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdDrawIndexedInstanced(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t base_index, uint32_t num_instances, uint32_t base_instance)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdMeshletDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdComputeDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdRaytraceDispatch(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTableView shader_table, uint32_t width, uint32_t height, uint32_t depth)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdCopyBufferToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdCopyBufferToTexture(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_TextureRegion dst)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdCopyTextureToBuffer(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferView dst)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdBufferTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdBufferQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdBufferQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdTextureTransitionBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdTextureQueueGrabBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result vulkan_deviceCmdTextureQueueReleaseBarrier(Opal_Device this, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
static Opal_DeviceTable device_vtbl =
{
	vulkan_deviceDestroy,
	vulkan_deviceGetInfo,
	vulkan_deviceGetQueue,

	vulkan_deviceCreateBuffer,
	vulkan_deviceCreateTexture,
	vulkan_deviceCreateTextureView,
	vulkan_deviceCreateSampler,
	vulkan_deviceCreateCommandBuffer,
	vulkan_deviceCreateShader,
	vulkan_deviceCreateBindsetLayout,
	vulkan_deviceCreateBindsetPool,
	vulkan_deviceCreatePipelineLayout,
	vulkan_deviceCreateGraphicsPipeline,
	vulkan_deviceCreateMeshletPipeline,
	vulkan_deviceCreateComputePipeline,
	vulkan_deviceCreateRaytracePipeline,
	vulkan_deviceCreateSwapChain,

	vulkan_deviceDestroyBuffer,
	vulkan_deviceDestroyTexture,
	vulkan_deviceDestroyTextureView,
	vulkan_deviceDestroySampler,
	vulkan_deviceDestroyCommandBuffer,
	vulkan_deviceDestroyShader,
	vulkan_deviceDestroyBindsetLayout,
	vulkan_deviceDestroyBindsetPool,
	vulkan_deviceDestroyPipelineLayout,
	vulkan_deviceDestroyGraphicsPipeline,
	vulkan_deviceDestroyMeshletPipeline,
	vulkan_deviceDestroyComputePipeline,
	vulkan_deviceDestroyRaytracePipeline,
	vulkan_deviceDestroySwapChain,

	vulkan_deviceAllocateBindset,
	vulkan_deviceFreeBindset,
	vulkan_deviceMapBuffer,
	vulkan_deviceUnmapBuffer,
	vulkan_deviceUpdateBindset,
	vulkan_deviceBeginCommandBuffer,
	vulkan_deviceEndCommandBuffer,
	vulkan_deviceWaitCommandBuffers,
	vulkan_deviceWaitIdle,
	vulkan_deviceSubmit,
	vulkan_deviceAcquire,
	vulkan_devicePresent,

	vulkan_deviceCmdBeginGraphicsPass,
	vulkan_deviceCmdEndGraphicsPass,
	vulkan_deviceCmdSetGraphicsPipeline,
	vulkan_deviceCmdSetMeshletPipeline,
	vulkan_deviceCmdSetComputePipeline,
	vulkan_deviceCmdSetRaytracePipeline,
	vulkan_deviceCmdSetBindsets,
	vulkan_deviceCmdSetVertexBuffers,
	vulkan_deviceCmdSetIndexBuffer,
	vulkan_deviceCmdDrawIndexedInstanced,
	vulkan_deviceCmdMeshletDispatch,
	vulkan_deviceCmdComputeDispatch,
	vulkan_deviceCmdRaytraceDispatch,
	vulkan_deviceCmdCopyBufferToBuffer,
	vulkan_deviceCmdCopyBufferToTexture,
	vulkan_deviceCmdCopyTextureToBuffer,
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
		VkPhysicalDeviceProperties properties = {0};
		vkGetPhysicalDeviceProperties(physical_device, &properties);

		uint32_t buffer_image_granularity = (uint32_t)properties.limits.bufferImageGranularity;

		Opal_Result result = vulkan_allocatorInitialize(device_ptr, instance_ptr->heap_size, instance_ptr->max_heap_allocations, instance_ptr->max_heaps, buffer_image_granularity);
		assert(result == OPAL_SUCCESS);
	}

	// pools
	opal_poolInitialize(&device_ptr->queues, sizeof(Vulkan_Queue), 32);
	opal_poolInitialize(&device_ptr->buffers, sizeof(Vulkan_Buffer), 32);
	opal_poolInitialize(&device_ptr->images, sizeof(Vulkan_Image), 32);
	opal_poolInitialize(&device_ptr->image_views, sizeof(Vulkan_ImageView), 32);
	opal_poolInitialize(&device_ptr->samplers, sizeof(Vulkan_Sampler), 32);
	opal_poolInitialize(&device_ptr->command_buffers, sizeof(Vulkan_CommandBuffer), 32);
	opal_poolInitialize(&device_ptr->shaders, sizeof(Vulkan_Shader), 32);
	opal_poolInitialize(&device_ptr->bindset_layouts, sizeof(Vulkan_BindsetLayout), 32);
	opal_poolInitialize(&device_ptr->bindset_pools, sizeof(Vulkan_BindsetPool), 32);
	opal_poolInitialize(&device_ptr->bindsets, sizeof(Vulkan_Bindset), 32);
	opal_poolInitialize(&device_ptr->pipeline_layouts, sizeof(Vulkan_PipelineLayout), 32);
	opal_poolInitialize(&device_ptr->pipelines, sizeof(Vulkan_Pipeline), 32);
	opal_poolInitialize(&device_ptr->swap_chains, sizeof(Vulkan_SwapChain), 32);

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
			device_ptr->vk.vkGetDeviceQueue(device, queue_family, j, &queue.queue);
			queue_handles[j] = opal_poolAddElement(&device_ptr->queues, &queue);
		}

		device_ptr->queue_handles[i] = queue_handles;
	}

	// command pools
	for (uint32_t i = 0; i < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX; ++i)
	{
		VkCommandPoolCreateInfo command_pool_info = {0};
		command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_info.queueFamilyIndex = engines_info->queue_families[i];

		VkResult vulkan_result = device_ptr->vk.vkCreateCommandPool(device, &command_pool_info, NULL, &device_ptr->command_pools[i]);

		if (vulkan_result != VK_SUCCESS)
			return OPAL_VULKAN_ERROR;
	}

	return OPAL_SUCCESS;
}

Opal_Result vulkan_deviceAllocateMemory(Vulkan_Device *device_ptr, const Vulkan_AllocationDesc *desc, Vulkan_Allocation *allocation)
{
	assert(device_ptr);

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
			opal_result = vulkan_allocatorAllocateMemory(device_ptr, desc, memory_type, 0, allocation);

		if (opal_result == OPAL_NO_MEMORY)
			opal_result = vulkan_allocatorAllocateMemory(device_ptr, desc, memory_type, 1, allocation);

		if (opal_result == OPAL_SUCCESS)
			return OPAL_SUCCESS;

		memory_type_bits &= ~(1 << memory_type);
	}

	return OPAL_NO_MEMORY;
}
