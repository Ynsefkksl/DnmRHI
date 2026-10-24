#include <DnmRHI/Platform/Vulkan/Buffer.hpp>

namespace DnmRHI::Vk {
    InternalBuffer::InternalBuffer(Device &device, const vk::BufferCreateInfo &info, DnmRHI::MemoryType memory_type, DnmRHI::HostMemoryAccess host_memory_usage, u16 align)
        : m_device(device) {
        Create(info, memory_type, host_memory_usage, align);
    }

    InternalBuffer::InternalBuffer(Device &device, const BufferDesc &desc)
        : m_device(device) {
        vk::BufferUsageFlags usage_flags{};

        if (desc.flags.Has(BufferFlagBit::eConditional))
            usage_flags |= vk::BufferUsageFlagBits::eConditionalRenderingEXT;
        if (desc.flags.Has(BufferFlagBit::eIndex))
            usage_flags |= vk::BufferUsageFlagBits::eIndexBuffer;
        if (desc.flags.Has(BufferFlagBit::eIndirect))
            usage_flags |= vk::BufferUsageFlagBits::eIndirectBuffer;
        if (desc.flags.Has(BufferFlagBit::eStorage))
            usage_flags |= {vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress};
        if (desc.flags.Has(BufferFlagBit::eUniform))
            usage_flags |= vk::BufferUsageFlagBits::eUniformBuffer;
        if (desc.flags.Has(BufferFlagBit::eCopySrc))
            usage_flags |= vk::BufferUsageFlagBits::eTransferSrc;
        if (desc.flags.Has(BufferFlagBit::eCopyDst))
            usage_flags |= vk::BufferUsageFlagBits::eTransferDst;

        const auto create_info
            = vk::BufferCreateInfo{}
                .setSize(desc.size)
                .setUsage(usage_flags)
                .setSharingMode(vk::SharingMode::eConcurrent)
                ;

        Create(create_info, desc.memory_type, desc.host_memory_usage, desc.align);
        if (!desc.name.empty()) SET_DEBUG_NAME_VK_OBJECT(device.GetDevice(), m_buffer, desc.name.data());
    }

    void InternalBuffer::Create(const vk::BufferCreateInfo &info, DnmRHI::MemoryType memory_type, DnmRHI::HostMemoryAccess host_memory_usage, u16 align) {
        VmaAllocationCreateInfo alloc_create_info{};
        if (host_memory_usage != HostMemoryAccess::eNone) {
            alloc_create_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            alloc_create_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
            alloc_create_info.flags |= (host_memory_usage == HostMemoryAccess::eWrite) ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        }

        if (memory_type == MemoryType::eDevice) {
            alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        }
        else if (memory_type == MemoryType::eHost) {
            alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        }
        else {
            alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;
        }

        VmaAllocationInfo alloc_info;
        if (align == 0)
            vmaCreateBuffer(m_device.GetAllocator(), (VkBufferCreateInfo *)&info, &alloc_create_info, (VkBuffer *)&m_buffer, &m_alloc, &alloc_info);
        else
            vmaCreateBufferWithAlignment(m_device.GetAllocator(), (VkBufferCreateInfo *)&info, &alloc_create_info, align, (VkBuffer *)&m_buffer, &m_alloc, &alloc_info);

        if (host_memory_usage != HostMemoryAccess::eNone)
            m_mapped_ptr = alloc_info.pMappedData;

        m_device_address = info.usage & vk::BufferUsageFlagBits::eShaderDeviceAddress ? m_device.GetDevice().getBufferAddress(m_buffer) : u64{};
    }
};