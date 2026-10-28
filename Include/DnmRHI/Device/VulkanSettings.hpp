#pragma once

#include <DnmRHI/Types.hpp>

namespace DnmRHI {
    struct VulkanSettings {
        b8 no_descriptor_heap:1 = false;
        b8 no_unified_image_layouts:1 = false;
        b8 no_maintenance9:1 = false;
        b8 no_pageable_device_local_memory:1 = false;
        // TODO: maybe it is unnecessary
        b8 no_swapchain_maintenance1:1 = false;
        // TODO: implament this
        b8 no_input_output_16:1 = false;

        // TODO: implament this
        b8 generate_spirv_code_via_glsl:1 = true;

        constexpr auto &SetNoDescriptorHeap(b8 no_descriptor_heap) noexcept { this->no_descriptor_heap = no_descriptor_heap; return *this; }
        constexpr auto &SetNoUnifiedImageLayouts(b8 no_unified_image_layouts) noexcept { this->no_unified_image_layouts = no_unified_image_layouts; return *this; }
        constexpr auto &SetNoMaintenance9(b8 no_maintenance9) noexcept { this->no_maintenance9 = no_maintenance9; return *this; }
        constexpr auto &SetNoPageableDeviceLocalMemory(b8 no_pageable_device_local_memory) noexcept { this->no_pageable_device_local_memory = no_pageable_device_local_memory; return *this; }
        constexpr auto &SetNoSwapchainMaintenance1(b8 no_swapchain_maintenance1) noexcept { this->no_swapchain_maintenance1 = no_swapchain_maintenance1; return *this; }
        constexpr auto &SetNoInputOutput16(b8 no_input_output_16) noexcept { this->no_input_output_16 = no_input_output_16; return *this; }
        constexpr auto &SetGenerateSpirvCodeViaGlsl(b8 generate_spirv_code_via_glsl) noexcept { this->generate_spirv_code_via_glsl = generate_spirv_code_via_glsl; return *this; }
    };
}