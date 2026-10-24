#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Buffer/Buffer.hpp>

namespace DnmRHI::Vk {
    class InternalBuffer {
    public:
        explicit InternalBuffer(Vk::Device &device, const vk::BufferCreateInfo &info, DnmRHI::MemoryType type, DnmRHI::HostMemoryAccess host_memory_usage, u16 align = 0);
        explicit InternalBuffer(Vk::Device &device, const BufferDesc &desc);

        ~InternalBuffer() noexcept {
            m_device.GetDevice().waitIdle();
            if (!m_imported_mem) {
                vmaDestroyBuffer(m_device.GetAllocator(), m_buffer, m_alloc);
                return;
            }
            m_device.GetDevice().destroy(m_buffer);
            m_device.GetDevice().free(m_imported_mem);
        }

        [[nodiscard]] vk::Buffer GetBuffer() const noexcept { return m_buffer; }
        [[nodiscard]] VmaAllocation GetAllocation() const noexcept { return m_alloc; }
        [[nodiscard]] vk::DeviceMemory GetImportMemory() const noexcept { return m_imported_mem; }

        [[nodiscard]] u64 GetDeviceAddress() const noexcept { return m_device_address; }
        [[nodiscard]] void *GetMappedPtr() const noexcept { return m_mapped_ptr; }
        template <typename T> [[nodiscard]] T *GetMappedPtr() const noexcept { return reinterpret_cast<T *>(m_mapped_ptr); }
    private:
        void Create(const vk::BufferCreateInfo &info, DnmRHI::MemoryType type, DnmRHI::HostMemoryAccess host_memory_usage, u16 align);

        vk::Buffer m_buffer{};
        VmaAllocation m_alloc{};
        u64 m_device_address;
        vk::DeviceMemory m_imported_mem{};
        void *m_mapped_ptr{};
        Vk::Device &m_device;
    };

    class Buffer final : public DnmRHI::Buffer {
    public:
        explicit Buffer(Vk::Device &device, const BufferDesc &desc) : DnmRHI::Buffer(device, desc), m_internal_buffer{static_cast<Device &>(device), desc} {
            m_buffer_device_address = m_internal_buffer.GetDeviceAddress();
            m_mapped_ptr = m_internal_buffer.GetMappedPtr(); 
        }
        virtual ~Buffer() override = default;

        [[nodiscard]] auto &GetInternalBuffer() const noexcept { return m_internal_buffer; }

        [[nodiscard]] vk::Buffer GetBuffer() const noexcept { return m_internal_buffer.GetBuffer(); }
        [[nodiscard]] VmaAllocation GetAllocation() const noexcept { return m_internal_buffer.GetAllocation(); }
        [[nodiscard]] vk::DeviceMemory GetImportMemory() const noexcept { return m_internal_buffer.GetImportMemory(); }
    private:
        const InternalBuffer m_internal_buffer;
    };
}