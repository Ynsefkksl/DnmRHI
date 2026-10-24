#include <DnmRHI/Swapchain/Swapchain.hpp>

namespace DnmRHI {
    void Swapchain::ReCreate(const SwapchainSettings &settings) noexcept {
        //TODO: check desc
        IReCreate(settings);
    }
    
    u32 Swapchain::AcquireNextImage() noexcept {
        //TODO: check somethings
        return IAcquireNextImage();
    }
}