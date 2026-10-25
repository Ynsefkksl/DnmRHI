#include <DnmRHI/DnmRHI.hpp>
#include <print>

int main() {
    auto vk_devices = DnmRHI::QuerySupportedVulkanDevices();
    if (vk_devices.empty()) std::println("there is no vulkan supported device");
    for (const auto &device : vk_devices) {
        std::println("name device name: {}", device.properties.name);
    }
}