#include "webgpu_internal.h"

/*
 */
Opal_Result webgpu_deviceGetInfo(Device *this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	WebGpu_Device *ptr = (WebGpu_Device *)this;
	return webgpu_fillDeviceInfo(ptr->adapter, info);
}

/*
 */
Opal_Result webgpu_deviceDestroy(Device *this)
{
	assert(this);

	WebGpu_Device *ptr = (WebGpu_Device *)this;
	wgpuAdapterRelease(ptr->adapter);
	wgpuDeviceRelease(ptr->device);

	return OPAL_SUCCESS;
}

/*
 */
Opal_Result webgpu_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result webgpu_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_deviceUnmapBuffer(Device *this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result webgpu_deviceDestroyBuffer(Device *this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_deviceDestroyTexture(Device *this, Opal_Texture texture)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result webgpu_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view)
{
	return OPAL_NOT_SUPPORTED;
}
