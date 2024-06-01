#include "metal_internal.h"

/*
 */
Opal_Result metal_deviceGetInfo(Device *this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	Metal_Device *ptr = (Metal_Device *)this;
	return metal_fillDeviceInfo(ptr->device, info);
}

Opal_Result metal_deviceGetQueue(Device *this, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue)
{
	assert(this);
	assert(queue);
	assert(engine_type < OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX);

	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result metal_deviceDestroy(Device *this)
{
	assert(this);

	Metal_Device *ptr = (Metal_Device *)this;
	// [ptr->device release]; // FIXME: do we need to manually release it here?

	return OPAL_SUCCESS;
}

/*
 */
Opal_Result metal_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result metal_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result metal_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result metal_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result metal_deviceUnmapBuffer(Device *this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result metal_deviceDestroyBuffer(Device *this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result metal_deviceDestroyTexture(Device *this, Opal_Texture texture)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result metal_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view)
{
	return OPAL_NOT_SUPPORTED;
}
