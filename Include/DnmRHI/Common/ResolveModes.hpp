#pragma once

#include <DnmRHI/Utility/Flags.hpp>

namespace DnmRHI {
    enum class ResolveModeFlagBit : u8 {
        eNone = 0,
        eZero = 1 << 0,
        eAverage = 1 << 1,
        eMin = 1 << 2,
        eMax = 1 << 3,
    };
    using ResolveModeFlags = Flags<ResolveModeFlagBit>;
}

template <> struct DnmRHI::FlagTraits<DnmRHI::ResolveModeFlagBit> {
    static constexpr b8 is_bitmask = true;
};