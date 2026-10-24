#pragma once

#include <DnmRHI/Types.hpp>

namespace DnmRHI {
    enum class SamplerAddressMode : u8 {
        eRepeat,
        eMirroredRepeat,
        eClampToEdge,
        eClampToBorder,
    };
}