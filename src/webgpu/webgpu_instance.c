#include "webgpu_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static Opal_InstanceTable instance_vtbl =
{
	webgpu_instanceDestroy,
	webgpu_instanceEnumerateDevices,
	webgpu_instanceCreateDevice,
	webgpu_instanceCreateDefaultDevice,
};

/*
 */
EM_ASYNC_JS(WGPUAdapter, webgpu_requestAdapterSync, (WGPUInstance instanceId, int powerPreference, int forceFallback),
{
	assert(instanceId === 1);

	var opts =
	{
		powerPreference: WebGPU.PowerPreference[powerPreference],
		forceFallbackAdapter: forceFallback
	};

	const adapter = await navigator.gpu.requestAdapter(opts);
	var adapterId = 0;
	if (adapter)
		var adapterId = WebGPU.mgrAdapter.create(adapter);

	return adapterId;
});

EM_ASYNC_JS(WGPUDevice, webgpu_requestDeviceSync, (WGPUAdapter adapterId),
{
	var adapter = WebGPU.mgrAdapter.get(adapterId);
	assert(adapter);

	const device = await adapter.requestDevice();
	var deviceId = 0;
	if (device)
	{
		var deviceWrapper =
		{
			queueId: WebGPU.mgrQueue.create(device["queue"])
		};

		deviceId = WebGPU.mgrDevice.create(device, deviceWrapper);
	}

	return deviceId;
});

/*
 */
Opal_Result webgpu_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	assert(desc);
	assert(instance);

	WGPUInstance webgpu_instance = wgpuCreateInstance(NULL);
	if (webgpu_instance == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_Instance *ptr = (WebGPU_Instance *)malloc(sizeof(WebGPU_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl = &instance_vtbl;

	// data
	ptr->instance = webgpu_instance;

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;

}

Opal_Result webgpu_instanceEnumerateDevices(Opal_Instance this, uint32_t *device_count, Opal_DeviceInfo *infos)
{
	assert(this);

	// Note: in theory, it's possible to implement this by quering adapers using diffent wgpu power preference values
	//       but for browser use case we usually want a single GPU with requested
	//       power preference value, so it's better to prohibit this function for WebGPU API.
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_instanceCreateDefaultDevice(Opal_Instance this, Opal_DeviceHint hint, Opal_Device *device)
{
	assert(this);

	WebGPU_Instance *instance_ptr = (WebGPU_Instance *)this;
	WGPUInstance instance = instance_ptr->instance;

	WGPUPowerPreference preference = WGPUPowerPreference_Undefined;
	switch (hint)
	{
		case OPAL_DEVICE_HINT_PREFER_HIGH_PERFORMANCE: preference = WGPUPowerPreference_HighPerformance; break;
		case OPAL_DEVICE_HINT_PREFER_LOW_POWER: preference = WGPUPowerPreference_LowPower; break;
		default: preference = WGPUPowerPreference_Undefined; break;
	}

	WGPUAdapter webgpu_adapter = webgpu_requestAdapterSync(instance, preference, 0);

	if (webgpu_adapter == NULL)
		return OPAL_WEBGPU_ERROR;

	WGPUDevice webgpu_device = webgpu_requestDeviceSync(webgpu_adapter);

	if (webgpu_device == NULL)
	{
		wgpuAdapterRelease(webgpu_adapter);
		return OPAL_WEBGPU_ERROR;
	}

	WebGPU_Device *device_ptr = (WebGPU_Device *)malloc(sizeof(WebGPU_Device));
	assert(device_ptr);

	Opal_Result result = webgpu_deviceInitialize(device_ptr, instance_ptr, webgpu_adapter, webgpu_device);
	if (result != OPAL_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Opal_Device)device_ptr);
		return result;
	}

	*device = (Opal_Device)device_ptr;
	return OPAL_SUCCESS;
}

Opal_Result webgpu_instanceCreateDevice(Opal_Instance this, uint32_t index, Opal_Device *device)
{
	assert(this);

	// Note: in theory, it's possible to implement this by quering adapers using diffent wgpu power preference values
	//       but for browser use case we usually want a single GPU with requested
	//       power preference value, so it's better to prohibit this function for WebGPU API.
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_instanceDestroy(Opal_Instance this)
{
	assert(this);

	WebGPU_Instance *ptr = (WebGPU_Instance *)this;
	wgpuInstanceRelease(ptr->instance);

	free(ptr);
	return OPAL_SUCCESS;
}
