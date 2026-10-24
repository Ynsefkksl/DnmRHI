#pragma once

#include <DnmRHI/Types.hpp>

namespace DnmRHI {
    struct VulkanSettings {
        b8 no_stencil_export: 1 = false;
        b8 no_descriptor_heap:1 = false;
        b8 no_unified_image_layouts:1 = false;
        b8 no_maintenance9:1 = false;
        b8 no_pageable_device_local_memory:1 = false;
        b8 no_swapchain_maintenance1:1 = false;
        b8 generate_spirv_code_via_glsl:1 = true;
    };
}