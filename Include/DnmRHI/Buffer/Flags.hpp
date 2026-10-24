#pragma once

#include <DnmRHI/Types.hpp>
#include <DnmRHI/Utility/Flags.hpp>

namespace DnmRHI {
    enum class BufferFlagBit : u8 {
        eUniform = 1 << 0,
        eStorage = 1 << 1,
        eIndex = 1 << 2,
        eIndirect = 1 << 3,
        eConditional = 1 << 4,
        eCopySrc = 1 << 5,
        eCopyDst = 1 << 6,
    };
    using BufferFlags = Flags<BufferFlagBit>;
}

template <> struct DnmRHI::FlagTraits<DnmRHI::BufferFlagBit> {
    static constexpr b8 is_bitmask = true;
};