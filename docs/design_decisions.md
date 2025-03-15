# Design Decisions

## Common

### Driver version

Always uint64_t. Vulkan fills lower 32 bits, DX 12 fills full 64 bits. Other APIs have hardcoded value for lower 32 bits. It's up to the user how to transform this value to string.

One possible solution might be based on provided vendor ID and device ID.

### No support for shader records and parameters both on Vulkan Ray Tracing and DXR

Intentionally disabled due to different ways to put data in them (LocalRootSignature with 8 byte offset on DXR, Uniform buffer rules for Vulkan).

### Artifical queue limits on DirectX 12 and Metal

Both API allow creating an arbitary number of queues, however, in order to keep the API consistent there are artifical limits for each device engine type.

- Main (aka direct or graphics) engine will have 16 queues.
- Compute (aka async compute) engine will have 8 queues.
- Copy (aka async transfer) engine will have 2 queues.

### No support for acceleration structure serialization / deserialization

Intentionally removed api as there is no practical use for the end user.

### Command buffer memory management

Opal command buffers will rely on command allocators for memory storage. User is free to create multiple command buffers for a single command allocator but only one command buffer is allowed to be in the recording state (via opalBeginCommandBuffer). Also, opalResetCommandAllocator requires all command buffers to be either in the initial state or executed. This design is similar to DirectX 12.

## Vulkan

### No support for queue priorities

Intentionally set to 1.0f for all available queues on physical device.

### No support for VK_IMAGE_TILING_LINEAR

All images will use VK_IMAGE_TILING_OPTIMAL, because there is no practical use case for linear tiled images.

### No memory type hint for images

Images will prefer memory with VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT flag. For transfer & readback it's better to create staging buffer.

### Fixed command pools & command buffers flags

VkCommandPool is always created with VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT so that command buffer can be reset and resubmitted.

opalBeginCommandBuffer will call vkBeginCommandBuffer with VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT so command buffer must be either reset or recreated after submit. This is done intentionally to match other API designs.

## WebGPU

### Enumerate devices & create device by index on WebGPU

Intentionally disabled as WebGPU expects to work with a single device. Use opalCreateDefaultDevice with proper hint.

### No map / unmap support for uniform / storage / indirect / vertex / index buffer usages

WebGPU only allow mapping buffers with WGPUBufferUsage_CopySrc or WGPUBufferUsage_CopyDst usage. This is similar to previous GAPI designs where staging buffer must be created in order to upload data to the GPU memory.

opalMapBuffer / webgpu_deviceUnmapBuffer will return OPAL_BUFFER_NONMAPPABLE for any combinations of uniform / storage / indirect / vertex / index buffer usages.

## DirectX 12

### Automatic register space assignment

To match Vulkan binding model, every descriptor in descriptor set will share the same register space. The value will match the index of corresponding descriptor set layout in pipeline layout. Opal will not allow setting register spaces manually.

### Different semantics for binding indices

DirectX 12 backend will use binding indices as-is. That allows setting the same binding index for different binding types which is not possible in other APIs.

For example, buffer & texture could have the same binding index, because there're assigned to different register types in HLSL.

### Hardcoded input element semantic name

Intentionally introduced semantic name "LOCATION[n]" for vertex shader input attributes to match other shading language designs. This semantic only required for vertex shader inputs. Other inter-stage semantics could be used as usual.

## Metal
