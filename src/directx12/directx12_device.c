#include "directx12_internal.h"

/*
 */
Opal_Result directx12_deviceGetInfo(Device *this, Opal_DeviceInfo *info)
{
	assert(this);
	assert(info);

	DirectX12_Device *ptr = (DirectX12_Device *)this;
	return directx12_fillDeviceInfoWithDevice(ptr->adapter, ptr->device, info);
}

/*
 */
Opal_Result directx12_deviceDestroy(Device *this)
{
	assert(this);

	DirectX12_Device *ptr = (DirectX12_Device *)this;
	IDXGIAdapter1_Release(ptr->adapter);
	ID3D12Device_Release(ptr->device);

	return OPAL_SUCCESS;
}

/*
 */
Opal_Result directx12_deviceCreateBuffer(Device *this, const Opal_BufferDesc *desc, Opal_Buffer *buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result directx12_deviceCreateTexture(Device *this, const Opal_TextureDesc *desc, Opal_Texture *texture)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result directx12_deviceCreateTextureView(Device *this, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result directx12_deviceMapBuffer(Device *this, Opal_Buffer buffer, void **ptr)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result directx12_deviceUnmapBuffer(Device *this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

/*
 */
Opal_Result directx12_deviceDestroyBuffer(Device *this, Opal_Buffer buffer)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result directx12_deviceDestroyTexture(Device *this, Opal_Texture texture)
{
	return OPAL_NOT_SUPPORTED;
}

Opal_Result directx12_deviceDestroyTextureView(Device *this, Opal_TextureView texture_view)
{
	return OPAL_NOT_SUPPORTED;
}
