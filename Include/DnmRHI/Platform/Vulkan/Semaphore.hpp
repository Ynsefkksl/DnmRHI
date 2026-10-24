#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Semaphore/Semaphore.hpp>

namespace DnmRHI::Vk {
    class Semaphore final : public DnmRHI::Semaphore {
    public:
        explicit Semaphore(Vk::Device &device, const SemaphoreDesc &desc) noexcept;
        virtual ~Semaphore() override { 
            GetVkDevice.GetDevice().waitIdle();
            GetVkDevice.GetDevice().destroy(m_semaphore);
        }

        [[nodiscard]] virtual u64 IGetValue() noexcept override;
        virtual void ISignal(u64 value) noexcept override;

        [[nodiscard]] auto GetSemaphore() const noexcept { return m_semaphore; }
    private:
        vk::Semaphore m_semaphore;
    };
};