# Design Decisions

## Driver version

Always uint64_t. Vulkan fills lower 32 bits, DX 12 fills full 64 bits. Other APIs have hardcoded value for lower 32 bits. It's up to the user how to transform this value to string.

One possible solution might be based on provided vendor ID and device ID.

## Enumerate devices & create device by index on WebGPU

Intentionally disabled as WebGPU expects to work with a single device. Use opalCreateDefaultDevice with proper hint.

## Queue priorities on Vulkan

Intentionally set to 1.0f for all available queues on physical device.

## Ignore VK_IMAGE_TILING_LINEAR on Vulkan

All images will use VK_IMAGE_TILING_OPTIMAL, because there is no practical use case for linear tiled images.

## No memory type hint for images

On Vulkan, images will prefer memory with VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT flag. For transfer & readback it's better to create staging buffer.
