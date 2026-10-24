#pragma once

#include <DnmRHI/Types.hpp>

namespace DnmRHI {
    enum class ImageSubresourceType : u8 {
        e2D,
        e2DArray,
        e3D,
        eCube,
        eCubeArray,
    };
}
