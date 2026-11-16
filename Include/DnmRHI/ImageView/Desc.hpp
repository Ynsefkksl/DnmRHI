#pragma once

#include <DnmRHI/Common/Subresource.hpp>
#include <DnmRHI/ImageView/ImageViewType.hpp>

namespace DnmRHI {
    struct ImageViewDesc {
        Image *image;
        ImageSubresource subresource;
        ImageViewType type;
    };
}