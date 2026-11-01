#include <DnmRHI/Swapchain/Swapchain.hpp>

namespace DnmRHI {
    Result Swapchain::ReCreate(const SwapchainSettings &settings) noexcept {
        //TODO: check desc
        return IReCreate(settings);
    }
    
    ResultOr<u32> Swapchain::AcquireNextImage() noexcept {
        //TODO: check somethings
        return IAcquireNextImage();
    }
}