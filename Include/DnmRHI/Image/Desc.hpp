#pragma once

#include <DnmRHI/Image/SubresourceType.hpp>
#include <DnmRHI/Image/Type.hpp>
#include <DnmRHI/Image/SampleCount.hpp>
#include <DnmRHI/Image/Flags.hpp>
#include <DnmRHI/Common/Format.hpp>
#include <string>

namespace DnmRHI {
    struct ImageDesc {
        std::string name;
        Extent3D extent{1,1,1};
        ImageType type;
        Format format;
        ImageFlags flags;
        u8 mipmap_count = 1;
        SampleCount sample_count = SampleCount::e1;
        
        constexpr auto &SetName(std::string_view name) noexcept { this->name = name; return *this; }
        constexpr auto &SetExtent(Extent3D extent) noexcept { this->extent = extent; return *this; }
        constexpr auto &SetType(ImageType type) noexcept { this->type = type; return *this; }
        constexpr auto &SetFormat(Format format) noexcept { this->format = format; return *this; }
        constexpr auto &SetFlags(ImageFlags flags) noexcept { this->flags = flags; return *this; }
        constexpr auto &SetMipmapCount(u8 mipmap_count) noexcept { this->mipmap_count = mipmap_count; return *this; }        
        constexpr auto &SetSampleCount(SampleCount sample_count) noexcept { this->sample_count = sample_count; return *this; }
    };
}