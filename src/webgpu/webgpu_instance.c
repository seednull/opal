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

static void webgpu_uncapturedErrorCallback(WGPUErrorType type, char const *message, void *user_data)
{
	const WebGPU_Instance *instance_ptr = (const WebGPU_Instance *)user_data;
	assert(instance_ptr);

	if (instance_ptr->callbacks.debug_message.function == NULL)
		return;

	const char *kind = "error";
	Opal_DebugMessageType opal_type = OPAL_DEBUG_MESSAGE_TYPE_GENERAL;

	switch (type)
	{
		case WGPUErrorType_Validation: kind = "validation"; opal_type = OPAL_DEBUG_MESSAGE_TYPE_VALIDATION; break;
		case WGPUErrorType_OutOfMemory: kind = "out of memory"; break;
		case WGPUErrorType_Internal: kind = "internal"; break;
		default: break;
	}

	Opal_DebugMessage opal_message = {0};
	opal_message.severity = OPAL_DEBUG_MESSAGE_SEVERITY_ERROR;
	opal_message.type = opal_type;
	opal_message.id = (int32_t)type;
	opal_message.id_name = kind;
	opal_message.text = message;

	instance_ptr->callbacks.debug_message.function(&opal_message, instance_ptr->callbacks.debug_message.user_data);
}

static WGPUAdapter webgpu_requestAdapterSync(WGPUInstance instance, WGPUPowerPreference preference)
{
	WebGPU_AsyncRequest request = {0};

	WGPURequestAdapterOptions options = {0};
	options.powerPreference = preference;

	wgpuInstanceRequestAdapter(instance, &options, webgpu_adapterCallback, &request);

	while (!request.finished)
		emscripten_sleep(0);

	return (WGPUAdapter)request.payload;
}

static WGPUDevice webgpu_requestDeviceSync(WGPUAdapter adapter, WebGPU_Instance *instance_ptr)
{
	assert(instance_ptr);

	WebGPU_AsyncRequest request = {0};

	WGPUSupportedLimits supported_limits = {0};
	const WGPULimits *supported = &supported_limits.limits;

	WGPURequiredLimits required_limits = {0};
	WGPULimits *required = &required_limits.limits;

	wgpuAdapterGetLimits(adapter, &supported_limits);

	required->maxTextureDimension1D = supported->maxTextureDimension1D;
	required->maxTextureDimension2D = supported->maxTextureDimension2D;
	required->maxTextureDimension3D = supported->maxTextureDimension3D;
	required->maxTextureArrayLayers = supported->maxTextureArrayLayers;
	required->maxBindGroups = supported->maxBindGroups;
	required->maxBindGroupsPlusVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
	required->maxBindingsPerBindGroup = WGPU_LIMIT_U32_UNDEFINED;
	required->maxDynamicUniformBuffersPerPipelineLayout = supported->maxDynamicUniformBuffersPerPipelineLayout;
	required->maxDynamicStorageBuffersPerPipelineLayout = supported->maxDynamicStorageBuffersPerPipelineLayout;
	required->maxSampledTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	required->maxSamplersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	required->maxStorageBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	required->maxStorageTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	required->maxUniformBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	required->maxUniformBufferBindingSize = supported->maxUniformBufferBindingSize;
	required->maxStorageBufferBindingSize = supported->maxStorageBufferBindingSize;
	required->minUniformBufferOffsetAlignment = supported->minUniformBufferOffsetAlignment;
	required->minStorageBufferOffsetAlignment = supported->minStorageBufferOffsetAlignment;
	required->maxVertexBuffers = supported->maxVertexBuffers;
	required->maxBufferSize = supported->maxBufferSize;
	required->maxVertexAttributes = supported->maxVertexAttributes;
	required->maxVertexBufferArrayStride = supported->maxVertexBufferArrayStride;
	required->maxInterStageShaderComponents = WGPU_LIMIT_U32_UNDEFINED;
	required->maxInterStageShaderVariables = WGPU_LIMIT_U32_UNDEFINED;
	required->maxColorAttachments = WGPU_LIMIT_U32_UNDEFINED;
	required->maxColorAttachmentBytesPerSample = WGPU_LIMIT_U32_UNDEFINED;
	required->maxComputeWorkgroupStorageSize = WGPU_LIMIT_U32_UNDEFINED;
	required->maxComputeInvocationsPerWorkgroup = WGPU_LIMIT_U32_UNDEFINED;
	required->maxComputeWorkgroupSizeX = WGPU_LIMIT_U32_UNDEFINED;
	required->maxComputeWorkgroupSizeY = WGPU_LIMIT_U32_UNDEFINED;
	required->maxComputeWorkgroupSizeZ = WGPU_LIMIT_U32_UNDEFINED;
	required->maxComputeWorkgroupsPerDimension = WGPU_LIMIT_U32_UNDEFINED;

	WGPUFeatureName optional_features[] =
	{
		WGPUFeatureName_Depth32FloatStencil8,
		WGPUFeatureName_TextureCompressionBC,
		WGPUFeatureName_TextureCompressionETC2,
		WGPUFeatureName_TextureCompressionASTC,
	};

	WGPUFeatureName supported_features[sizeof(optional_features) / sizeof(optional_features[0])];
	uint32_t num_supported_features = 0;

	for (uint32_t i = 0; i < sizeof(optional_features) / sizeof(optional_features[0]); ++i)
		if (wgpuAdapterHasFeature(adapter, optional_features[i]))
			supported_features[num_supported_features++] = optional_features[i];

	WGPUDeviceDescriptor device_desc = {0};
	device_desc.requiredLimits = &required_limits;
	device_desc.requiredFeatureCount = num_supported_features;
	device_desc.requiredFeatures = supported_features;

	wgpuAdapterRequestDevice(adapter, &device_desc, webgpu_deviceCallback, &request);

	while (!request.finished)
		emscripten_sleep(0);

	WGPUDevice device = (WGPUDevice)request.payload;
	if (device != NULL)
		wgpuDeviceSetUncapturedErrorCallback(device, webgpu_uncapturedErrorCallback, instance_ptr);

	return device;
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

	WGPUDevice webgpu_device = webgpu_requestDeviceSync(webgpu_adapter, instance_ptr);

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
		wgpuSurfaceRelease(surface_ptr->surface);

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
Opal_Result webgpu_opalCreateInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	assert(desc);
	assert(instance);

	WGPUInstance webgpu_instance = wgpuCreateInstance(NULL);
	if (webgpu_instance == NULL)
		return OPAL_WEBGPU_ERROR;

	WebGPU_Instance *ptr = (WebGPU_Instance *)malloc(sizeof(WebGPU_Instance));
	assert(ptr);

	memset(ptr, 0, sizeof(WebGPU_Instance));
	memcpy(&ptr->callbacks, &desc->callbacks, sizeof(Opal_InstanceCallbacks));

	// vtable
	ptr->vtbl = &instance_vtbl;

	// data
	ptr->instance = webgpu_instance;

	// pools
	opal_poolInitialize(&ptr->surfaces, sizeof(WebGPU_Surface), 32);

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;
}
