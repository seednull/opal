#include "opal_internal.h"

/*
 */
Opal_Result opalCreateInstance(Opal_Api api, const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	switch (api)
	{
		case OPAL_API_VULKAN: return vulkan_createInstance(desc, instance);
		case OPAL_API_DIRECTX12: return directx12_createInstance(desc, instance);
		case OPAL_API_METAL: return metal_createInstance(desc, instance);
		case OPAL_API_WEBGPU: return webgpu_createInstance(desc, instance);
		case OPAL_API_NULL: return null_createInstance(desc, instance);

		default: return OPAL_NOT_SUPPORTED;
	}
}

Opal_Result opalDestroyInstance(Opal_Instance instance)
{
	// FIXME: change to handle + generation and do proper check
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Instance *ptr = (Instance *)(instance);
	Opal_Result result = ptr->destroy(ptr);

	free(ptr);

	return result;
}

/*
 */
Opal_Result opalEnumerateDevices(Opal_Instance instance, uint32_t *device_count, Opal_DeviceInfo *infos)
{
	// FIXME: change to handle + generation and do proper check
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Instance *ptr = (Instance *)(instance);
	return ptr->enumerateDevices(ptr, device_count, infos);
}

/*
 */
Opal_Result opalCreateDevice(Opal_Instance instance, uint32_t index, Opal_Device *device)
{
	// FIXME: change to handle + generation and do proper check
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Instance *ptr = (Instance *)(instance);
	return ptr->createDevice(ptr, index, device);
}

Opal_Result opalCreateDefaultDevice(Opal_Instance instance, Opal_DeviceHint hint, Opal_Device *device)
{
	// FIXME: change to handle + generation and do proper check
	if (instance == OPAL_NULL_HANDLE)
		return OPAL_INVALID_INSTANCE;

	Instance *ptr = (Instance *)(instance);
	return ptr->createDefaultDevice(ptr, hint, device);
}

Opal_Result opalDestroyDevice(Opal_Device device)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	Opal_Result result = ptr->destroy(ptr);

	free(ptr);

	return result;
}

/*
 */
Opal_Result opalGetDeviceInfo(Opal_Device device, Opal_DeviceInfo *info)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->getInfo(ptr, info);
}

/*
 */
Opal_Result opalCreateBuffer(Opal_Device device, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->createBuffer(ptr, desc, buffer);
}

Opal_Result opalCreateTexture(Opal_Device device, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->createTexture(ptr, desc, texture);
}

Opal_Result opalCreateTextureView(Opal_Device device, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->createTextureView(ptr, desc, texture_view);
}

/*
 */
Opal_Result opalDestroyBuffer(Opal_Device device, Opal_Buffer buffer)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->destroyBuffer(ptr, buffer);
}

Opal_Result opalDestroyTexture(Opal_Device device, Opal_Texture texture)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->destroyTexture(ptr, texture);
}

Opal_Result opalDestroyTextureView(Opal_Device device, Opal_TextureView texture_view)
{
	// FIXME: change to handle + generation and do proper check
	if (device == OPAL_NULL_HANDLE)
		return OPAL_INVALID_DEVICE;

	Device *ptr = (Device *)(device);
	return ptr->destroyTextureView(ptr, texture_view);
}
