#include <DnmRHI/Platform/Vulkan/DescriptorManager.hpp>
#include <DnmRHI/Platform/Vulkan/ToVk.hpp>
#include <DnmRHI/Platform/Vulkan/Buffer.hpp>
#include <DnmRHI/Platform/Vulkan/Image.hpp>

namespace DnmRHI::Vk {
    namespace Utility {
        static vk::DescriptorSetLayout CreateResourceDescriptorSetLayout(vk::Device device, u32 count) noexcept {
            constexpr auto descriptor_type_list = std::array{
                vk::DescriptorType::eSampledImage,
                vk::DescriptorType::eStorageImage,
                vk::DescriptorType::eUniformBuffer,
            };

            const auto mutable_descriptor_type_list 
                = vk::MutableDescriptorTypeListEXT{}
                    .setDescriptorTypes(descriptor_type_list)
                    ;

            const auto mutable_descriptor_create_info
                = vk::MutableDescriptorTypeCreateInfoEXT{}
                    .setMutableDescriptorTypeLists(mutable_descriptor_type_list)
                    ;

            const auto bindless_binding_flags 
                = vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending 
                | vk::DescriptorBindingFlagBits::ePartiallyBound;

            const auto binding_flags_create_info
                = vk::DescriptorSetLayoutBindingFlagsCreateInfo{}
                    .setBindingFlags(bindless_binding_flags)
                    .setPNext(&mutable_descriptor_create_info)
                    ;
    
            const auto binding 
                = vk::DescriptorSetLayoutBinding{}
                    .setBinding(0)
                    .setDescriptorType(vk::DescriptorType::eMutableEXT)
                    .setDescriptorCount(count)
                    .setStageFlags(vk::ShaderStageFlagBits::eAll)
                    ;
    
            const vk::DescriptorSetLayoutCreateFlags layout_flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
    
            const auto create_info
                = vk::DescriptorSetLayoutCreateInfo{}
                    .setBindings(binding)
                    .setFlags(layout_flags)
                    .setPNext(&binding_flags_create_info)
                    ;
    
            return device.createDescriptorSetLayout(create_info);
        }

        static vk::DescriptorSetLayout CreateDescriptorSetLayout(vk::Device device, vk::DescriptorType type, u32 count) noexcept {
            const auto bindless_binding_flags 
                = vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending 
                | vk::DescriptorBindingFlagBits::ePartiallyBound;
    
            const auto binding_flags_create_info
                = vk::DescriptorSetLayoutBindingFlagsCreateInfo{}
                    .setBindingFlags(bindless_binding_flags)
                    ;
    
            const auto binding 
                = vk::DescriptorSetLayoutBinding{}
                    .setBinding(0)
                    .setDescriptorType(type)
                    .setDescriptorCount(count)
                    .setStageFlags(vk::ShaderStageFlagBits::eAll)
                    ;
    
            vk::DescriptorSetLayoutCreateFlags layout_flags = vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT;
    
            const auto create_info
                = vk::DescriptorSetLayoutCreateInfo{}
                    .setBindings(binding)
                    .setFlags(layout_flags)
                    .setPNext(&binding_flags_create_info)
                    ;
    
            return device.createDescriptorSetLayout(create_info);
        }
    
        static vk::PipelineLayout CreatePipelineLayout(vk::Device device, std::span<const vk::DescriptorSetLayout> sets) noexcept {
            const auto push_constant = vk::PushConstantRange{vk::ShaderStageFlagBits::eAll, 0, c_Push_Constant_Size};
    
            const auto create_info
                = vk::PipelineLayoutCreateInfo{}
                    .setPushConstantRanges(push_constant)
                    .setSetLayouts(sets)
                    ;
    
            return device.createPipelineLayout(create_info);
        }
    }

    vk::SamplerCreateInfo DescriptorManager::GetSamplerCreateInfo(const SamplerDesc &sampler) noexcept {
        return vk::SamplerCreateInfo{}
            .setMinFilter(To_Vk(sampler.min_filter_type))
            .setMagFilter(To_Vk(sampler.mag_filter_type))
            .setMipmapMode(sampler.mip_filter_type == FilterType::eLinear ? vk::SamplerMipmapMode::eLinear : vk::SamplerMipmapMode::eNearest)
            .setMipLodBias(sampler.mip_lod_bias)
            .setMaxLod(sampler.max_lod)
            .setMinLod(sampler.min_lod)
            .setCompareOp(To_Vk(sampler.compare_op))
            .setAddressModeU(To_Vk(sampler.address_mode_u))
            .setAddressModeV(To_Vk(sampler.address_mode_v))
            .setAddressModeW(To_Vk(sampler.address_mode_w))
            .setAnisotropyEnable(sampler.anisotropy != 1.f)
            .setMaxAnisotropy(sampler.anisotropy)
            .setCompareEnable(sampler.compare_op != CompareOp::eNone)
            .setBorderColor(vk::BorderColor::eFloatTransparentBlack)
            ;
    }

    DescriptorManagerBuffer::DescriptorManagerBuffer(Device &device) : DescriptorManager(device) {
        m_resource_set_layout = Utility::CreateResourceDescriptorSetLayout(m_device.GetDevice(), m_device.GetSettings().resource_count);
        m_sampler_set_layout = Utility::CreateDescriptorSetLayout(m_device.GetDevice(), vk::DescriptorType::eSampler, m_device.GetSettings().sampler_count);
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), m_resource_set_layout, "ResourceSetLayout");
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), m_sampler_set_layout, "SamplerSetLayout");

        m_resource_layout_size = m_device.GetDevice().getDescriptorSetLayoutSizeEXT(m_resource_set_layout);
        m_sampler_layout_size = m_device.GetDevice().getDescriptorSetLayoutSizeEXT(m_sampler_set_layout);

        const auto layouts = {m_resource_set_layout, m_sampler_set_layout};
        m_pipeline_layout = Utility::CreatePipelineLayout(m_device.GetDevice(), layouts);
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), m_pipeline_layout, "GlobalPipelineLayout");

        auto buff_create_info 
            = vk::BufferCreateInfo{}
                .setSharingMode(vk::SharingMode::eExclusive)
                .setSize(m_resource_layout_size)
                .setUsage(vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT | vk::BufferUsageFlagBits::eShaderDeviceAddress)
                ;

        m_resource_buffer = std::make_unique<InternalBuffer>(m_device, buff_create_info, MemoryType::eDevice, HostMemoryAccess::eWrite);
        m_resource_buffer_address = m_resource_buffer->GetDeviceAddress();
        auto buffer = m_resource_buffer->GetBuffer();
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), buffer, "ResourceDescriptorBuffer");

        buff_create_info 
                .setSize(m_sampler_layout_size)
                .setUsage(vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT | vk::BufferUsageFlagBits::eShaderDeviceAddress)
                ;
                
        m_sampler_buffer = std::make_unique<InternalBuffer>(m_device, buff_create_info, MemoryType::eDevice, HostMemoryAccess::eWrite);
        m_sampler_buffer_address = m_sampler_buffer->GetDeviceAddress();
        buffer = m_sampler_buffer->GetBuffer();
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), buffer, "SamplerDescriptorBuffer");
    }

    DescriptorManagerBuffer::~DescriptorManagerBuffer() {
        m_device.GetDevice().destroy(m_resource_set_layout);
        m_device.GetDevice().destroy(m_sampler_set_layout);
        m_device.GetDevice().destroy(m_pipeline_layout);

        for (const auto sampler : m_samplers)
            m_device.GetDevice().destroy(sampler);
    }
    
    void DescriptorManagerBuffer::PushConstants(vk::CommandBuffer cmd_buffer, const void *data, u32 size, u32 offset) const noexcept {
        cmd_buffer.pushConstants(m_pipeline_layout, vk::ShaderStageFlagBits::eAll, offset, size, data);
    }

    void DescriptorManagerBuffer::BindCommandBuffer(vk::CommandBuffer cmd_buffer) const noexcept {
        const auto bind_info
            = std::array{
                vk::DescriptorBufferBindingInfoEXT{}
                .setUsage(vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT)
                .setAddress(m_sampler_buffer_address),
                vk::DescriptorBufferBindingInfoEXT{}
                .setUsage(vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT)
                .setAddress(m_resource_buffer_address),
            };

        cmd_buffer.bindDescriptorBuffersEXT(bind_info);

        cmd_buffer.setDescriptorBufferOffsetsEXT(
            vk::PipelineBindPoint::eGraphics,
            m_pipeline_layout, 
            0, 
            {0, 1}, 
            {0, 0});

        cmd_buffer.setDescriptorBufferOffsetsEXT(
            vk::PipelineBindPoint::eCompute,
            m_pipeline_layout, 
            0, 
            {0, 1}, 
            {0, 0});

        cmd_buffer.setDescriptorBufferOffsetsEXT(
            vk::PipelineBindPoint::eRayTracingKHR,
            m_pipeline_layout, 
            0, 
            {0, 1}, 
            {0, 0});
    }

    void DescriptorManagerBuffer::WriteResourceDescriptor(u32 index, vk::DescriptorType type, vk::DescriptorDataEXT data) {
        const auto offset = m_device.GetDevice().getDescriptorSetLayoutBindingOffsetEXT(m_resource_set_layout, index);

        m_device.GetDevice().getDescriptorEXT(
                vk::DescriptorGetInfoEXT{}.setType(type).setData(data),
                1, 
                GetResourceHostPtr() + offset
            );
    }

    void DescriptorManagerBuffer::SetUniformBufferBindlessIndex(u32 index, Buffer &buffer, BufferSubresource subresource)  {
        const auto descriptor_buffer_info
            = vk::DescriptorAddressInfoEXT{}
                .setAddress(buffer.GetDeviceAddress() + subresource.offset)
                .setRange(subresource.size)
                ;

        WriteResourceDescriptor(index, vk::DescriptorType::eUniformBuffer, &descriptor_buffer_info);
    }

    void DescriptorManagerBuffer::SetSampledImageBindlessIndex(u32 index, Image &image, ImageSubresource subresource) {
        const auto descriptor_image_info
            = vk::DescriptorImageInfo{}
                .setImageLayout(image.c_DefaultLayout)
                .setImageView(image.GetImageView(subresource))
                ;

        WriteResourceDescriptor(index, vk::DescriptorType::eSampledImage, &descriptor_image_info);
    }

    void DescriptorManagerBuffer::SetStorageImageBindlessIndex(u32 index, Image &image, ImageSubresource subresource) {
        const auto descriptor_image_info
            = vk::DescriptorImageInfo{}
                .setImageLayout(image.c_DefaultLayout)
                .setImageView(image.GetImageView(subresource))
                ;

        WriteResourceDescriptor(index, vk::DescriptorType::eStorageImage, &descriptor_image_info);
    }

    void DescriptorManagerBuffer::SetSamplerBindlessIndex(u32 index, const SamplerDesc &sampler_desc) { 
        const auto sampler = m_device.GetDevice().createSampler(GetSamplerCreateInfo(sampler_desc));
        m_samplers.push_back(sampler);

        m_device.GetDevice().getDescriptorEXT(
                vk::DescriptorGetInfoEXT{}.setType(vk::DescriptorType::eSampler).setData(&sampler),
                1, 
                GetSamplerHostPtr() + (index * m_device.GetVulkanDeviceProperties().sampler_descriptor_size)
            );
    }

    DescriptorManagerHeap::DescriptorManagerHeap(Device &device) 
    : DescriptorManager(device), 
    c_ResourceHeapOffset(
        m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap * m_device.GetSettings().resource_count
    ), 
    c_SamplerHeapOffset(
        m_device.GetVulkanDeviceProperties().sampler_descriptor_size_for_heap * m_device.GetSettings().sampler_count
    )
    {
        auto buff_create_info 
            = vk::BufferCreateInfo{}
                .setSharingMode(vk::SharingMode::eExclusive)
                .setUsage(vk::BufferUsageFlagBits::eDescriptorHeapEXT | vk::BufferUsageFlagBits::eShaderDeviceAddress)
                .setSize((m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap * (m_device.GetSettings().resource_count))
                        + (m_device.GetVulkanDeviceProperties().resource_heap_min_reserve)
                    )
                ;

        m_resource_buffer = std::make_unique<InternalBuffer>(
            m_device,
            buff_create_info, 
            MemoryType::eDevice, 
            HostMemoryAccess::eWrite,
            m_device.GetVulkanDeviceProperties().resource_heap_alignment
        );
        m_resource_buffer_address = m_resource_buffer->GetDeviceAddress();

        auto buffer = m_resource_buffer->GetBuffer();
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), buffer, "ResourceDescriptorHeap");

        buff_create_info 
                .setSize((m_device.GetVulkanDeviceProperties().sampler_descriptor_size_for_heap * m_device.GetSettings().sampler_count)
                        + (m_device.GetVulkanDeviceProperties().sampler_heap_min_reserve)
                );

        m_sampler_buffer = std::make_unique<InternalBuffer>(
            m_device,
            buff_create_info, 
            MemoryType::eDevice, 
            HostMemoryAccess::eWrite,
            m_device.GetVulkanDeviceProperties().sampler_heap_alignment
        );
        buffer = m_sampler_buffer->GetBuffer();
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), buffer, "SamplerDescriptorHeap");
        m_sampler_buffer_address = m_sampler_buffer->GetDeviceAddress();
    }

    void DescriptorManagerHeap::SetUniformBufferBindlessIndex(u32 index, Buffer &buffer, BufferSubresource subresource) {
        const auto device_adress_range
            = vk::DeviceAddressRangeEXT{}
                .setAddress(buffer.GetDeviceAddress() + subresource.offset)
                .setSize(subresource.size)
                ;

        const auto descriptor_info 
            = vk::ResourceDescriptorInfoEXT{}
                .setType(vk::DescriptorType::eUniformBuffer)
                .setData(&device_adress_range)
                ;

        m_device.GetDevice().writeResourceDescriptorsEXT(
            descriptor_info, 
            {{
                GetResourceHostPtr() + (index * m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap), 
                m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap}});
    }

    void DescriptorManagerHeap::SetSampledImageBindlessIndex(u32 index, Image &image, ImageSubresource subresource) {
        const auto image_view_create_info = image.GetCreateImageViewInfo(subresource);

        const auto image_info
            = vk::ImageDescriptorInfoEXT{}
                .setLayout(image.c_DefaultLayout)
                .setPView(&image_view_create_info)
                ;

        const auto descriptor_info 
            = vk::ResourceDescriptorInfoEXT{}
                .setType(vk::DescriptorType::eSampledImage)
                .setData(&image_info)
                ;

        m_device.GetDevice().writeResourceDescriptorsEXT(
            descriptor_info, 
            {{
                GetResourceHostPtr() + (index * m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap), 
                m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap}});
    }

    void DescriptorManagerHeap::SetStorageImageBindlessIndex(u32 index, Image &image, ImageSubresource subresource) {
        const auto image_view_create_info = image.GetCreateImageViewInfo(subresource);

        const auto image_info
            = vk::ImageDescriptorInfoEXT{}
                .setLayout(image.c_DefaultLayout)
                .setPView(&image_view_create_info)
                ;

        const auto descriptor_info 
            = vk::ResourceDescriptorInfoEXT{}
                .setType(vk::DescriptorType::eStorageImage)
                .setData(&image_info)
                ;

        m_device.GetDevice().writeResourceDescriptorsEXT(
            descriptor_info, 
            {{
                GetResourceHostPtr() + (index * m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap), 
                m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap}});
    }

    void DescriptorManagerHeap::SetSamplerBindlessIndex(u32 index, const SamplerDesc &sampler) {
        const auto sampler_create_info = GetSamplerCreateInfo(sampler);

        m_device.GetDevice().writeSamplerDescriptorsEXT(
            sampler_create_info, {{
                GetSamplerHostPtr() + (index * m_device.GetVulkanDeviceProperties().sampler_descriptor_size_for_heap), 
                m_device.GetVulkanDeviceProperties().sampler_descriptor_size_for_heap}});
    }

    void DescriptorManagerHeap::PushConstants(vk::CommandBuffer cmd_buffer, const void *data, u32 size, u32 offset) const noexcept {
        cmd_buffer.pushDataEXT(
            {offset, {data, size}}
        );
    }

    void DescriptorManagerHeap::BindCommandBuffer(vk::CommandBuffer cmd_buffer) const noexcept {
        cmd_buffer.bindResourceHeapEXT(
            vk::BindHeapInfoEXT{}
                .setHeapRange(vk::DeviceAddressRangeEXT{}
                    .setAddress(m_resource_buffer_address)
                    .setSize(
                        m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap * (m_device.GetSettings().resource_count)
                    + m_device.GetVulkanDeviceProperties().resource_heap_min_reserve))
                .setReservedRangeOffset(c_ResourceHeapOffset)
                .setReservedRangeSize(m_device.GetVulkanDeviceProperties().resource_heap_min_reserve)
            );

        cmd_buffer.bindSamplerHeapEXT(
            vk::BindHeapInfoEXT{}
                .setHeapRange(vk::DeviceAddressRangeEXT{}
                    .setAddress(m_sampler_buffer_address)
                    .setSize((m_device.GetVulkanDeviceProperties().sampler_descriptor_size_for_heap * m_device.GetSettings().sampler_count) + m_device.GetVulkanDeviceProperties().sampler_heap_min_reserve))
                .setReservedRangeOffset(c_SamplerHeapOffset)
                .setReservedRangeSize(m_device.GetVulkanDeviceProperties().sampler_heap_min_reserve)
            );
    }
}