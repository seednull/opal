#pragma once

#include <opal.h>

#define OPAL_UNUSED(x) (x)

Opal_Result vulkan_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
Opal_Result directx12_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
Opal_Result metal_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
Opal_Result webgpu_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);
Opal_Result null_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance);

uint32_t opal_evaluateDevice(const Opal_DeviceInfo *info, Opal_DeviceHint hint);