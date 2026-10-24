#pragma once

#include <DnmRHI/Types.hpp>
#include <DnmRHI/Utility/Flags.hpp>

namespace DnmRHI {
    enum class ImageFlagBit : u16 {
        eSampled = 1 << 0,
        eStorage = 1 << 1,
        eCopySrc = 1 << 2,
        eCopyDst = 1 << 3,
        eAttachment = 1 << 4,
        eVariableRateShadingAttachment = 1 << 5,
        eGeneralQueue = 1 << 6,
        eComputeQueue = 1 << 7,
        eCopyQueue = 1 << 8,
    };
    using ImageFlags = Flags<ImageFlagBit>;
}

template <> struct DnmRHI::FlagTraits<DnmRHI::ImageFlagBit> {
    static constexpr b8 is_bitmask = true;
};