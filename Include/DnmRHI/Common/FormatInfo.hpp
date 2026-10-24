#pragma once

#include <DnmRHI/Common/Format.hpp>
#include <utility>

namespace DnmRHI {
    enum class FormatDataType : u8 {
        eUndefined,
        eFloat,
        eUfloat,
        eInt,
        eUint,
        eSnorm,
        eUnorm,
        eSrgb
    };

    struct FormatInfo {
        u8 channel_count;
        u8 byte_size;
        FormatDataType data_type;
        b8 is_compressed;
        u8 block_size;
        u8 bit_per_pixel;
    };

    constexpr auto FormatChannelCount(Format format) noexcept {
        switch (format) {
            case Format::eUndefined: return 0u;

            case Format::eBC4_Snorm:
            case Format::eBC4_Unorm:
            case Format::eR16_Uint:
            case Format::eR32_Uint:
            case Format::eR8_Unorm:
            case Format::eR16_Unorm:
            case Format::eD16_Unorm: 
            case Format::eR16_Snorm: 
            case Format::eR8_Snorm: 
            case Format::eR8_Uint:
            case Format::eS8_Uint:
            case Format::eR16_Float: 
            case Format::eD32_Float: 
            case Format::eR32_Float: 
            case Format::eR8_Int:
            case Format::eR16_Int:
            case Format::eR32_Int: return 1u;

            case Format::eBC5_Snorm:
            case Format::eBC5_Unorm:
            case Format::eR8G8_Uint:
            case Format::eR16G16_Uint:
            case Format::eD16_Unorm_S8_Uint:
            case Format::eD24_Unorm_S8_Uint:
            case Format::eR32G32_Uint:
            case Format::eD32_Float_S8_Uint:
            case Format::eR8G8_Unorm:
            case Format::eR16G16_Unorm:
            case Format::eR8G8_Snorm:
            case Format::eR16G16_Snorm:
            case Format::eR8G8_Int:
            case Format::eR16G16_Int:
            case Format::eR32G32_Int:
            case Format::eR16G16_Float:
            case Format::eR32G32_Float: return 2u;
            
            case Format::eBC6H_Float:
            case Format::eBC6H_Ufloat:
            case Format::eBC1_Rgb_Unorm:
            case Format::eBC1_Rgb_Srgb:
            case Format::eR11G11B10_Ufloat: return 3u;

            case Format::eBC1_Rgba_Unorm:
            case Format::eBC1_Rgba_Srgb:
            case Format::eBC2_Unorm:
            case Format::eBC2_Srgb:
            case Format::eBC3_Unorm:
            case Format::eBC3_Srgb:
            case Format::eBC7_Unorm:
            case Format::eBC7_Srgb:
            case Format::eR8G8B8A8_Uint:
            case Format::eR10G10B10A2_Uint:
            case Format::eR16G16B16A16_Uint:
            case Format::eR32G32B32A32_Uint: 
            case Format::eR8G8B8A8_Unorm:
            case Format::eR10G10B10A2_Unorm:
            case Format::eB8G8R8A8_Unorm:
            case Format::eB8G8R8A8_Srgb:
            case Format::eR16G16B16A16_Unorm: 
            case Format::eR8G8B8A8_Snorm:
            case Format::eR16G16B16A16_Snorm:
            case Format::eR8G8B8A8_Srgb: 
            case Format::eR8G8B8A8_Int:
            case Format::eR16G16B16A16_Int:
            case Format::eR16G16B16A16_Float:
            case Format::eR32G32B32A32_Float:
            case Format::eR32G32B32A32_Int: return 4u;
        }
        std::unreachable();
    }

    constexpr auto FormatDataType(Format format) noexcept {
        switch (format) {
            case Format::eUndefined: return FormatDataType::eUndefined;
            
            case Format::eR16_Float: 
            case Format::eR16G16_Float:
            case Format::eD32_Float: 
            case Format::eR32_Float:
            case Format::eR16G16B16A16_Float:
            case Format::eR32G32_Float: 
            case Format::eR32G32B32A32_Float:
            case Format::eBC6H_Float: return FormatDataType::eFloat;

            case Format::eBC6H_Ufloat:
            case Format::eR11G11B10_Ufloat: return FormatDataType::eUfloat;
            
            case Format::eR8_Int:
            case Format::eR8G8_Int:
            case Format::eR16_Int:
            case Format::eR8G8B8A8_Int:
            case Format::eR16G16_Int:
            case Format::eR32_Int:
            case Format::eR16G16B16A16_Int:
            case Format::eR32G32_Int:
            case Format::eR32G32B32A32_Int: return FormatDataType::eInt;
            
            case Format::eR8_Uint:
            case Format::eS8_Uint:
            case Format::eR8G8_Uint:
            case Format::eR16_Uint:
            case Format::eR8G8B8A8_Uint:
            case Format::eR16G16_Uint:
            case Format::eR32_Uint:
            case Format::eR10G10B10A2_Uint:
            case Format::eD16_Unorm_S8_Uint:
            case Format::eD24_Unorm_S8_Uint:
            case Format::eR16G16B16A16_Uint:
            case Format::eR32G32_Uint:
            case Format::eD32_Float_S8_Uint:
            case Format::eR32G32B32A32_Uint: return FormatDataType::eUint;

            case Format::eR8G8_Snorm:
            case Format::eR16_Snorm: 
            case Format::eR8G8B8A8_Snorm:
            case Format::eR16G16_Snorm:
            case Format::eR16G16B16A16_Snorm:
            case Format::eR8_Snorm: 
            case Format::eBC4_Snorm:
            case Format::eBC5_Snorm: return FormatDataType::eSnorm;

            case Format::eR8_Unorm:
            case Format::eR8G8_Unorm:
            case Format::eR16_Unorm:
            case Format::eD16_Unorm: 
            case Format::eR8G8B8A8_Unorm:
            case Format::eR16G16_Unorm:
            case Format::eB8G8R8A8_Unorm:
            case Format::eR10G10B10A2_Unorm:
            case Format::eR16G16B16A16_Unorm: 
            case Format::eBC1_Rgba_Unorm:
            case Format::eBC1_Rgb_Unorm:
            case Format::eBC2_Unorm:
            case Format::eBC3_Unorm:
            case Format::eBC4_Unorm:
            case Format::eBC5_Unorm:
            case Format::eBC7_Unorm: return FormatDataType::eUnorm;

            case Format::eR8G8B8A8_Srgb:
            case Format::eB8G8R8A8_Srgb: 
            case Format::eBC1_Rgba_Srgb:
            case Format::eBC1_Rgb_Srgb:
            case Format::eBC2_Srgb:
            case Format::eBC3_Srgb:
            case Format::eBC7_Srgb: return FormatDataType::eSrgb;
        }
        std::unreachable();
    }

    constexpr auto FormatSize(Format format) noexcept {
        switch (format) {
            case Format::eUndefined:
            case Format::eBC1_Rgba_Unorm:
            case Format::eBC1_Rgba_Srgb:
            case Format::eBC1_Rgb_Unorm:
            case Format::eBC1_Rgb_Srgb:
            case Format::eBC2_Unorm:
            case Format::eBC2_Srgb:
            case Format::eBC3_Unorm:
            case Format::eBC3_Srgb:
            case Format::eBC4_Unorm:
            case Format::eBC4_Snorm:
            case Format::eBC5_Unorm:
            case Format::eBC5_Snorm:
            case Format::eBC6H_Float:
            case Format::eBC6H_Ufloat:
            case Format::eBC7_Unorm:
            case Format::eBC7_Srgb: return 0u;

            case Format::eR8_Unorm:
            case Format::eR8_Snorm:
            case Format::eR8_Int:
            case Format::eR8_Uint:
            case Format::eS8_Uint: return 1u;

            case Format::eR8G8_Unorm:
            case Format::eR8G8_Snorm:
            case Format::eR8G8_Int:
            case Format::eR8G8_Uint:
            case Format::eR16_Uint:
            case Format::eR16_Int:
            case Format::eR16_Float:
            case Format::eR16_Unorm:
            case Format::eR16_Snorm: 
            case Format::eD16_Unorm: return 2u;

            case Format::eR8G8B8A8_Unorm:
            case Format::eR8G8B8A8_Snorm:
            case Format::eB8G8R8A8_Srgb:
            case Format::eB8G8R8A8_Unorm:
            case Format::eR8G8B8A8_Srgb:
            case Format::eR8G8B8A8_Int:
            case Format::eR8G8B8A8_Uint:
            case Format::eR16G16_Unorm:
            case Format::eR16G16_Snorm:
            case Format::eR16G16_Uint:
            case Format::eR16G16_Int:
            case Format::eR16G16_Float:
            case Format::eR32_Int:
            case Format::eR32_Uint:
            case Format::eR32_Float:
            case Format::eR11G11B10_Ufloat:
            case Format::eR10G10B10A2_Unorm:
            case Format::eR10G10B10A2_Uint:
            case Format::eD32_Float: 
            case Format::eD16_Unorm_S8_Uint:
            case Format::eD24_Unorm_S8_Uint: return 4u;

            case Format::eR16G16B16A16_Unorm:
            case Format::eR16G16B16A16_Snorm:
            case Format::eR16G16B16A16_Int:
            case Format::eR16G16B16A16_Uint:
            case Format::eR16G16B16A16_Float:
            case Format::eR32G32_Int:
            case Format::eR32G32_Uint:
            case Format::eR32G32_Float: 
            case Format::eD32_Float_S8_Uint: return 8u;

            case Format::eR32G32B32A32_Int:
            case Format::eR32G32B32A32_Uint:
            case Format::eR32G32B32A32_Float: return 16u;
        }
        std::unreachable();
    }

    constexpr auto FormatBlockSize(Format format) noexcept {
        switch (format) {
            case Format::eUndefined:
            case Format::eR8_Unorm:
            case Format::eR8_Snorm:
            case Format::eR8_Int:
            case Format::eR8_Uint:
            case Format::eS8_Uint:
            case Format::eR8G8_Unorm:
            case Format::eR8G8_Snorm:
            case Format::eR8G8_Int:
            case Format::eR8G8_Uint:
            case Format::eR16_Uint:
            case Format::eR16_Int:
            case Format::eR16_Float:
            case Format::eR16_Unorm:
            case Format::eR16_Snorm: 
            case Format::eD16_Unorm:
            case Format::eB8G8R8A8_Srgb:
            case Format::eB8G8R8A8_Unorm:
            case Format::eR8G8B8A8_Unorm:
            case Format::eR8G8B8A8_Snorm:
            case Format::eR8G8B8A8_Srgb:
            case Format::eR8G8B8A8_Int:
            case Format::eR8G8B8A8_Uint:
            case Format::eR16G16_Unorm:
            case Format::eR16G16_Snorm:
            case Format::eR16G16_Uint:
            case Format::eR16G16_Int:
            case Format::eR16G16_Float:
            case Format::eR32_Int:
            case Format::eR32_Uint:
            case Format::eR32_Float:
            case Format::eR11G11B10_Ufloat:
            case Format::eR10G10B10A2_Unorm:
            case Format::eR10G10B10A2_Uint:
            case Format::eD32_Float: 
            case Format::eD16_Unorm_S8_Uint:
            case Format::eD24_Unorm_S8_Uint:
            case Format::eR16G16B16A16_Unorm:
            case Format::eR16G16B16A16_Snorm:
            case Format::eR16G16B16A16_Int:
            case Format::eR16G16B16A16_Uint:
            case Format::eR16G16B16A16_Float:
            case Format::eR32G32_Int:
            case Format::eR32G32_Uint:
            case Format::eR32G32_Float: 
            case Format::eD32_Float_S8_Uint:
            case Format::eR32G32B32A32_Int:
            case Format::eR32G32B32A32_Uint:
            case Format::eR32G32B32A32_Float: return 0u;

            case Format::eBC1_Rgba_Unorm:
            case Format::eBC1_Rgba_Srgb:
            case Format::eBC1_Rgb_Unorm:
            case Format::eBC1_Rgb_Srgb:
            case Format::eBC4_Unorm:
            case Format::eBC4_Snorm: return 8u;

            case Format::eBC2_Unorm:
            case Format::eBC2_Srgb:
            case Format::eBC3_Unorm:
            case Format::eBC3_Srgb:
            case Format::eBC5_Unorm:
            case Format::eBC5_Snorm:
            case Format::eBC6H_Float:
            case Format::eBC6H_Ufloat:
            case Format::eBC7_Unorm:
            case Format::eBC7_Srgb: return 16u;
        }
        std::unreachable();
    }

    // for compressed formats
    constexpr auto FormatBitPerPixel(Format format) noexcept {
        switch (format) {
            case Format::eUndefined:
            case Format::eR8_Unorm:
            case Format::eR8_Snorm:
            case Format::eR8_Int:
            case Format::eR8_Uint:
            case Format::eS8_Uint:
            case Format::eR8G8_Unorm:
            case Format::eR8G8_Snorm:
            case Format::eR8G8_Int:
            case Format::eR8G8_Uint:
            case Format::eR16_Uint:
            case Format::eR16_Int:
            case Format::eR16_Float:
            case Format::eR16_Unorm:
            case Format::eR16_Snorm: 
            case Format::eD16_Unorm:
            case Format::eR8G8B8A8_Unorm:
            case Format::eR8G8B8A8_Snorm:
            case Format::eR8G8B8A8_Srgb:
            case Format::eR8G8B8A8_Int:
            case Format::eR8G8B8A8_Uint:
            case Format::eR16G16_Unorm:
            case Format::eR16G16_Snorm:
            case Format::eR16G16_Uint:
            case Format::eR16G16_Int:
            case Format::eR16G16_Float:
            case Format::eR32_Int:
            case Format::eR32_Uint:
            case Format::eR32_Float:
            case Format::eR11G11B10_Ufloat:
            case Format::eR10G10B10A2_Unorm:
            case Format::eR10G10B10A2_Uint:
            case Format::eD32_Float: 
            case Format::eD16_Unorm_S8_Uint:
            case Format::eD24_Unorm_S8_Uint:
            case Format::eR16G16B16A16_Unorm:
            case Format::eR16G16B16A16_Snorm:
            case Format::eR16G16B16A16_Int:
            case Format::eR16G16B16A16_Uint:
            case Format::eR16G16B16A16_Float:
            case Format::eR32G32_Int:
            case Format::eR32G32_Uint:
            case Format::eR32G32_Float: 
            case Format::eB8G8R8A8_Srgb:
            case Format::eB8G8R8A8_Unorm:
            case Format::eD32_Float_S8_Uint:
            case Format::eR32G32B32A32_Int:
            case Format::eR32G32B32A32_Uint:
            case Format::eR32G32B32A32_Float: return 0u;

            case Format::eBC1_Rgba_Unorm:
            case Format::eBC1_Rgba_Srgb:
            case Format::eBC1_Rgb_Unorm:
            case Format::eBC1_Rgb_Srgb:
            case Format::eBC4_Unorm:
            case Format::eBC4_Snorm: return 4u;

            case Format::eBC2_Unorm:
            case Format::eBC2_Srgb:
            case Format::eBC3_Unorm:
            case Format::eBC3_Srgb:
            case Format::eBC5_Unorm:
            case Format::eBC5_Snorm:
            case Format::eBC6H_Float:
            case Format::eBC6H_Ufloat:
            case Format::eBC7_Unorm:
            case Format::eBC7_Srgb: return 8u;
        }
        std::unreachable();
    }

    constexpr auto IsDepthFormat(Format format) noexcept {
        return format == Format::eD32_Float || format == Format::eD16_Unorm;
    }

    constexpr auto IsStencilFormat(Format format) noexcept {
        return format == Format::eS8_Uint;
    }

    constexpr auto IsDepthStencilFormat(Format format) noexcept {
        return format == Format::eD32_Float_S8_Uint || format == Format::eD24_Unorm_S8_Uint || format == Format::eD16_Unorm_S8_Uint;
    }

    constexpr auto IsColorFormat(Format format) noexcept {
        return !(IsDepthFormat(format) || IsStencilFormat(format) || IsDepthStencilFormat(format) || format == Format::eUndefined);
    }

    constexpr auto IsCompressedFormat(Format format) noexcept {
        return FormatBitPerPixel(format) > 0;
    }
}