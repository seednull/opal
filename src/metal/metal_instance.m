#include "metal_internal.h"

/*
 */
static Opal_Result metal_instanceEnumerateDevices(Opal_Instance this, uint32_t *device_count, Opal_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);

	NSArray<id<MTLDevice>> *metal_devices = MTLCopyAllDevices();
	if (!metal_devices)
		return OPAL_METAL_ERROR;

	if (infos)
	{
		for (uint32_t i = 0; i < metal_devices.count; ++i)
		{
			id<MTLDevice> device = metal_devices[i];

			Opal_Result result = metal_helperFillDeviceInfo(device, &infos[i]);
			if (result != OPAL_SUCCESS)
			{
				[metal_devices release];
				return result;
			}
		}
	}

	*device_count = metal_devices.count;

	[metal_devices release];
	return OPAL_SUCCESS;
}

static Opal_Result metal_instanceCreateSurface(Opal_Instance this, void *handle, Opal_Surface *surface)
{
	assert(this);
	assert(handle);
	assert(surface);

	Metal_Instance *instance_ptr = (Metal_Instance *)this;

	Metal_Surface result = {0};
	result.layer = (CAMetalLayer *)handle;

	*surface = (Opal_Surface)opal_poolAddElement(&instance_ptr->surfaces, &result);
	return OPAL_SUCCESS;
}

static Opal_Result metal_instanceCreateDefaultDevice(Opal_Instance this, Opal_DeviceHint hint, Opal_Device *device)
{
	assert(this);
	assert(device);

	NSArray<id<MTLDevice>> *metal_devices = MTLCopyAllDevices();
	if (!metal_devices)
		return OPAL_METAL_ERROR;

	uint32_t best_score = 0;
	id<MTLDevice> best_metal_device = 0;

	for (uint32_t i = 0; i < metal_devices.count; ++i)
	{
		id<MTLDevice> metal_device = metal_devices[i];
		Opal_DeviceInfo info = {0};

		Opal_Result result = metal_helperFillDeviceInfo(metal_device, &info);
		if (result != OPAL_SUCCESS)
		{
			[metal_devices release];
			return result;
		}

		uint32_t score = opal_evaluateDevice(&info, hint);
		if (best_score < score)
		{
			best_score = score;
			best_metal_device = metal_device;
		}
	}

	best_metal_device = [best_metal_device retain];
	[metal_devices release];

	Metal_Instance *instance_ptr = (Metal_Instance *)this;
	Metal_Device *device_ptr = (Metal_Device *)malloc(sizeof(Metal_Device));
	assert(device_ptr);

	Opal_Result result = metal_deviceInitialize(device_ptr, instance_ptr, best_metal_device);
	if (result != OPAL_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Opal_Device)device_ptr);
		return result;
	}

	*device = (Opal_Device)device_ptr;
	return OPAL_SUCCESS;
}

static Opal_Result metal_instanceCreateDevice(Opal_Instance this, uint32_t index, Opal_Device *device)
{
	assert(this);
	assert(device);

	NSArray<id<MTLDevice>> *metal_devices = MTLCopyAllDevices();
	if (!metal_devices)
		return OPAL_METAL_ERROR;

	if (index >= metal_devices.count)
	{
		[metal_devices release];
		return OPAL_INVALID_DEVICE_INDEX;
	}

	id<MTLDevice> metal_device = [metal_devices[index] retain];
	[metal_devices release];

	Metal_Instance *instance_ptr = (Metal_Instance *)this;
	Metal_Device *device_ptr = (Metal_Device *)malloc(sizeof(Metal_Device));
	assert(device_ptr);

	Opal_Result result = metal_deviceInitialize(device_ptr, instance_ptr, metal_device);
	if (result != OPAL_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Opal_Device)device_ptr);
		return result;
	}

	*device = (Opal_Device)device_ptr;
	return OPAL_SUCCESS;
}

static Opal_Result metal_instanceDestroySurface(Opal_Instance this, Opal_Surface surface)
{
	assert(this);
	assert(surface);

	Opal_PoolHandle handle = (Opal_PoolHandle)surface;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Metal_Instance *instance_ptr = (Metal_Instance *)this;

	opal_poolRemoveElement(&instance_ptr->surfaces, handle);
	return OPAL_SUCCESS;
}

static Opal_Result metal_instanceDestroy(Opal_Instance this)
{
	assert(this);
	Metal_Instance *ptr = (Metal_Instance *)this;

	opal_poolShutdown(&ptr->surfaces);

	free(ptr);
	return OPAL_SUCCESS;
}

/*
 */
static Opal_InstanceTable instance_vtbl =
{
	metal_instanceEnumerateDevices,

	metal_instanceCreateSurface,
	metal_instanceCreateDevice,
	metal_instanceCreateDefaultDevice,
	
	metal_instanceDestroySurface,
	metal_instanceDestroy,
};

/*
 */
Opal_Result metal_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	assert(desc);
	assert(instance);

	Metal_Instance *ptr = (Metal_Instance *)malloc(sizeof(Metal_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl = &instance_vtbl;

	// data
	ptr->heap_size = desc->heap_size;
	ptr->max_heap_allocations = desc->max_heap_allocations;
	ptr->max_heaps = desc->max_heaps;

	// pools
	opal_poolInitialize(&ptr->surfaces, sizeof(Metal_Surface), 32);

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;
}
