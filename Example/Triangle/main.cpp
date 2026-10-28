#include <DnmRHI/DnmRHI.hpp>
#include <DnmRHI/Device/VulkanSettings.hpp>
#include <print>

int main() {
    // currently i am testing something, i write triangle example when i complate device init
    auto vk_devices = DnmRHI::QuerySupportedVulkanDevices();
    if (vk_devices.empty()) std::println("there is no vulkan supported device");
    for (const auto &device : vk_devices) {
        std::println("name device: {}", device.properties.name);
        auto *device_ = DnmRHI::CreateVulkanDevice(device, 
            DnmRHI::DeviceSettings{}
                .SetDescValidation(true)
                .SetValidationLayer(true),
            DnmRHI::VulkanSettings{}
        );
        device_->Init();
        delete device_;
    }
}
