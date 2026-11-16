#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Platform/Vulkan/Swapchain.hpp>
#include <DnmRHI/Image/Image.hpp>

namespace DnmRHI::Vk {
    class Image final : public DnmRHI::Image {
    public:
        explicit Image(Vk::Device &device, const ImageDesc &desc) noexcept;
        // for swapchain
        explicit Image(Vk::Device &device, vk::Image image, ImageFlags flags, u32 width, u32 height, vk::Format format, Vk::Swapchain *swapchain, u8 swapchain_image_index, std::string_view name) noexcept;
        virtual ~Image() noexcept override { 
            if (m_allocation) vmaDestroyImage(GetVkDevice.GetAllocator(), m_image, m_allocation); 
        }

        [[nodiscard]] auto GetImage() const noexcept { return m_image; }
        [[nodiscard]] auto *GetAllocation() const noexcept { return m_allocation; }

        [[nodiscard]] auto GetSwapchain() const noexcept { return m_swapchain; }
        [[nodiscard]] auto GetSwapchainImageIndex() const noexcept { return c_SwapchainImageIndex; }

        const vk::ImageLayout c_DefaultLayout;
        const vk::ImageLayout c_ReadOnlyAttachmentImageLayout;
        const vk::ImageLayout c_AttachmentImageLayout;
        const vk::ImageLayout c_CopySrcImageLayout;
        const vk::ImageLayout c_CopyDstImageLayout;
        const vk::ImageLayout c_VariableRateShadingLayout;

        const b8 c_IsSwapchainImage;
    private:
        vk::Image m_image{};
        VmaAllocation m_allocation{};

        Swapchain *const m_swapchain{};
        const u8 c_SwapchainImageIndex{};
    };
}