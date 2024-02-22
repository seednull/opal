#include "directx12_internal.h"

/*
 */
Opal_Result directx12_fillDeviceInfo(IDXGIAdapter1 *adapter, Opal_DeviceInfo *info)
{
	assert(adapter);
	assert(info);

	// TODO: detect feature level by creating d3ddevice with various feature levels

	// TODO: detect driver version by looking into windows registry using LUID from adapter desc

	DXGI_ADAPTER_DESC1 desc;
	HRESULT hr = IDXGIAdapter1_GetDesc1(adapter, &desc);
	if (!SUCCEEDED(hr))
		return OPAL_DIRECX12_ERROR;

	memset(info, 0, sizeof(Opal_DeviceInfo));

	wcstombs(info->name, &desc.Description[0], 256);
	info->driver_version = 0;
	info->vendor_id = desc.VendorId;
	info->device_id = desc.DeviceId;
	info->tessellation_shader = 1;
	info->geometry_shader = 1;
	info->compute_shader = 1;
	info->texture_compression_bc = 1;
	info->max_buffer_alignment = 0xFFFF;

	return OPAL_SUCCESS;
}
