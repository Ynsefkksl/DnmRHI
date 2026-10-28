#include <DnmRHI/DnmRHI.hpp>
#include <DnmRHI/Device/VulkanSettings.hpp>
#include <print>

int main() {
    // currently i am testing something, i write triangle example when i complate device init
    auto vk_devices = DnmRHI::QuerySupportedVulkanDevices();
    if (vk_devices.empty()) std::println("there is no vulkan supported device");
    for (const auto &device_info : vk_devices) {
        std::println("name device: {}", device_info.properties.name);
        auto *device = DnmRHI::CreateVulkanDevice(device_info, 
            DnmRHI::DeviceSettings{}
                .SetDescValidation(true)
                .SetValidationLayer(true),
            DnmRHI::VulkanSettings{}
        );
        device->Init();
        delete device;
    }
}
