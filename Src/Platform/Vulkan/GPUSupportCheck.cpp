#include <DnmRHI/Platform/Vulkan/Device.hpp>

namespace DnmRHI::Vk {
    // TODO:
    static b8 CheckGPUSupportAndGetFeatures(vk::PhysicalDevice device, DeviceInfo &device_info, 
        b8 just_check, Vk::DeviceFeatures vk_features, std::vector<const char *> &extensions, void *&features);
}