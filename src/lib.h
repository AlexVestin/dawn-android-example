#ifndef __DAWN_ANDROID_LIB_H
#define __DAWN_ANDROID_LIB_H

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/VulkanBackend.h"
#include "dawn/dawn_proc.h"

#include <android/native_activity.h>
#include <memory>

namespace DawnAndroid {
    void Init(
        ANativeWindow* window, 
        uint32_t width, 
        uint32_t height,
        int32_t ahbFormat);
    void Frame();
    wgpu::TextureFormat GetVkFormatFromAHBFormat(uint32_t ahbFormat);
};

#endif // define __DAWN_ANDROID_LIB_H