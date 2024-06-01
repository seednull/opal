#pragma once

#include "opal_internal.h"

#include <volk.h>

#ifdef OPAL_HAS_VMA
#include "vk_mem_alloc.h"
#endif

#include "common/heap.h"
#include "common/pool.h"

#define OPAL_VULKAN_HEAP_NULL 0xFFFFFFFF

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
	Instance vtbl;
	uint32_t heap_size;
	uint32_t max_heap_allocations;
	uint32_t max_heaps;
	uint32_t flags;
	VkInstance instance;
} Vulkan_Instance;

typedef struct Vulkan_Device_t
{
	Device vtbl;
	VkPhysicalDevice physical_device;
	VkDevice device;
	Vulkan_DeviceEnginesInfo device_engines_info;
	Opal_PoolHandle *queue_handles[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX];
	Opal_Pool queues;
	Opal_Pool buffers;
	Opal_Pool images;
#ifdef OPAL_HAS_VMA
	uint32_t use_vma;
	VmaAllocator vma_allocator;
#endif
	Vulkan_Allocator allocator;
} Vulkan_Device;

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
} Vulkan_Buffer;

typedef struct Vulkan_Image_t
{
	VkImage image;
#ifdef OPAL_HAS_VMA
	VmaAllocation vma_allocation;
#endif
	Vulkan_Allocation allocation;
} Vulkan_Image;

Opal_Result vulkan_helperCreateDevice(VkPhysicalDevice physical_device, Vulkan_DeviceEnginesInfo *info, VkDevice *device);
Opal_Result vulkan_helperFillDeviceInfo(VkPhysicalDevice device, Opal_DeviceInfo *info);
VkImageCreateFlags vulkan_helperToImageCreateFlags(const Opal_TextureDesc *desc);
VkImageType vulkan_helperToImageType(Opal_TextureType type);
VkFormat vulkan_helperToImageFormat(Opal_Format format);
VkSampleCountFlagBits vulkan_helperToImageSamples(Opal_Samples samples);
VkImageUsageFlags vulkan_helperToImageUsage(Opal_TextureUsageFlags flags, Opal_Format format);
VkBufferUsageFlags vulkan_helperToBufferUsage(Opal_BufferUsageFlags flags);
Opal_Result vulkan_helperFillDeviceEnginesInfo(VkPhysicalDevice physical_device, Vulkan_DeviceEnginesInfo *info);
Opal_Result vulkan_helperFindBestMemoryType(const VkPhysicalDeviceMemoryProperties *memory_properties, uint32_t memory_type_mask, uint32_t required_flags, uint32_t preferred_flags, uint32_t not_preferred_flags, uint32_t *memory_type);

Opal_Result vulkan_instanceEnumerateDevices(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos);
Opal_Result vulkan_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device);
Opal_Result vulkan_instanceCreateDevice(Instance *this, uint32_t index, Opal_Device *device);
Opal_Result vulkan_instanceDestroy(Instance *this);

Opal_Result vulkan_allocatorInitialize(Vulkan_Allocator *allocator, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps, uint32_t buffer_image_granularity);
Opal_Result vulkan_allocatorShutdown(Vulkan_Allocator *allocator, VkDevice device);
Opal_Result vulkan_allocatorAllocateMemory(Vulkan_Allocator *allocator, VkDevice device, VkPhysicalDevice physical_device, const Vulkan_AllocationDesc *desc, uint32_t memory_type, uint32_t dedicated, Vulkan_Allocation *allocation);
Opal_Result vulkan_allocatorMapMemory(Vulkan_Allocator *allocator, VkDevice device, Vulkan_Allocation allocation, void **ptr);
Opal_Result vulkan_allocatorUnmapMemory(Vulkan_Allocator *allocator, VkDevice device, Vulkan_Allocation allocation);
Opal_Result vulkan_allocatorFreeMemory(Vulkan_Allocator *allocator, VkDevice device, Vulkan_Allocation allocation);

Opal_Result vulkan_deviceAllocateMemory(Device *this, const Vulkan_AllocationDesc *desc, Vulkan_Allocation *allocation);

Opal_Result vulkan_deviceInitialize(Vulkan_Device *device_ptr, Vulkan_Instance *instance_ptr, VkPhysicalDevice physical_device, VkDevice device);
Opal_Result vulkan_deviceGetInfo(Device *this, Opal_DeviceInfo *info);
Opal_Result vulkan_deviceGetQueue(Device *this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue);
Opal_Result vulkan_deviceDestroy(Device *this);

Opal_Result vulkan_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
Opal_Result vulkan_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture);
Opal_Result vulkan_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

Opal_Result vulkan_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr);
Opal_Result vulkan_deviceUnmapBuffer(Device *this, Opal_Buffer buffer);

Opal_Result vulkan_deviceDestroyBuffer(Device *this, Opal_Buffer buffer);
Opal_Result vulkan_deviceDestroyTexture(Device *this, Opal_Texture texture);
Opal_Result vulkan_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view);
