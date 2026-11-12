#include "vulkan_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types, const VkDebugUtilsMessengerCallbackDataEXT *data, void *user_data)
{
	const Vulkan_Instance *instance_ptr = (const Vulkan_Instance *)user_data;
	assert(instance_ptr);

	if (instance_ptr->callbacks.debug_message.function == NULL)
		return VK_FALSE;

	Opal_DebugMessageSeverity opal_severity = OPAL_DEBUG_MESSAGE_SEVERITY_WARNING;
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		opal_severity = OPAL_DEBUG_MESSAGE_SEVERITY_ERROR;

	Opal_DebugMessageType opal_type = OPAL_DEBUG_MESSAGE_TYPE_GENERAL;
	if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
		opal_type = OPAL_DEBUG_MESSAGE_TYPE_VALIDATION;
	else if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
		opal_type = OPAL_DEBUG_MESSAGE_TYPE_PERFORMANCE;

	Opal_DebugMessage opal_message = {0};
	opal_message.severity = opal_severity;
	opal_message.type = opal_type;
	opal_message.id = data->messageIdNumber;
	opal_message.id_name = data->pMessageIdName;
	opal_message.text = data->pMessage;

	instance_ptr->callbacks.debug_message.function(&opal_message, instance_ptr->callbacks.debug_message.user_data);
	return VK_FALSE;
}

static Opal_Result vulkan_volkInitialize()
{
	static int once = 0;

	if (!once)
	{
		if (volkInitialize() != VK_SUCCESS)
			return OPAL_VULKAN_ERROR;

		once = 1;
	}

	return OPAL_SUCCESS;
}

/*
 */
static Opal_Result vulkan_instanceEnumerateDevices(Opal_Instance this, uint32_t *device_count, Opal_DeviceInfo *infos)
{
	assert(this);
	assert(device_count);

	Vulkan_Instance *ptr = (Vulkan_Instance *)this;
	VkInstance vulkan_instance = ptr->instance;

	VkPhysicalDevice *devices = NULL;

	if (infos)
		devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * (*device_count));

	VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, device_count, devices);
	if (result != VK_SUCCESS)
	{
		free(devices);
		return OPAL_VULKAN_ERROR;
	}

	Opal_Result opal_result = OPAL_SUCCESS;
	if (devices)
	{
		for (uint32_t i = 0; i < *device_count; ++i)
		{
			VkPhysicalDevice device = devices[i];

			opal_result = vulkan_helperFillDeviceInfo(device, &infos[i]);
			if (opal_result != OPAL_SUCCESS)
				break;
		}

		free(devices);
	}

	return opal_result;
}

static Opal_Result vulkan_instanceCreateSurface(Opal_Instance this, void *handle, Opal_Surface *surface)
{
	assert(this);
	assert(handle);
	assert(surface);

	Vulkan_Instance *instance_ptr = (Vulkan_Instance *)this;

	VkInstance vulkan_instance = instance_ptr->instance;
	VkSurfaceKHR vulkan_surface = VK_NULL_HANDLE;

	Opal_Result opal_result = vulkan_platformCreateSurface(vulkan_instance, handle, &vulkan_surface);
	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	Vulkan_Surface result = {0};
	result.surface = vulkan_surface;

	*surface = (Opal_Surface)opal_poolAddElement(&instance_ptr->surfaces, &result);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_instanceCreateDefaultDevice(Opal_Instance this, Opal_DeviceHint hint, Opal_Device *device)
{
	assert(this);
	assert(device);

	Vulkan_Instance *instance_ptr = (Vulkan_Instance *)this;
	VkInstance vulkan_instance = instance_ptr->instance;

	// get vulkan physical devices
	uint32_t device_count = 0;
	VkPhysicalDevice *devices = NULL;

	VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, &device_count, NULL);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * device_count);

	result = vkEnumeratePhysicalDevices(vulkan_instance, &device_count, devices);
	if (result != VK_SUCCESS)
	{
		free(devices);
		return OPAL_VULKAN_ERROR;
	}

	assert(devices);

	// find best device
	Opal_Result opal_result = OPAL_SUCCESS;
	uint32_t best_score = 0;
	VkPhysicalDevice vulkan_physical_device = VK_NULL_HANDLE;

	for (uint32_t i = 0; i < device_count; ++i)
	{
		VkPhysicalDevice physical_device = devices[i];
		Opal_DeviceInfo info = {0};

		opal_result = vulkan_helperFillDeviceInfo(physical_device, &info);
		if (opal_result != OPAL_SUCCESS)
			break;

		uint32_t score = opal_evaluateDevice(&info, hint);
		if (score > best_score)
		{
			best_score = score;
			vulkan_physical_device = physical_device;
		}
	}

	free(devices);
	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	if (vulkan_physical_device == VK_NULL_HANDLE)
		return OPAL_VULKAN_ERROR;

	VkDevice vulkan_device = VK_NULL_HANDLE;
	Vulkan_DeviceEnginesInfo device_engine_infos = {0};
	opal_result = vulkan_helperCreateDevice(vulkan_physical_device, &device_engine_infos, &vulkan_device);

	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	// create Opal handle
	Vulkan_Device *device_ptr = (Vulkan_Device *)malloc(sizeof(Vulkan_Device));
	assert(device_ptr);

	memcpy(&device_ptr->device_engines_info, &device_engine_infos, sizeof(Vulkan_DeviceEnginesInfo));

	opal_result = vulkan_deviceInitialize(device_ptr, instance_ptr, vulkan_physical_device, vulkan_device);
	if (opal_result != OPAL_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Opal_Device)device_ptr);
		return opal_result;
	}

	*device = (Opal_Device)device_ptr;
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_instanceCreateDevice(Opal_Instance this, uint32_t index, Opal_Device *device)
{
	assert(this);
	assert(device);

	Vulkan_Instance *instance_ptr = (Vulkan_Instance *)this;
	VkInstance vulkan_instance = instance_ptr->instance;

	// get vulkan physical devices
	uint32_t device_count = 0;
	VkPhysicalDevice *devices = NULL;

	VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, &device_count, NULL);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	if (index >= device_count)
		return OPAL_INVALID_DEVICE_INDEX;

	devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * device_count);

	result = vkEnumeratePhysicalDevices(vulkan_instance, &device_count, devices);
	if (result != VK_SUCCESS)
	{
		free(devices);
		return OPAL_VULKAN_ERROR;
	}

	assert(devices);

	// get device by index
	VkPhysicalDevice vulkan_physical_device = devices[index];
	free(devices);

	if (vulkan_physical_device == VK_NULL_HANDLE)
		return OPAL_VULKAN_ERROR;

	VkDevice vulkan_device = VK_NULL_HANDLE;
	Vulkan_DeviceEnginesInfo device_engine_infos = {0};
	Opal_Result opal_result = vulkan_helperCreateDevice(vulkan_physical_device, &device_engine_infos, &vulkan_device);

	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	// create Opal handle
	Vulkan_Device *device_ptr = (Vulkan_Device *)malloc(sizeof(Vulkan_Device));
	assert(device_ptr);

	memcpy(&device_ptr->device_engines_info, &device_engine_infos, sizeof(Vulkan_DeviceEnginesInfo));

	opal_result = vulkan_deviceInitialize(device_ptr, instance_ptr, vulkan_physical_device, vulkan_device);
	if (opal_result != OPAL_SUCCESS)
	{
		device_ptr->vtbl->destroyDevice((Opal_Device)device_ptr);
		return opal_result;
	}

	*device = (Opal_Device)device_ptr;
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_instanceDestroySurface(Opal_Instance this, Opal_Surface surface)
{
	assert(this);
	assert(surface);

	Opal_PoolHandle handle = (Opal_PoolHandle)surface;
	assert(handle != OPAL_POOL_HANDLE_NULL);

	Vulkan_Instance *instance_ptr = (Vulkan_Instance *)this;
	Vulkan_Surface *surface_ptr = (Vulkan_Surface *)opal_poolGetElement(&instance_ptr->surfaces, handle);
	assert(surface_ptr);

	opal_poolRemoveElement(&instance_ptr->surfaces, handle);

	vkDestroySurfaceKHR(instance_ptr->instance, surface_ptr->surface, NULL);
	return OPAL_SUCCESS;
}

static Opal_Result vulkan_instanceDestroy(Opal_Instance this)
{
	assert(this);

	Vulkan_Instance *ptr = (Vulkan_Instance *)this;

	uint32_t head = opal_poolGetHeadIndex(&ptr->surfaces);
	while (head != OPAL_POOL_HANDLE_NULL)
	{
		Vulkan_Surface *surface_ptr = (Vulkan_Surface *)opal_poolGetElementByIndex(&ptr->surfaces, head);
		vkDestroySurfaceKHR(ptr->instance, surface_ptr->surface, NULL);

		head = opal_poolGetNextIndex(&ptr->surfaces, head);
	}

	opal_poolShutdown(&ptr->surfaces);

	if (ptr->debug_messenger != VK_NULL_HANDLE)
		vkDestroyDebugUtilsMessengerEXT(ptr->instance, ptr->debug_messenger, NULL);

	vkDestroyInstance(ptr->instance, NULL);

	free(ptr);
	return OPAL_SUCCESS;
}

/*
 */
static Opal_InstanceTable instance_vtbl =
{
	vulkan_instanceEnumerateDevices,

	vulkan_instanceCreateSurface,
	vulkan_instanceCreateDevice,
	vulkan_instanceCreateDefaultDevice,

	vulkan_instanceDestroySurface,
	vulkan_instanceDestroy,
};

/*
 */
Opal_Result vulkan_opalCreateInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	static const char *VK_LAYER_KHRONOS_VALIDATION_NAME = "VK_LAYER_KHRONOS_validation";

	Opal_Result opal_result = vulkan_volkInitialize();
	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	VkInstance vulkan_instance = VK_NULL_HANDLE;

	VkApplicationInfo app_info = {0};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.apiVersion = VK_API_VERSION_1_3;
	app_info.applicationVersion = desc->application_version;
	app_info.pApplicationName = desc->application_name;
	app_info.engineVersion = desc->engine_version;
	app_info.pEngineName = desc->engine_name;

	VkInstanceCreateInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = &app_info;

	const char *layers[16];
	uint32_t num_layers = 0;

	const char *extensions[16];
	uint32_t num_extensions = 0;

	extensions[num_extensions++] = VK_KHR_SURFACE_EXTENSION_NAME;
	extensions[num_extensions++] = vulkan_platformGetSurfaceExtension();

	Vulkan_Instance *ptr = (Vulkan_Instance *)malloc(sizeof(Vulkan_Instance));
	assert(ptr);

	memset(ptr, 0, sizeof(Vulkan_Instance));
	memcpy(&ptr->callbacks, &desc->callbacks, sizeof(Opal_InstanceCallbacks));

	ptr->heap_size = desc->heap_size;
	ptr->max_heap_allocations = desc->max_heap_allocations;
	ptr->max_heaps = desc->max_heaps;
	ptr->flags = desc->flags;

	VkDebugUtilsMessengerCreateInfoEXT messenger_info = {0};
	messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	messenger_info.pfnUserCallback = vulkan_debugMessageCallback;
	messenger_info.pUserData = ptr;

	if (desc->flags & OPAL_INSTANCE_CREATION_FLAGS_USE_DEBUG_LAYERS)
	{
		layers[num_layers++] = VK_LAYER_KHRONOS_VALIDATION_NAME;

		if (desc->callbacks.debug_message.function != NULL)
		{
			extensions[num_extensions++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
			info.pNext = &messenger_info;
		}
	}

	info.enabledLayerCount = num_layers;
	info.ppEnabledLayerNames = layers;

	info.enabledExtensionCount = num_extensions;
	info.ppEnabledExtensionNames = extensions;

	VkResult result = vkCreateInstance(&info, NULL, &vulkan_instance);
	if (result != VK_SUCCESS)
	{
		free(ptr);
		return OPAL_VULKAN_ERROR;
	}

	volkLoadInstance(vulkan_instance);

	VkDebugUtilsMessengerEXT vulkan_debug_messenger = VK_NULL_HANDLE;
	if (desc->flags & OPAL_INSTANCE_CREATION_FLAGS_USE_DEBUG_LAYERS && desc->callbacks.debug_message.function != NULL)
	{
		result = vkCreateDebugUtilsMessengerEXT(vulkan_instance, &messenger_info, NULL, &vulkan_debug_messenger);
		if (result != VK_SUCCESS)
		{
			vkDestroyInstance(vulkan_instance, NULL);
			free(ptr);

			return OPAL_VULKAN_ERROR;
		}
	}

	// vtable
	ptr->vtbl = &instance_vtbl;

	// data
	ptr->instance = vulkan_instance;
	ptr->debug_messenger = vulkan_debug_messenger;

	// pools
	opal_poolInitialize(&ptr->surfaces, sizeof(Vulkan_Surface), 32);

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;
}
