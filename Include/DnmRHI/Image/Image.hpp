#pragma once

#include <DnmRHI/Image/Desc.hpp>

namespace DnmRHI {
    class Image : public Resource {
    protected:
        explicit Image(Device &device, const ImageDesc &desc) noexcept : m_desc(desc), Resource(device, ObjectType::eImage) {}
    public:
        [[nodiscard]] const auto &GetDesc() const noexcept { return m_desc; }
    protected:
        ImageDesc m_desc;
    };
}