#pragma once

#include <DnmRHI/Types.hpp>
#include <DnmRHI/Utility/Flags.hpp>

namespace DnmRHI {
    enum class ShaderStageFlagBit : u16 {
        eVertex = 1 << 0,
        eFragment = 1 << 1,
        eCompute = 1 << 2,
        eMesh = 1 << 3,
        eTask = 1 << 4,
        eAnyHit = 1 << 5,
        eCallable = 1 << 6,
        eClosestHit = 1 << 7,
        eIntersection = 1 << 8,
        eMiss = 1 << 9,
        eRaygen = 1 << 10,
    };
    using ShaderStageFlags = Flags<ShaderStageFlagBit>;
}

template <> struct DnmRHI::FlagTraits<DnmRHI::ShaderStageFlagBit> {
    static constexpr b8 is_bitmask = true;
};