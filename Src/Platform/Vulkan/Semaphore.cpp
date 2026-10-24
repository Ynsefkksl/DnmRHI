#include <DnmRHI/Platform/Vulkan/Semaphore.hpp>

namespace DnmRHI::Vk {
    Semaphore::Semaphore(Vk::Device &device, const SemaphoreDesc &desc) noexcept : DnmRHI::Semaphore(device, desc) {
        const auto type_info 
            = vk::SemaphoreTypeCreateInfo{}
                .setInitialValue(desc.initial_value)
                .setSemaphoreType(vk::SemaphoreType::eTimeline)
                ;

        const auto create_info
            = vk::SemaphoreCreateInfo{}
                .setPNext(&type_info)
                ;

        m_semaphore = GetVkDevice.GetDevice().createSemaphore(create_info);
        if (!desc.name.empty()) SET_DEBUG_NAME_VK_OBJECT(GetVkDevice.GetDevice(), m_semaphore, desc.name.data());
    }

    u64 Semaphore::IGetValue() noexcept {
        return GetVkDevice.GetDevice().getSemaphoreCounterValue(m_semaphore);
    }

    void Semaphore::ISignal(u64 value) noexcept {
        GetVkDevice.GetDevice().signalSemaphore({m_semaphore, value});
    }
};