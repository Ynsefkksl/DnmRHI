#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Platform/Vulkan/Swapchain.hpp>
#include <DnmRHI/Image/Image.hpp>
#include <ranges>
#include <unordered_map>

namespace DnmRHI::Vk {
    namespace Utility {
        struct ImageSubresourceHasher {
            size_t operator()(DnmRHI::ImageSubresource subresource) const noexcept {
                return std::hash<uint64_t>{}(reinterpret_cast<const uint64_t&>(subresource));
            }
        };
        
        struct ImageSubresourceEq {
            bool operator()(DnmRHI::ImageSubresource lhs, DnmRHI::ImageSubresource rhs) const noexcept {
                return memcmp(&lhs, &rhs, sizeof(DnmRHI::ImageSubresource)) == 0;
            }
        };
    }

    class Image final : public DnmRHI::Image {
    public:
        explicit Image(Vk::Device &device, const ImageDesc &desc) noexcept;
        // for swapchain
        explicit Image(Vk::Device &device, vk::Image image, ImageFlags flags, u32 width, u32 height, vk::Format format, Vk::Swapchain *swapchain, u8 swapchain_image_index, std::string_view name) noexcept;
        virtual ~Image() noexcept override { 
            GetVkDevice.GetDevice().waitIdle();
            for (const auto view : m_image_views | std::views::values)
                GetVkDevice.GetDevice().destroy(view);
            if (c_SwapchainImageView) GetVkDevice.GetDevice().destroy(c_SwapchainImageView);
            if (m_allocation) vmaDestroyImage(GetVkDevice.GetAllocator(), m_image, m_allocation); 
        }

        [[nodiscard]] auto GetImage() const noexcept { return m_image; }
        [[nodiscard]] auto *GetAllocation() const noexcept { return m_allocation; }

        [[nodiscard]] auto GetImageView(DnmRHI::ImageSubresource subresource) noexcept {
            if (c_IsSwapchainImage) return c_SwapchainImageView;
            auto [it, is_emplaced] = m_image_views.try_emplace(subresource, VK_NULL_HANDLE);
            if (is_emplaced)
                it->second = CreateImageView(subresource);
            return it->second;
        }

        // for VK_EXT_descriptor_heap
        [[nodiscard]] vk::ImageViewCreateInfo GetCreateImageViewInfo(DnmRHI::ImageSubresource subresource) const noexcept;
        
        [[nodiscard]] auto GetSwapchain() const noexcept { return m_swapchain; }
        [[nodiscard]] auto GetSwapchainImageIndex() const noexcept { return c_SwapchainImageIndex; }

        const vk::ImageLayout c_DefaultLayout;
        const vk::ImageLayout c_ReadOnlyAttachmentImageLayout;
        const vk::ImageLayout c_AttachmentImageLayout;
        const vk::ImageLayout c_CopySrcImageLayout;
        const vk::ImageLayout c_CopyDstImageLayout;
        const vk::ImageLayout c_VariableRateShadingLayout;

        const b8 c_IsSwapchainImage;
        const vk::ImageView c_SwapchainImageView;
    private:
        [[nodiscard]] vk::ImageView CreateImageView(DnmRHI::ImageSubresource subresource) const noexcept { return GetVkDevice.GetDevice().createImageView(GetCreateImageViewInfo(subresource)); }

        vk::Image m_image{};
        VmaAllocation m_allocation{};

        std::unordered_map<ImageSubresource, vk::ImageView, Utility::ImageSubresourceHasher, Utility::ImageSubresourceEq> m_image_views{};
        Swapchain *const m_swapchain{};
        const u8 c_SwapchainImageIndex{};
    };
}