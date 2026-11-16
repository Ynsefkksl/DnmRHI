#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/ImageView/ImageView.hpp>

namespace DnmRHI::Vk {
    class ImageView final : public DnmRHI::ImageView {
    public:
        explicit ImageView(Vk::Device &device, const DnmRHI::ImageViewDesc &desc) noexcept;

        virtual ~ImageView() noexcept override {
            if (m_image_view) GetVkDevice.GetDevice().destroy(m_image_view);
        }

        [[nodiscard]] auto GetImageView() const noexcept { return m_image_view; }
        [[nodiscard]] const auto &GetCreateInfo() const noexcept { return m_create_info; }
    private:
        vk::ImageViewCreateInfo m_create_info;
        vk::ImageView m_image_view;
    };
}