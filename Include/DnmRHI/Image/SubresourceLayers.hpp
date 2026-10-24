#pragma once

#include <DnmRHI/Common/AspectFlags.hpp>

namespace DnmRHI {
    struct ImageSubresourceLayers {
        u16 base_layer{0};
        u16 layer_count{1};
        u8 mipmap_level{0};
        AspectFlags aspect_flags;

        constexpr auto &SetBaseLayer(u16 layer) noexcept { this->base_layer = layer; return *this; }
        constexpr auto &SetLayerCount(u16 count) noexcept { this->layer_count = count; return *this; }
        constexpr auto &SetMipmapLevel(u8 mip) noexcept { this->mipmap_level = mip; return *this; }
        constexpr auto &SetAspectFlags(AspectFlags flags) noexcept { this->aspect_flags = flags; return *this; }
    };
}
