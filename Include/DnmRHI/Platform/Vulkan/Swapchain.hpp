#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Swapchain/Swapchain.hpp>

namespace DnmRHI::Vk {
    // TODO: VK_KHR_present_mode_fifo_latest_ready
    class Swapchain final : public DnmRHI::Swapchain {
    public:
        explicit Swapchain(Vk::Device &device, const SwapchainDesc &desc) noexcept;
        ~Swapchain() noexcept;

        virtual void IReCreate(const SwapchainSettings &settings) noexcept override;
        [[nodiscard]] virtual u32 IAcquireNextImage() noexcept override;

        [[nodiscard]] auto GetSurface() const noexcept { return m_surface; }
        [[nodiscard]] auto GetSwapchain() const noexcept { return m_swapchain; }
        [[nodiscard]] auto GetFormat() const noexcept { return m_format; }
        [[nodiscard]] auto GetImageCount() const noexcept { return m_image_count; }

        [[nodiscard]] auto GetSwapchainImages() const noexcept { return std::span(m_swapchain_images); }
        [[nodiscard]] auto GetImageIndex() const noexcept { return m_image_index; }
        [[nodiscard]] auto GetVkImageIndex() const noexcept { return m_vk_image_index; }
        [[nodiscard]] auto GetCurrentFrameIndex() const noexcept { return m_frame_index; }
        [[nodiscard]] auto GetFrameIndex() noexcept { 
            const auto frame_index = m_frame_index; ;
            return m_frame_index = (frame_index + 1) % c_Frames_In_Flight_Count, frame_index; }
        [[nodiscard]] auto GetImageAcquireSemaphores() const noexcept { return std::span(m_image_acquire_semaphores); }
        [[nodiscard]] auto GetImageAvailableSemaphore() const noexcept { return std::span(m_image_available_semaphores); }
    private:
        vk::SurfaceKHR m_surface{};
        vk::SwapchainKHR m_swapchain{};
        vk::Format m_format{};
        u32 m_image_count{};
        u32 m_image_index{};
        u32 m_frame_index{};
        u32 m_vk_image_index{};
        u32 m_width{}, m_height{};

        std::vector<vk::Semaphore> m_image_available_semaphores;
        std::vector<vk::Semaphore> m_image_acquire_semaphores;

        std::vector<vk::Image> m_swapchain_images;
    };
}