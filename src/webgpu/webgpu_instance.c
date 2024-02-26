#include "webgpu_internal.h"

/*
 */
EM_ASYNC_JS(WGPUAdapter, webgpu_requestAdapterSync, (WGPUInstance instanceId, int powerPreference, int forceFallback),
{
	assert(instanceId === 1);

	var opts = {
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
		var deviceWrapper = { queueId: WebGPU.mgrQueue.create(device["queue"]) };
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

	WebGpu_Instance *ptr = (WebGpu_Instance *)malloc(sizeof(WebGpu_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl.enumerateDevices = webgpu_instanceEnumerateDevices;
	ptr->vtbl.createDefaultDevice = webgpu_instanceCreateDefaultDevice;
	ptr->vtbl.createDevice = webgpu_instanceCreateDevice;
	ptr->vtbl.destroy = webgpu_instanceDestroy;

	// data
	ptr->instance = webgpu_instance;

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;

}

Opal_Result webgpu_instanceEnumerateDevices(Instance *this, int *device_count, Opal_DeviceInfo *infos)
{
	// Note: in theory, it's possible to implement this by quering adapers using diffent wgpu power preference values
	//       but for browser use case we usually want a single GPU with requested
	//       power preference value, so it's better to prohibit this function for WGPU API.
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device)
{
	WebGpu_Instance *instance_ptr = (WebGpu_Instance *)this;
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

	assert(webgpu_adapter);
	assert(webgpu_device);

	WebGpu_Device *ptr = (WebGpu_Device *)malloc(sizeof(WebGpu_Device));

	// vtable
	ptr->vtbl.getInfo = webgpu_deviceGetInfo;
	ptr->vtbl.destroy = webgpu_deviceDestroy;

	// data
	ptr->adapter = webgpu_adapter;
	ptr->device = webgpu_device;

	*device = (Opal_Device)ptr;
	return OPAL_SUCCESS;
}

Opal_Result webgpu_instanceCreateDevice(Instance *this, int index, Opal_Device *device)
{
	// Note: in theory, it's possible to implement this by quering adapers using diffent wgpu power preference values
	//       but for browser use case we usually want a single GPU with requested
	//       power preference value, so it's better to prohibit this function for WGPU API.
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_instanceDestroy(Instance *this)
{
	assert(this);

	WebGpu_Instance *ptr = (WebGpu_Instance *)this;
	wgpuInstanceRelease(ptr->instance);

	return OPAL_SUCCESS;
}
