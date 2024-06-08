#include "vulkan_internal.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*
 */
const char *vulkan_platformGetSurfaceExtension()
{
	return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

Opal_Result vulkan_platformCreateSurface(VkInstance instance, void *handle, VkSurfaceKHR *surface)
{
	VkWin32SurfaceCreateInfoKHR surface_info = {0};
	surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_info.hinstance = GetModuleHandle(NULL);
	surface_info.hwnd = (HWND)handle;

	VkResult result = vkCreateWin32SurfaceKHR(instance, &surface_info, NULL, surface);
	if (result != VK_SUCCESS)
		return OPAL_VULKAN_ERROR;

	return OPAL_SUCCESS;
}
