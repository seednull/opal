#pragma once

#include "opal_internal.h"

#include <volk.h>

#ifdef OPAL_HAS_VMA
#include "vk_mem_alloc.h"
#endif

#include "common/bump.h"
#include "common/heap.h"
#include "common/pool.h"

#define OPAL_VULKAN_HEAP_NULL 0xFFFFFFFF

typedef struct VolkDeviceTable VolkDeviceTable;

typedef enum Vulkan_ResourceType_t
{
	VULKAN_RESOURCE_TYPE_LINEAR = 1,
	VULKAN_RESOURCE_TYPE_NONLINEAR = 2,
} Vulkan_ResourceType;

typedef struct Vulkan_MemoryBlock_t
{
	VkDeviceMemory memory;
	VkDeviceSize size;
	void *mapped_ptr;
	uint32_t memory_type;
	uint32_t map_count;
	uint32_t heap;
} Vulkan_MemoryBlock;

typedef struct Vulkan_MemoryHeap_t
{
	Opal_Heap heap;
	Opal_PoolHandle block;
	uint16_t *granularity_pages;
	uint32_t next_heap;
} Vulkan_MemoryHeap;

typedef struct Vulkan_Allocator_t
{
	Vulkan_MemoryHeap *heaps;
	uint32_t num_heaps;

	Opal_Pool blocks;

	uint32_t first_heap[VK_MAX_MEMORY_TYPES];
	uint32_t last_used_heaps[VK_MAX_MEMORY_TYPES];

	uint32_t heap_size;
	uint32_t max_heaps;
	uint32_t max_heap_allocations;
	uint32_t buffer_image_granularity;
} Vulkan_Allocator;

typedef struct Vulkan_AllocationDesc_t
{
	VkDeviceSize size;
	VkDeviceSize alignment;
	uint32_t memory_type_bits;
	uint32_t required_flags;
	uint32_t preferred_flags;
	uint32_t not_preferred_flags;
	uint32_t resource_type;
	Opal_AllocationHint hint;
	VkBool32 prefers_dedicated;
	VkBool32 requires_dedicated;
} Vulkan_AllocationDesc;

typedef struct Vulkan_Allocation_t
{
	VkDeviceMemory memory;
	uint32_t offset;
	Opal_PoolHandle block;
	Opal_NodeIndex heap_metadata;
} Vulkan_Allocation;

typedef struct Vulkan_DeviceEnginesInfo_t
{
	uint32_t queue_families[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX];
	uint32_t queue_counts[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX];
} Vulkan_DeviceEnginesInfo;

typedef struct Vulkan_Instance_t
{
	Opal_InstanceTable *vtbl;
	// TODO: add head for Vulkan_Device intrusive list
	uint32_t heap_size;
	uint32_t max_heap_allocations;
	uint32_t max_heaps;
	uint32_t flags;
	VkInstance instance;
	Opal_Pool surfaces;
} Vulkan_Instance;

typedef struct Vulkan_Device_t
{
	Opal_DeviceTable *vtbl;
	Vulkan_Instance *instance;
	// TODO: add Vulkan_Device intrusive list head here, so Vulkan_Instance will be able to delete all of them
	VolkDeviceTable vk;
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytrace_properties;
	Vulkan_DeviceEnginesInfo device_engines_info;
	Opal_Queue *queue_handles[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX];
	Opal_Bump bump;
	Opal_Pool queues;
	Opal_Pool semaphores;
	Opal_Pool buffers;
	Opal_Pool images;
	Opal_Pool image_views;
	Opal_Pool samplers;
	Opal_Pool acceleration_structures;
	Opal_Pool command_pools;
	Opal_Pool command_buffers;
	Opal_Pool shaders;
	Opal_Pool bindset_layouts;
	Opal_Pool bindset_pools;
	Opal_Pool bindsets;
	Opal_Pool pipeline_layouts;
	Opal_Pool pipelines;
	Opal_Pool swapchains;

#ifdef OPAL_HAS_VMA
	uint32_t use_vma;
	VmaAllocator vma_allocator;
#endif
	Vulkan_Allocator allocator;
} Vulkan_Device;

typedef struct Vulkan_Semaphore_t
{
	VkSemaphore semaphore;
} Vulkan_Semaphore;

typedef struct Vulkan_Queue_t
{
	VkQueue queue;
	uint32_t family_index;
} Vulkan_Queue;

typedef struct Vulkan_Buffer_t
{
	VkBuffer buffer;
	uint32_t map_count;
#ifdef OPAL_HAS_VMA
	VmaAllocation vma_allocation;
#endif
	Vulkan_Allocation allocation;
	VkDeviceAddress device_address;
} Vulkan_Buffer;

typedef struct Vulkan_Image_t
{
	VkImage image;
	Opal_Format format;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
#ifdef OPAL_HAS_VMA
	VmaAllocation vma_allocation;
#endif
	Vulkan_Allocation allocation;
} Vulkan_Image;

typedef struct Vulkan_ImageView_t
{
	VkImageView image_view;
	VkImage image;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t base_mip;
	uint32_t num_mips;
	uint32_t base_layer;
	uint32_t num_layers;
	VkImageAspectFlagBits aspect_mask;
} Vulkan_ImageView;

typedef struct Vulkan_Sampler_t
{
	VkSampler sampler;
} Vulkan_Sampler;

typedef struct Vulkan_AccelerationStructure_t
{
	VkAccelerationStructureKHR acceleration_structure;
	VkQueryPool size_pool;
	VkQueryPool serialization_size_pool;
	VkQueryPool compacted_size_pool;
} Vulkan_AccelerationStructure;

typedef struct Vulkan_CommandPool_t
{
	VkCommandPool pool;
	Opal_Queue queue;
} Vulkan_CommandPool;

typedef struct Vulkan_CommandBuffer_t
{
	VkCommandBuffer command_buffer;
	VkPipelineBindPoint pipeline_bind_point;
} Vulkan_CommandBuffer;

typedef struct Vulkan_Shader_t
{
	VkShaderModule shader;
} Vulkan_Shader;

typedef struct Vulkan_BindsetLayout_t
{
	VkDescriptorSetLayout layout;
	VkDescriptorSetLayoutBinding *layout_bindings;
	uint32_t num_layout_bindings;
} Vulkan_BindsetLayout;

typedef struct Vulkan_BindsetPool_t
{
	VkDescriptorPool pool;
} Vulkan_BindsetPool;

typedef struct Vulkan_Bindset_t
{
	VkDescriptorSet set;
	Opal_BindsetLayout bindset_layout;
} Vulkan_Bindset;

typedef struct Vulkan_PipelineLayout_t
{
	VkPipelineLayout layout;
} Vulkan_PipelineLayout;

typedef struct Vulkan_Pipeline_t
{
	VkPipeline pipeline;
	VkPipelineBindPoint bind_point;
} Vulkan_Pipeline;

typedef struct Vulkan_Surface_t
{
	VkSurfaceKHR surface;
} Vulkan_Surface;

typedef struct Vulkan_Swapchain_t
{
	VkSwapchainKHR swapchain;
	Opal_Queue present_queue;
	Opal_TextureView *texture_views;
	VkSemaphore *acquire_semaphores;
	VkSemaphore *present_semaphores;
	uint32_t num_images;
	uint32_t current_image;
	uint32_t current_semaphore;
} Vulkan_Swapchain;

Opal_Result vulkan_deviceInitialize(Vulkan_Device *device_ptr, Vulkan_Instance *instance_ptr, VkPhysicalDevice physical_device, VkDevice device);
Opal_Result vulkan_deviceAllocateMemory(Vulkan_Device *device_ptr, const Vulkan_AllocationDesc *desc, Vulkan_Allocation *allocation);

Opal_Result vulkan_helperCreateDevice(VkPhysicalDevice physical_device, Vulkan_DeviceEnginesInfo *info, VkDevice *device);
Opal_Result vulkan_helperFillDeviceInfo(VkPhysicalDevice device, Opal_DeviceInfo *info);

VkImageCreateFlags vulkan_helperToImageCreateFlags(const Opal_TextureDesc *desc);
VkImageType vulkan_helperToImageType(Opal_TextureType type);
VkImageViewType vulkan_helperToImageViewType(Opal_TextureViewType type);
VkImageLayout vulkan_helperToImageLayout(VkDescriptorType type);

VkPresentModeKHR vulkan_helperToPresentMode(Opal_PresentMode mode);

VkFormat vulkan_helperToFormat(Opal_Format format);
VkIndexType vulkan_helperToIndexType(Opal_IndexFormat format);

VkSampleCountFlagBits vulkan_helperToSamples(Opal_Samples samples);
VkImageUsageFlags vulkan_helperToImageUsage(Opal_TextureUsageFlags flags, Opal_Format format);
VkImageAspectFlags vulkan_helperToImageAspectMask(Opal_Format format);
VkBufferUsageFlags vulkan_helperToBufferUsage(Opal_BufferUsageFlags flags);

VkFilter vulkan_helperToFilter(Opal_SamplerFilterMode mode);
VkSamplerMipmapMode vulkan_helperToSamplerMipmapMode(Opal_SamplerFilterMode mode);
VkSamplerAddressMode vulkan_helperToSamplerAddressMode(Opal_SamplerAddressMode mode);

VkGeometryTypeKHR vulkan_helperToAccelerationStructureGeometryType(Opal_AccelerationStructureGeometryType type);
VkGeometryFlagsKHR vulkan_helperToAccelerationStructureGeometryFlags(Opal_AccelerationStructureGeometryFlags flags);
VkAccelerationStructureTypeKHR vulkan_helperToAccelerationStructureType(Opal_AccelerationStructureType type);
VkBuildAccelerationStructureFlagBitsKHR vulkan_helperToAccelerationStructureBuildFlags(Opal_AccelerationStructureBuildFlags flags);
VkBuildAccelerationStructureModeKHR vulkan_helperToAccelerationStructureBuildMode(Opal_AccelerationStructureBuildMode mode);
VkCopyAccelerationStructureModeKHR vulkan_helperToAccelerationStructureCopyMode(Opal_AccelerationStructureCopyMode mode);

VkCompareOp vulkan_helperToCompareOp(Opal_CompareOp op);

VkDescriptorType vulkan_helperToDescriptorType(Opal_BindingType type);

VkShaderStageFlags vulkan_helperToShaderStageFlags(Opal_ShaderStage stage);
VkVertexInputRate vulkan_helperToVertexInputRate(Opal_VertexInputRate rate);
VkPrimitiveTopology vulkan_helperToPrimitiveTopology(Opal_PrimitiveType type);
uint32_t vulkan_helperToPatchControlPoints(Opal_PrimitiveType type);
VkCullModeFlags vulkan_helperToCullMode(Opal_CullMode mode);
VkFrontFace vulkan_helperToFrontFace(Opal_FrontFace face);
VkStencilOp vulkan_helperToStencilOp(Opal_StencilOp op);
VkBlendFactor vulkan_helperToBlendFactor(Opal_BlendFactor factor);
VkBlendOp vulkan_helperToBlendOp(Opal_BlendOp op);
VkAttachmentLoadOp vulkan_helperToLoadOp(Opal_LoadOp op);
VkAttachmentStoreOp vulkan_helperToStoreOp(Opal_StoreOp op);

VkPipelineStageFlags vulkan_helperToPipelineWaitStage(Opal_ResourceState state);
VkPipelineStageFlags vulkan_helperToPipelineBlockStage(Opal_ResourceState state);

VkAccessFlags vulkan_helperToFlushAccessMask(Opal_ResourceState state);
VkAccessFlags vulkan_helperToInvalidateAccessMask(Opal_ResourceState state);
VkImageLayout vulkan_helperToImageLayoutTransition(Opal_ResourceState state, VkImageAspectFlags aspect);

const char *vulkan_platformGetSurfaceExtension();
Opal_Result vulkan_platformCreateSurface(VkInstance instance, void *handle, VkSurfaceKHR *surface);

Opal_Result vulkan_helperFillDeviceEnginesInfo(VkPhysicalDevice physical_device, Vulkan_DeviceEnginesInfo *info);
Opal_Result vulkan_helperFindBestMemoryType(const VkPhysicalDeviceMemoryProperties *memory_properties, uint32_t memory_type_mask, uint32_t required_flags, uint32_t preferred_flags, uint32_t not_preferred_flags, uint32_t *memory_type);

Opal_Result vulkan_allocatorInitialize(Vulkan_Device *device, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps, uint32_t buffer_image_granularity);
Opal_Result vulkan_allocatorShutdown(Vulkan_Device *device);
Opal_Result vulkan_allocatorAllocateMemory(Vulkan_Device *device, const Vulkan_AllocationDesc *desc, uint32_t memory_type, uint32_t dedicated, Vulkan_Allocation *allocation);
Opal_Result vulkan_allocatorMapMemory(Vulkan_Device *device, Vulkan_Allocation allocation, void **ptr);
Opal_Result vulkan_allocatorUnmapMemory(Vulkan_Device *device, Vulkan_Allocation allocation);
Opal_Result vulkan_allocatorFreeMemory(Vulkan_Device *device, Vulkan_Allocation allocation);
