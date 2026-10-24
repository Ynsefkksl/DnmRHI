#pragma once

#include <DnmRHI/Types.hpp>
#include <DnmRHI/Utility/Flags.hpp>

namespace DnmRHI {
    enum class ColorComponentFlagBit : u8 {
        eR = 1 << 0,
        eG = 1 << 1,
        eB = 1 << 2,
        eA = 1 << 3,
    };
    using ColorComponentFlags = Flags<ColorComponentFlagBit>;
}

template <> struct DnmRHI::FlagTraits<DnmRHI::ColorComponentFlagBit> {
    static constexpr b8 is_bitmask = true;
};