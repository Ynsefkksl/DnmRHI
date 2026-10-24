#pragma once

#include <DnmRHI/Swapchain/Desc.hpp>
#include <span>
#include <vector>

namespace DnmRHI {
    class Swapchain : public Object {
    protected:
        explicit Swapchain(Device &device, const SwapchainDesc &desc) noexcept : m_desc(desc), Object(device, ObjectType::eSwapchain) {}
    public:
        void ReCreate(const SwapchainSettings &settings) noexcept;
        [[nodiscard]] u32 AcquireNextImage() noexcept;

        [[nodiscard]] auto GetImages() const noexcept { return std::span(m_images); }
        [[nodiscard]] const auto &GetDesc() const noexcept { return m_desc; }
    protected:
        virtual void IReCreate(const SwapchainSettings &settings) noexcept = 0;
        [[nodiscard]] virtual u32 IAcquireNextImage() noexcept = 0;

        SwapchainDesc m_desc;
        std::vector<Image *> m_images;
    };
}