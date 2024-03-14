#include "vulkan_internal.h"

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

static void vulkan_volkLoadInstance(VkInstance instance)
{
	static int once = 0;

	if (!once)
	{
		volkLoadInstance(instance);
		once = 1;
	}
}

static Vulkan_Device *opal_createDevice(VkPhysicalDevice physical_device, VkDevice device)
{
	assert(physical_device != VK_NULL_HANDLE);
	assert(device != VK_NULL_HANDLE);

	Vulkan_Device *ptr = (Vulkan_Device *)malloc(sizeof(Vulkan_Device));
	assert(ptr);

	// vtable
	ptr->vtbl.getInfo = vulkan_deviceGetInfo;
	ptr->vtbl.destroy = vulkan_deviceDestroy;
	ptr->vtbl.createBuffer = vulkan_deviceCreateBuffer;
	ptr->vtbl.createTexture = vulkan_deviceCreateTexture;
	ptr->vtbl.createTextureView = vulkan_deviceCreateTextureView;
	ptr->vtbl.destroyBuffer = vulkan_deviceDestroyBuffer;
	ptr->vtbl.destroyTexture = vulkan_deviceDestroyTexture;
	ptr->vtbl.destroyTextureView = vulkan_deviceDestroyTextureView;

	// data
	ptr->physical_device = physical_device;
	ptr->device = device;

	return ptr;
}

/*
 */
Opal_Result vulkan_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	vulkan_volkInitialize();

	VkInstance vulkan_instance = VK_NULL_HANDLE;

	VkApplicationInfo app_info = {0};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.apiVersion = VK_VERSION_1_1;
	app_info.applicationVersion = desc->application_version;
	app_info.pApplicationName = desc->application_name;
	app_info.engineVersion = desc->engine_version;
	app_info.pEngineName = desc->engine_name;

	VkInstanceCreateInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = &app_info;

	VkResult result = vkCreateInstance(&info, NULL, &vulkan_instance);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	vulkan_volkLoadInstance(vulkan_instance);

	Vulkan_Instance *ptr = (Vulkan_Instance *)malloc(sizeof(Vulkan_Instance));
	assert(ptr);

	// vtable
	ptr->vtbl.enumerateDevices = vulkan_instanceEnumerateDevices;
	ptr->vtbl.createDefaultDevice = vulkan_instanceCreateDefaultDevice;
	ptr->vtbl.createDevice = vulkan_instanceCreateDevice;
	ptr->vtbl.destroy = vulkan_instanceDestroy;

	// data
	ptr->instance = vulkan_instance;

	*instance = (Opal_Instance)ptr;
	return OPAL_SUCCESS;
}

Opal_Result vulkan_instanceEnumerateDevices(Instance *this, uint32_t *device_count, Opal_DeviceInfo *infos)
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

			opal_result = vulkan_fillDeviceInfo(device, &infos[i]);
			if (opal_result != OPAL_SUCCESS)
				break;
		}

		free(devices);
	}

	return opal_result;
}

Opal_Result vulkan_instanceCreateDefaultDevice(Instance *this, Opal_DeviceHint hint, Opal_Device *device)
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
		VkPhysicalDevice device = devices[i];
		Opal_DeviceInfo info = {0};

		opal_result = vulkan_fillDeviceInfo(device, &info);
		if (opal_result != OPAL_SUCCESS)
			break;

		uint32_t score = opal_evaluateDevice(&info, hint);
		if (score > best_score)
		{
			best_score = score;
			vulkan_physical_device = device;
		}
	}

	free(devices);
	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	if (vulkan_physical_device == VK_NULL_HANDLE)
		return OPAL_VULKAN_ERROR;

	VkDevice vulkan_device = VK_NULL_HANDLE;
	opal_result = vulkan_createDevice(vulkan_physical_device, &vulkan_device);

	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	*device = (Opal_Device)opal_createDevice(vulkan_physical_device, vulkan_device);
	return OPAL_SUCCESS;
}

Opal_Result vulkan_instanceCreateDevice(Instance *this, uint32_t index, Opal_Device *device)
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
	Opal_Result opal_result = vulkan_createDevice(vulkan_physical_device, &vulkan_device);

	if (opal_result != OPAL_SUCCESS)
		return opal_result;

	*device = (Opal_Device)opal_createDevice(vulkan_physical_device, vulkan_device);
	return OPAL_SUCCESS;
}

Opal_Result vulkan_instanceDestroy(Instance *this)
{
	assert(this);

	Vulkan_Instance *ptr = (Vulkan_Instance *)this;
	vkDestroyInstance(ptr->instance, NULL);

	return OPAL_SUCCESS;
}
