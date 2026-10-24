#pragma once

#include <DnmRHI/Types.hpp>
#include <DnmRHI/Utility/Flags.hpp>

namespace DnmRHI {
    enum class AspectFlagBit : u8 {
        eColor = 1 << 0,
        eDepth = 1 << 1,
        eStencil = 1 << 2,
    };
    using AspectFlags = Flags<AspectFlagBit>;
}

template <> struct DnmRHI::FlagTraits<DnmRHI::AspectFlagBit> {
    static constexpr b8 is_bitmask = true;
};