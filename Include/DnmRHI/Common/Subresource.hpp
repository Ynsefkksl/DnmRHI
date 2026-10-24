#pragma once

#include <DnmRHI/Common/AspectFlags.hpp>
#include <DnmRHI/Image/SubresourceType.hpp>

namespace DnmRHI {
    enum class Subresource : u64 {};

    struct alignas(8) BufferSubresource {
        constexpr BufferSubresource() = default;
        constexpr BufferSubresource(Subresource subresource) noexcept {
            const u64 raw = static_cast<u64>(subresource);

            offset = static_cast<u32>((raw >> 0)  & 0xFFFFFFFF);
            size = static_cast<u32>((raw >> 32) & 0xFFFFFFFF);
        }

        u32 offset;
        u32 size;

        constexpr auto &SetOffset(u32 offset) noexcept { this->offset = offset; return *this; }
        constexpr auto &SetSize(u32 size) noexcept { this->size = size; return *this; }

        constexpr operator Subresource() const noexcept { 
            return static_cast<Subresource>(
                (u64{offset} << 0) |
                (u64{size} << 32)); 
        }
    };

    struct alignas(8) ImageSubresource {
        constexpr ImageSubresource() = default;
        constexpr ImageSubresource(Subresource subresource) noexcept {
            const u64 raw = static_cast<u64>(subresource);

            base_layer = static_cast<decltype(base_layer)>((raw >> 0)  & 0xFF);
            layer_count = static_cast<decltype(layer_count)>((raw >> 16)  & 0xFF);
            mipmap_count = static_cast<decltype(mipmap_count)>((raw >> 32) & 0xFF);
            base_mipmap = static_cast<decltype(base_mipmap)>((raw >> 40) & 0xFF);
            aspect_flags = static_cast<decltype(aspect_flags)>((raw >> 48) & 0xFF);
            type = static_cast<decltype(type)>((raw >> 56) & 0xFF);
        }

        u16 base_layer{0};
        u16 layer_count{1};
        u8 base_mipmap{0};
        u8 mipmap_count{1};
        AspectFlags aspect_flags{AspectFlagBit::eColor};
        ImageSubresourceType type{ImageSubresourceType::e2D};

        constexpr auto &SetBaseMipmap(u16 base_mipmap) noexcept { this->base_layer = base_mipmap; return *this; }
        constexpr auto &SetBaseLayer(u16 base_layer) noexcept { this->layer_count = base_layer; return *this; }
        constexpr auto &SetMipmapCount(u8 mipmap_count) noexcept { this->mipmap_count = mipmap_count; return *this; }
        constexpr auto &SetLayerCount(u8 layer_count) noexcept { this->base_mipmap = layer_count; return *this; }
        constexpr auto &SetAspectFlags(AspectFlags aspect_flags) noexcept { this->aspect_flags = aspect_flags; return *this; }
        constexpr auto &SetImageType(ImageSubresourceType image_type) noexcept { this->type = image_type; return *this; }

        constexpr operator Subresource() const noexcept {
            return static_cast<Subresource>(
                (u64{base_layer} << 0) |
                (u64{layer_count} << 16) |
                (u64{mipmap_count} << 32) |
                (u64{base_mipmap} << 40) |
                (u64{AspectFlags::Type(aspect_flags)} << 48) |
                (u64(type) << 56)); 
        }
    };
}
