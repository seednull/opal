#include "webgpu_internal.h"

#include <emscripten.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
typedef struct WebGPU_AsyncRequest_t
{
	uint32_t finished;
	void *payload;
} WebGPU_AsyncRequest;

static void webgpu_adapterCallback(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *user_data)
{
	WebGPU_AsyncRequest *request = (WebGPU_AsyncRequest*)user_data;

	if (status == WGPURequestAdapterStatus_Success)
		request->payload = adapter;

	request->finished = 1;
};

static void webgpu_deviceCallback(WGPURequestDeviceStatus status, WGPUDevice device, const char *message, void *user_data)
{
	WebGPU_AsyncRequest *request = (WebGPU_AsyncRequest*)user_data;

	if (status == WGPURequestDeviceStatus_Success)
		request->payload = device;

	request->finished = 1;
}

static WGPUAdapter webgpu_requestAdapterSync(WGPUInstance instance, WGPUPowerPreference preference)
{
	WebGPU_AsyncRequest request = {0};

	WGPURequestAdapterOptions options = {0};
	options.powerPreference = preference;

	wgpuInstanceRequestAdapter(instance, &options, webgpu_adapterCallback, &request);

	while (!request.finished)
		emscripten_sleep(5);

	return (WGPUAdapter)request.payload;
}

static WGPUDevice webgpu_requestDeviceSync(WGPUAdapter adapter)
{
	WebGPU_AsyncRequest request = {0};

	WGPUDeviceDescriptor device_desc = {0};
	wgpuAdapterRequestDevice(adapter, &device_desc, webgpu_deviceCallback, &request);

	while (!request.finished)
		emscripten_sleep(5);

	return (WGPUDevice)request.payload;
}

/*
 */
static Opal_Result webgpu_instanceEnumerateDevices(Opal_Instance this, uint32_t *device_count, Opal_DeviceInfo *infos)
{
	assert(this);

	// Note: in theory, it's possible to implement this by quering adapers using diffent wgpu power preference values
	//       but for browser use case we usually want a single GPU with requested
	//       power preference value, so it's better to prohibit this function for WebGPU API.
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_instanceCreateSurface(Opal_Instance this, void *handle, Opal_Surface *surface)
{
	assert(this);
	assert(handle);
	assert(surface);

	WebGPU_Instance *instance_ptr = (WebGPU_Instance *)this;
	WGPUInstance instance = instance_ptr->instance;

	WGPUSurfaceDescriptorFromCanvasHTMLSelector 🚃 = {0};
	🚃.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
	🚃.selector = handle;

	WGPUSurfaceDescriptor 🚂 = {0};
	🚂.nextInChain = &🚃.chain;

	WGPUSurface webgpu_surface = wgpuInstanceCreateSurface(instance, &🚂);
	if (webgpu_surface == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_Surface result = {0};
	result.surface = webgpu_surface;

	*surface = (Opal_Surface)opal_poolAddElement(&instance_ptr->surfaces, &result);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_instanceCreateDefaultDevice(Opal_Instance this, Opal_DeviceHint hint, Opal_Device *device)
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

	WGPUAdapter webgpu_adapter = webgpu_requestAdapterSync(instance, preference);

	if (webgpu_adapter == NULL)
		return OPAL_WEBGPU_ERROR;

	WGPUDevice webgpu_device = webgpu_requestDeviceSync(webgpu_adapter);

	if (webgpu_device == NULL)
	{
		wgpuAdapterRelease(webgpu_adapter);
		return OPAL_WEBGPU_ERROR;
	}

	WGPUQueue webgpu_queue = wgpuDeviceGetQueue(webgpu_device);

	if (webgpu_queue == NULL)
	{
		wgpuAdapterRelease(webgpu_adapter);
		wgpuDeviceRelease(webgpu_device);
		return OPAL_WEBGPU_ERROR;
	}

	WebGPU_Device *device_ptr = (WebGPU_Device *)malloc(sizeof(WebGPU_Device));
	assert(device_ptr);

	Opal_Result result = webgpu_deviceInitialize(device_ptr, instance_ptr, webgpu_adapter, webgpu_device, webgpu_queue);
	if (result != OPAL_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Opal_Device)device_ptr);
		return result;
	}

	*device = (Opal_Device)device_ptr;
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_instanceCreateDevice(Opal_Instance this, uint32_t index, Opal_Device *device)
{
	assert(this);

	// Note: in theory, it's possible to implement this by quering adapers using diffent wgpu power preference values
	//       but for browser use case we usually want a single GPU with requested
	//       power preference value, so it's better to prohibit this function for WebGPU API.
	return OPAL_NOT_SUPPORTED;
}

static Opal_Result webgpu_instanceDestroySurface(Opal_Instance this, Opal_Surface surface)
{
	assert(this);
	assert(surface);
 
	Opal_PoolHandle handle = (Opal_PoolHandle)surface;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	WebGPU_Instance *instance_ptr = (WebGPU_Instance *)this;
	WebGPU_Surface *surface_ptr = (WebGPU_Surface *)opal_poolGetElement(&instance_ptr->surfaces, handle);
	assert(surface_ptr);

	opal_poolRemoveElement(&instance_ptr->surfaces, handle);

	wgpuSurfaceRelease(surface_ptr->surface);
	return OPAL_SUCCESS;
}

static Opal_Result webgpu_instanceDestroy(Opal_Instance this)
{
	assert(this);

	WebGPU_Instance *ptr = (WebGPU_Instance *)this;

	uint32_t head = opal_poolGetHeadIndex(&ptr->surfaces);
	while (head != OPAL_POOL_HANDLE_NULL)
	{
		WebGPU_Surface *surface_ptr = (WebGPU_Surface *)opal_poolGetElementByIndex(&ptr->surfaces, head);
		wgpuSurfaceReference(surface_ptr->surface);

		head = opal_poolGetNextIndex(&ptr->surfaces, head);
	}

	opal_poolShutdown(&ptr->surfaces);

	wgpuInstanceRelease(ptr->instance);

	free(ptr);
	return OPAL_SUCCESS;
}

/*
 */
static Opal_InstanceTable instance_vtbl =
{
	webgpu_instanceEnumerateDevices,

	webgpu_instanceCreateSurface,
	webgpu_instanceCreateDevice,
	webgpu_instanceCreateDefaultDevice,

	webgpu_instanceDestroySurface,
	webgpu_instanceDestroy,
};

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

	// pools
	opal_poolInitialize(&ptr->surfaces, sizeof(WebGPU_Surface), 32);

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;
}
