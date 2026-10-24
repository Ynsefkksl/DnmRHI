#pragma once

#include <DnmRHI/Semaphore/Desc.hpp>

namespace DnmRHI {
    class Semaphore : public Object {
    protected:
        explicit Semaphore(Device &device, const SemaphoreDesc &desc) noexcept : m_desc(desc), Object(device, ObjectType::eSemaphore) {}
    public:
        [[nodiscard]] u64 GetValue() noexcept { return IGetValue(); }
        void Signal(u64 value) noexcept { ISignal(value); }

        [[nodiscard]] const auto &GetDesc() const noexcept { return m_desc; }
    protected:
        [[nodiscard]] virtual u64 IGetValue() noexcept = 0;
        virtual void ISignal(u64 value) noexcept = 0;

        const SemaphoreDesc m_desc;
    };
}