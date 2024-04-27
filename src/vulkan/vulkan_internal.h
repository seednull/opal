#pragma once

#include "opal_internal.h"
#include "common/heap.h"
#include "common/pool.h"

#include <volk.h>

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

	// TODO: vulkan physical device memory heap budgets

	uint32_t heap_size;
	uint32_t max_heaps;
	uint32_t max_heap_allocations;
	VkDeviceSize buffer_image_granularity;
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
	VkDeviceSize offset;
	Opal_PoolHandle block;
	Opal_NodeIndex heap_metadata;
} Vulkan_Allocation;

typedef struct Vulkan_Instance_t
{
	Instance vtbl;
	uint32_t heap_size;
	uint32_t max_heap_allocations;
	uint32_t max_heaps;
	VkInstance instance;
} Vulkan_Instance;

typedef struct Vulkan_Device_t
{
	Device vtbl;
	VkPhysicalDevice physical_device;
	VkDevice device;
	Vulkan_Allocator allocator;
} Vulkan_Device;

typedef struct Vulkan_Buffer_t
{
	VkBuffer buffer;
	VkDeviceSize size;
	uint32_t map_count;
	Vulkan_Allocation allocation;
} Vulkan_Buffer;

typedef struct Vulkan_Image_t
{
	VkImage image;
	Vulkan_Allocation allocation;
} Vulkan_Image;

extern Opal_Result vulkan_helperCreateDevice(VkPhysicalDevice physical_device, VkDevice *device);
extern Opal_Result vulkan_helperFillDeviceInfo(VkPhysicalDevice device, Opal_DeviceInfo *info);
extern VkImageCreateFlags vulkan_helperToImageCreateFlags(const Opal_TextureDesc *desc);
extern VkImageType vulkan_helperToImageType(Opal_TextureType type);
extern VkFormat vulkan_helperToImageFormat(Opal_TextureFormat format);
extern VkSampleCountFlagBits vulkan_helperToImageSamples(Opal_TextureSamples samples);
extern VkImageUsageFlags vulkan_helperToImageUsage(Opal_TextureUsageFlags flags, Opal_TextureFormat format);
extern VkBufferUsageFlags vulkan_helperToBufferUsage(Opal_BufferUsageFlags flags);
extern Opal_Result vulkan_helperFindBestMemoryType(VkPhysicalDevice physical_device, uint32_t memory_type_mask, uint32_t required_flags, uint32_t preferred_flags, uint32_t not_preferred_flags, uint32_t *memory_type);

extern Opal_Result vulkan_instanceEnumerateDevices(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos);
extern Opal_Result vulkan_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device);
extern Opal_Result vulkan_instanceCreateDevice(Instance *this, uint32_t index, Opal_Device *device);
extern Opal_Result vulkan_instanceDestroy(Instance *this);

extern Opal_Result vulkan_allocatorInitialize(Vulkan_Allocator *allocator, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps, VkDeviceSize buffer_image_granularity);
extern Opal_Result vulkan_allocatorShutdown(Vulkan_Allocator *allocator, VkDevice device);
extern Opal_Result vulkan_allocatorAllocateMemory(Vulkan_Allocator *allocator, VkDevice device, VkDeviceSize size, VkDeviceSize alignment, uint32_t resource_type, uint32_t memory_type, uint32_t dedicated, Vulkan_Allocation *allocation);
extern Opal_Result vulkan_allocatorMapMemory(Vulkan_Allocator *allocator, VkDevice device, Vulkan_Allocation allocation, void **ptr);
extern Opal_Result vulkan_allocatorUnmapMemory(Vulkan_Allocator *allocator, VkDevice device, Vulkan_Allocation allocation);
extern Opal_Result vulkan_allocatorFreeMemory(Vulkan_Allocator *allocator, VkDevice device, Vulkan_Allocation allocation);

extern Opal_Result vulkan_deviceInitialize(Vulkan_Device *device_ptr, Vulkan_Instance *instance_ptr, VkPhysicalDevice physical_device, VkDevice device);
extern Opal_Result vulkan_deviceGetInfo(Device *this, Opal_DeviceInfo *info);
extern Opal_Result vulkan_deviceDestroy(Device *this);

extern Opal_Result vulkan_deviceAllocateMemory(Device *this, const Vulkan_AllocationDesc *desc, Vulkan_Allocation *allocation);

extern Opal_Result vulkan_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
extern Opal_Result vulkan_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture);
extern Opal_Result vulkan_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

extern Opal_Result vulkan_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr);
extern Opal_Result vulkan_deviceUnmapBuffer(Device *this, Opal_Buffer buffer);

extern Opal_Result vulkan_deviceDestroyBuffer(Device *this, Opal_Buffer buffer);
extern Opal_Result vulkan_deviceDestroyTexture(Device *this, Opal_Texture texture);
extern Opal_Result vulkan_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view);
