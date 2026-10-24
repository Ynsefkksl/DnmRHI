#include <DnmRHI/Platform/Vulkan/DescriptorManager.hpp>
#include <DnmRHI/Platform/Vulkan/ToVk.hpp>
#include <DnmRHI/Platform/Vulkan/Buffer.hpp>
#include <DnmRHI/Platform/Vulkan/Image.hpp>

namespace DnmRHI::Vk {
    namespace Utility {
        static vk::DescriptorSetLayout CreateDescriptorSetLayout(vk::Device device, vk::DescriptorType type, u32 count, bool is_bindless) noexcept {
            const auto bindless_binding_flags 
                = vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending 
                | vk::DescriptorBindingFlagBits::ePartiallyBound;
    
            const auto binding_flags 
                = vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending 
                | vk::DescriptorBindingFlagBits::ePartiallyBound;
    
            const auto binding_flags_create_info
                = vk::DescriptorSetLayoutBindingFlagsCreateInfo{}
                    .setBindingFlags(is_bindless ? bindless_binding_flags : binding_flags )
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
            const auto push_constant = vk::PushConstantRange{vk::ShaderStageFlagBits::eAll, 0, Push_Constant_Size};
    
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
        m_uniform_buffer_set_layout = Utility::CreateDescriptorSetLayout(m_device.GetDevice(), vk::DescriptorType::eUniformBuffer, Uniform_Buffer_Count, false);
        m_sampled_image_set_layout = Utility::CreateDescriptorSetLayout(m_device.GetDevice(), vk::DescriptorType::eSampledImage, Sampled_Image_Count, true);
        m_storage_image_set_layout = Utility::CreateDescriptorSetLayout(m_device.GetDevice(), vk::DescriptorType::eStorageImage, Storage_Image_Count, true);
        m_sampler_set_layout = Utility::CreateDescriptorSetLayout(m_device.GetDevice(), vk::DescriptorType::eSampler, Sampler_Count, true);
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), m_uniform_buffer_set_layout, "UniformBufferSetLayout");
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), m_sampled_image_set_layout, "SampledImageSetLayout");
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), m_storage_image_set_layout, "StorageImageSetLayout");
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), m_sampler_set_layout, "SamplerSetLayout");

        m_uniform_buffer_layout_size = m_device.GetDevice().getDescriptorSetLayoutSizeEXT(m_uniform_buffer_set_layout);
        m_sampled_image_layout_size = m_device.GetDevice().getDescriptorSetLayoutSizeEXT(m_sampled_image_set_layout);
        m_storage_image_layout_size = m_device.GetDevice().getDescriptorSetLayoutSizeEXT(m_storage_image_set_layout);
        m_sampler_layout_size = m_device.GetDevice().getDescriptorSetLayoutSizeEXT(m_sampler_set_layout);

        const auto layouts = {m_uniform_buffer_set_layout, m_sampled_image_set_layout, m_storage_image_set_layout, m_sampled_image_set_layout};
        m_pipeline_layout = Utility::CreatePipelineLayout(m_device.GetDevice(), layouts);
        SET_DEBUG_NAME_VK_OBJECT(m_device.GetDevice(), m_pipeline_layout, "GlobalPipelineLayout");

        auto buff_create_info 
            = vk::BufferCreateInfo{}
                .setSharingMode(vk::SharingMode::eExclusive)
                .setSize(m_uniform_buffer_layout_size + m_sampled_image_layout_size + m_storage_image_layout_size)
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
        m_device.GetDevice().destroy(m_uniform_buffer_set_layout);
        m_device.GetDevice().destroy(m_sampled_image_set_layout);
        m_device.GetDevice().destroy(m_storage_image_set_layout);
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
            {0, 1, 1, 1}, 
            {0, 0, m_uniform_buffer_layout_size, (m_storage_image_layout_size + m_uniform_buffer_layout_size)});
            
        cmd_buffer.setDescriptorBufferOffsetsEXT(
            vk::PipelineBindPoint::eCompute,
            m_pipeline_layout, 
            0, 
            {0, 1, 1, 1}, 
            {0, 0, m_uniform_buffer_layout_size, (m_storage_image_layout_size + m_uniform_buffer_layout_size)});
    }

    u32 DescriptorManagerBuffer::GetSampledImageBindlessIndex(Image &image, ImageSubresource subresource) {
        u32 index;
        if (m_sampled_image_bindless_index >= 200'000) {
            index = m_sampled_image_bindless_pending_indexes.back();
            m_sampled_image_bindless_pending_indexes.pop_back();
        }
        else {
            index = m_sampled_image_bindless_index++;
        }

        const auto descriptor_image_info
            = vk::DescriptorImageInfo{}
                .setImageLayout(image.c_DefaultLayout)
                .setImageView(image.GetImageView(subresource))
                ;

        m_device.GetDevice().getDescriptorEXT(
                vk::DescriptorGetInfoEXT{}.setType(vk::DescriptorType::eSampledImage).setData(&descriptor_image_info),
                1, 
                GetSampledImageHostPtr() + (index * m_device.GetVulkanDeviceProperties().sampled_image_descriptor_size)
            );
        return index;
    }

    u32 DescriptorManagerBuffer::GetStorageImageBindlessIndex(Image &image, ImageSubresource subresource) {
        u32 index;
        if (m_storage_image_bindless_index >= 20'000) {
            index = m_storage_image_bindless_pending_indexes.back();
            m_storage_image_bindless_pending_indexes.pop_back();
        }
        else {
            index = m_storage_image_bindless_index++;
        }

        const auto descriptor_image_info
            = vk::DescriptorImageInfo{}
                .setImageLayout(image.c_DefaultLayout)
                .setImageView(image.GetImageView(subresource))
                ;

        m_device.GetDevice().getDescriptorEXT(
                vk::DescriptorGetInfoEXT{}.setType(vk::DescriptorType::eStorageImage).setData(&descriptor_image_info),
                1, 
                GetStorageImageHostPtr() + (index * m_device.GetVulkanDeviceProperties().storage_image_descriptor_size)
            );
        return index;
    }

    u32 DescriptorManagerBuffer::GetSamplerBindlessIndex(const SamplerDesc &sampler_desc) { 
        u32 index;
        if (m_sampler_bindless_index >= 2'000) {
            index = m_sampler_bindless_pending_indexes.back();
            m_sampler_bindless_pending_indexes.pop_back();
        }
        else {
            index = m_sampler_bindless_index++;
        }

        const auto sampler = m_device.GetDevice().createSampler(GetSamplerCreateInfo(sampler_desc));
        m_samplers.push_back(sampler);

        m_device.GetDevice().getDescriptorEXT(
                vk::DescriptorGetInfoEXT{}.setType(vk::DescriptorType::eSampler).setData(&sampler),
                1, 
                GetSamplerHostPtr() + (index * m_device.GetVulkanDeviceProperties().sampler_descriptor_size)
            );
        return index;
    }

    void DescriptorManagerBuffer::SetUniformBuffer(Buffer &buffer, u32 index, u32 offset, u32 size) const {
        const auto buffer_info
            = vk::DescriptorAddressInfoEXT{}
                .setAddress(buffer.GetDeviceAddress() + offset)
                .setRange(size)
                ;

        m_device.GetDevice().getDescriptorEXT(
                vk::DescriptorGetInfoEXT{}.setType(vk::DescriptorType::eUniformBuffer).setData(&buffer_info),
                1, 
                GetUboHostPtr() + (index * m_device.GetVulkanDeviceProperties().ubo_image_descriptor_size)
            );
    }

    DescriptorManagerHeap::DescriptorManagerHeap(Device &device) : DescriptorManager(device) {
        auto buff_create_info 
            = vk::BufferCreateInfo{}
                .setSharingMode(vk::SharingMode::eExclusive)
                .setUsage(vk::BufferUsageFlagBits::eDescriptorHeapEXT | vk::BufferUsageFlagBits::eShaderDeviceAddress)
                .setSize((m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap * (Uniform_Buffer_Count + Storage_Image_Count + Sampled_Image_Count))
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
                .setSize((m_device.GetVulkanDeviceProperties().sampler_descriptor_size_for_heap * Sampler_Count)
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

    u32 DescriptorManagerHeap::GetSampledImageBindlessIndex(Image &image, ImageSubresource subresource) {
        u32 index;
        if (m_resource_bindless_index >= Uniform_Buffer_Count + Storage_Image_Count + Sampled_Image_Count) {
            index = m_resource_bindless_pending_indexes.back();
            m_resource_bindless_pending_indexes.pop_back();
        }
        else {
            index = m_resource_bindless_index++;
        }

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
            {{GetSampledImageHostPtr() + index, m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap}});
        return index;
    }

    u32 DescriptorManagerHeap::GetStorageImageBindlessIndex(Image &image, ImageSubresource subresource) {
        u32 index;
        if (m_resource_bindless_index >= Uniform_Buffer_Count + Storage_Image_Count + Sampled_Image_Count) {
            index = m_resource_bindless_pending_indexes.back();
            m_resource_bindless_pending_indexes.pop_back();
        }
        else {
            index = m_resource_bindless_index++;
        }

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
            {{GetStorageImageHostPtr() + index, m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap}});
        return index;
    }

    u32 DescriptorManagerHeap::GetSamplerBindlessIndex(const SamplerDesc &sampler) {
        u32 index;
        if (m_resource_bindless_index >= Uniform_Buffer_Count + Storage_Image_Count + Sampled_Image_Count) {
            index = m_resource_bindless_pending_indexes.back();
            m_resource_bindless_pending_indexes.pop_back();
        }
        else {
            index = m_resource_bindless_index++;
        }

        const auto sampler_create_info = GetSamplerCreateInfo(sampler);

        m_device.GetDevice().writeSamplerDescriptorsEXT(
            sampler_create_info, {{GetSamplerHostPtr() + index, m_device.GetVulkanDeviceProperties().sampler_descriptor_size_for_heap}});
        
        return index;
    }

    void DescriptorManagerHeap::SetUniformBuffer(Buffer &buffer, u32 index, u32 offset, u32 size) const {
        const auto device_adress_range
            = vk::DeviceAddressRangeEXT{}
                .setAddress(buffer.GetDeviceAddress() + offset)
                .setSize(size)
                ;

        const auto descriptor_info 
            = vk::ResourceDescriptorInfoEXT{}
                .setType(vk::DescriptorType::eUniformBuffer)
                .setData(&device_adress_range)
                ;

        m_device.GetDevice().writeResourceDescriptorsEXT(
            descriptor_info, 
            {{GetUboHostPtr() + index, m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap}});
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
                    .setAddress(GetUboDeviceAddress())
                    .setSize(
                        m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap * (Uniform_Buffer_Count + Storage_Image_Count + Sampled_Image_Count)
                    + m_device.GetVulkanDeviceProperties().resource_heap_min_reserve))
                .setReservedRangeOffset(GetResourceHeapReservedOffset())
                .setReservedRangeSize(m_device.GetVulkanDeviceProperties().resource_heap_min_reserve)
            );

        cmd_buffer.bindSamplerHeapEXT(
            vk::BindHeapInfoEXT{}
                .setHeapRange(vk::DeviceAddressRangeEXT{}
                    .setAddress(GetSamplerDeviceAddress())
                    .setSize((m_device.GetVulkanDeviceProperties().sampler_descriptor_size_for_heap * Sampler_Count) + m_device.GetVulkanDeviceProperties().sampler_heap_min_reserve))
                .setReservedRangeOffset(GetSamplerHeapReservedOffset())
                .setReservedRangeSize(m_device.GetVulkanDeviceProperties().sampler_heap_min_reserve)
            );
    }
}