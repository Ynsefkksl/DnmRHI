#pragma once

#include <DnmRHI/Buffer/Desc.hpp>

namespace DnmRHI {
    class Buffer : public Resource {
    protected:
        explicit Buffer(Device &device, const BufferDesc &desc) noexcept : m_desc(desc), Resource(device, ObjectType::eBuffer) {}
    public:
        template <typename T = void>
        [[nodiscard]] T *GetMappedPtr() const noexcept { return reinterpret_cast<T *>(m_mapped_ptr); }
        [[nodiscard]] u64 GetDeviceAddress() const noexcept { return m_buffer_device_address; }

        [[nodiscard]] const auto &GetDesc() const noexcept { return m_desc; }
    protected:
        const BufferDesc m_desc;

        void *m_mapped_ptr;
        u64 m_buffer_device_address;
    };
}