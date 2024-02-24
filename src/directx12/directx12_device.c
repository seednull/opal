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
