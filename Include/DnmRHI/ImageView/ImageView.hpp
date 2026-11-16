#pragma once

#include <DnmRHI/ImageView/Desc.hpp>

namespace DnmRHI {
    class ImageView : public Object {
    protected:
        explicit ImageView(Device &device, const ImageViewDesc &desc) noexcept : m_desc(desc), Object(device, ObjectType::eImageView) {}
    public:
        [[nodiscard]] const auto &GetDesc() const noexcept { return m_desc; }
    protected:
        ImageViewDesc m_desc;
    };
}