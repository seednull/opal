#include <opal.h>
#include <cassert>
#include <iostream>

void testBuffers(Opal_Device device)
{
	Opal_Buffer buffers[2] = {OPAL_NULL_HANDLE, OPAL_NULL_HANDLE};
	void *ptrs[2] = {nullptr, nullptr};

	Opal_BufferDesc desc =
	{
		OPAL_BUFFER_USAGE_TRANSFER_SRC,
		16,
		OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
		OPAL_ALLOCATION_HINT_AUTO,
	};

	Opal_Result result = opalCreateBuffer(device, &desc, &buffers[0]);
	assert(result == OPAL_SUCCESS);

	result = opalCreateBuffer(device, &desc, &buffers[1]);
	assert(result == OPAL_SUCCESS);

	result = opalMapBuffer(device, buffers[0], &ptrs[0]);
	assert(result == OPAL_SUCCESS);

	result = opalMapBuffer(device, buffers[1], &ptrs[1]);
	assert(result == OPAL_SUCCESS);

	uint32_t data_0 = 42;
	uint32_t data_1 = 0xDEADBEEF;
	memcpy(ptrs[0], &data_0, sizeof(uint32_t));
	memcpy(ptrs[1], &data_1, sizeof(uint32_t));

	result = opalDestroyBuffer(device, buffers[0]);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyBuffer(device, buffers[1]);
	assert(result == OPAL_SUCCESS);
}

int main()
{
	Opal_Instance instance = OPAL_NULL_HANDLE;

	Opal_InstanceDesc instance_desc =
	{
		"02_buffers",
		"Opal",
		0,
		0,
		OPAL_DEFAULT_HEAP_SIZE,
		OPAL_DEFAULT_HEAP_ALLOCATIONS,
		OPAL_DEFAULT_HEAPS,
	};

	Opal_Result result = opalCreateInstance(OPAL_API_AUTO, &instance_desc, &instance);
	assert(result == OPAL_SUCCESS);

	Opal_Device device = OPAL_NULL_HANDLE;
	result = opalCreateDefaultDevice(instance, OPAL_DEVICE_HINT_DEFAULT, &device);
	assert(result == OPAL_SUCCESS);

	testBuffers(device);

	result = opalDestroyDevice(device);
	assert(result == OPAL_SUCCESS);

	result = opalDestroyInstance(instance);
	assert(result == OPAL_SUCCESS);

	return 0;
}
