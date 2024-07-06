# Design Decisions

## Driver version

Always uint64_t. Vulkan fills lower 32 bits, DX 12 fills full 64 bits. Other APIs have hardcoded value for lower 32 bits. It's up to the user how to transform this value to string.

One possible solution might be based on provided vendor ID and device ID.

## Enumerate devices & create device by index on WebGPU

Intentionally disabled as WebGPU expects to work with a single device. Use opalCreateDefaultDevice with proper hint.

## No queue priorities on Vulkan

Intentionally set to 1.0f for all available queues on physical device.

## Ignore VK_IMAGE_TILING_LINEAR on Vulkan

All images will use VK_IMAGE_TILING_OPTIMAL, because there is no practical use case for linear tiled images.

## No memory type hint for images

On Vulkan, images will prefer memory with VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT flag. For transfer & readback it's better to create staging buffer.

## Artifical queue limits on DirectX 12 and Metal

Both API allow creating an arbitary number of queues, however, in order to keep the API consistent there are artifical limits for each device engine type.

- Main (aka direct or graphics) engine will have 16 queues.
- Compute (aka async compute) engine will have 8 queues.
- Copy (aka async transfer) engine will have 2 queues.

## Vulkan command pools & command buffers flags

VkCommandPool is always created with VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT so that command buffer can be reset and resubmitted.

opalBeginCommandBuffer will call vkBeginCommandBuffer with VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT so command buffer must be either reset or recreated after submit. This is done intentionally to match other API designs.

## No support for shader records and parameters both on Vulkan KHR Ray Tracing and DXR

Intentionally disabled due to different ways to put data in them (LocalRootSignature with 8 byte offset on DXR, Uniform buffer rules for Vulkan).
