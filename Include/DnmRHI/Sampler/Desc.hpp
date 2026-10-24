#pragma once

#include <DnmRHI/Sampler/AddressMode.hpp>
#include <DnmRHI/Common/FilterType.hpp>
#include <DnmRHI/Common/CompareOp.hpp>

namespace DnmRHI {
    struct SamplerDesc {
        f32 anisotropy = 1.f;
        f32 max_lod = 1000.f;
        f32 min_lod = 0.f;
        f32 mip_lod_bias = 0.f;
        SamplerAddressMode address_mode_u;
        SamplerAddressMode address_mode_v;
        SamplerAddressMode address_mode_w;
        FilterType min_filter_type;
        FilterType mag_filter_type;
        FilterType mip_filter_type;
        CompareOp compare_op;

        constexpr auto &SetAnisotropy(f32 anisotropy) noexcept { this->anisotropy = anisotropy; return *this; }
        constexpr auto &SetMaxLod(f32 max_lod) noexcept { this->max_lod = max_lod; return *this; }
        constexpr auto &SetMinLod(f32 min_lod) noexcept { this->min_lod = min_lod; return *this; }
        constexpr auto &SetMipLodBias(f32 mip_lod_bias) noexcept { this->mip_lod_bias = mip_lod_bias; return *this; }
        constexpr auto &SetAddressModeU(SamplerAddressMode address_mode_u) noexcept { this->address_mode_u = address_mode_u; return *this; }
        constexpr auto &SetAddressModeV(SamplerAddressMode address_mode_v) noexcept { this->address_mode_v = address_mode_v; return *this; }
        constexpr auto &SetAddressModeW(SamplerAddressMode address_mode_w) noexcept { this->address_mode_w = address_mode_w; return *this; }
        constexpr auto &SetMinFilterType(FilterType min_filter_type) noexcept { this->min_filter_type = min_filter_type; return *this; }
        constexpr auto &SetMagFilterType(FilterType mag_filter_type) noexcept { this->mag_filter_type = mag_filter_type; return *this; }
        constexpr auto &SetMipFilterType(FilterType mip_filter_type) noexcept { this->mip_filter_type = mip_filter_type; return *this; }
        constexpr auto &SetCompareOp(CompareOp compare_op) noexcept { this->compare_op = compare_op; return *this; }
    };
}
