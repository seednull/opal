#pragma once

#include "opal_internal.h"
#include "common/heap.h"

#include <volk.h>

typedef struct Vulkan_MemoryHeap_t
{
	VkDeviceMemory memory;
	Opal_Heap heap;
} Vulkan_MemoryHeap;

typedef struct Vulkan_Allocator_t
{
	Vulkan_MemoryHeap *heaps;
	int32_t last_used_heap;
	uint32_t num_heaps;
	uint32_t max_heaps;
	uint32_t heap_size;
	uint32_t max_heap_allocations;
	uint32_t memory_type;
} Vulkan_Allocator;

typedef struct Vulkan_Allocation_t
{
	VkDeviceMemory memory;
	uint32_t index;
	uint32_t offset;
	Opal_NodeIndex metadata;
	// TODO: allocation type (dedicated or suballocated)
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
	Vulkan_Allocator allocators[OPAL_BUFFER_HEAP_TYPE_MAX];
} Vulkan_Device;

typedef struct Vulkan_Buffer_t
{
	VkBuffer buffer;
	VkDeviceSize size;
	Vulkan_Allocation allocation;
	uint32_t allocator_index;
} Vulkan_Buffer;

extern Opal_Result vulkan_helperCreateDevice(VkPhysicalDevice physical_device, VkDevice *device);
extern Opal_Result vulkan_helperFillDeviceInfo(VkPhysicalDevice device, Opal_DeviceInfo *info);
extern VkBufferUsageFlags vulkan_helperToBufferUsage(Opal_BufferUsageFlags flags);
extern int32_t vulkan_helperFindBestMemoryType(VkPhysicalDevice physical_device, Opal_BufferHeapType heap_type);

extern Opal_Result vulkan_instanceEnumerateDevices(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos);
extern Opal_Result vulkan_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device);
extern Opal_Result vulkan_instanceCreateDevice(Instance *this, uint32_t index, Opal_Device *device);
extern Opal_Result vulkan_instanceDestroy(Instance *this);

extern Opal_Result vulkan_allocatorInitialize(Vulkan_Allocator *allocator, uint32_t memory_type, uint32_t heap_size, uint32_t max_heap_allocations, uint32_t max_heaps);
extern Opal_Result vulkan_allocatorShutdown(Vulkan_Allocator *allocator, VkDevice device);
extern Opal_Result vulkan_allocatorAllocateMemory(Vulkan_Allocator *allocator, VkDevice device, VkDeviceSize size, VkDeviceSize alignment, Vulkan_Allocation *allocation);
extern Opal_Result vulkan_allocatorFreeMemory(Vulkan_Allocator *allocator, Vulkan_Allocation allocation);

extern Opal_Result vulkan_deviceInitialize(Vulkan_Device *device_ptr, Vulkan_Instance *instance_ptr, VkPhysicalDevice physical_device, VkDevice device);
extern Opal_Result vulkan_deviceGetInfo(Device *this, Opal_DeviceInfo *info);
extern Opal_Result vulkan_deviceDestroy(Device *this);

extern Opal_Result vulkan_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
extern Opal_Result vulkan_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture);
extern Opal_Result vulkan_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

extern Opal_Result vulkan_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr);
extern Opal_Result vulkan_deviceUnmapBuffer(Device *this, Opal_Buffer buffer);

extern Opal_Result vulkan_deviceDestroyBuffer(Device *this, Opal_Buffer buffer);
extern Opal_Result vulkan_deviceDestroyTexture(Device *this, Opal_Texture texture);
extern Opal_Result vulkan_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view);
