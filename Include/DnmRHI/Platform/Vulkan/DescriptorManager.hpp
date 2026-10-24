#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Platform/Vulkan/Buffer.hpp>
#include <DnmRHI/Platform/Vulkan/Image.hpp>

namespace DnmRHI::Vk {
    class DescriptorManager {
    public:
        explicit DescriptorManager(Vk::Device &device) noexcept : m_device(device) {}
        virtual ~DescriptorManager() = default;

        [[nodiscard]] virtual u32 GetSampledImageBindlessIndex(Image &image, ImageSubresource subresource) = 0;
        [[nodiscard]] virtual u32 GetStorageImageBindlessIndex(Image &image, ImageSubresource subresource) = 0;
        [[nodiscard]] virtual u32 GetSamplerBindlessIndex(const SamplerDesc &sampler) = 0;

        virtual void DeleteSampledImageBindlessIndex(u32 index) = 0;
        virtual void DeleteStorageImageBindlessIndex(u32 index) = 0;
        virtual void DeleteSamplerBindlessIndex(u32 index) = 0;

        virtual void SetUniformBuffer(Buffer &buffer, u32 index, u32 offset, u32 size) const = 0;
        [[nodiscard]] virtual vk::PipelineLayout GetPipelineLayout() const noexcept = 0;

        virtual void PushConstants(vk::CommandBuffer cmd_buffer, const void *data, u32 size, u32 offset) const noexcept = 0;
        virtual void BindCommandBuffer(vk::CommandBuffer cmd_buffer) const noexcept = 0;
    protected:
        static vk::SamplerCreateInfo GetSamplerCreateInfo(const SamplerDesc &sampler) noexcept;

        std::unique_ptr<InternalBuffer> m_resource_buffer{};
        std::unique_ptr<InternalBuffer> m_sampler_buffer{};
        u64 m_resource_buffer_address;
        u64 m_sampler_buffer_address;

        Vk::Device &m_device;
    };

    class DescriptorManagerBuffer final : public DescriptorManager {
    public:
        DescriptorManagerBuffer(Vk::Device &device);
        virtual ~DescriptorManagerBuffer() override;

        [[nodiscard]] virtual u32 GetSampledImageBindlessIndex(Image &image, ImageSubresource subresource) override;
        [[nodiscard]] virtual u32 GetStorageImageBindlessIndex(Image &image, ImageSubresource subresource) override;
        [[nodiscard]] virtual u32 GetSamplerBindlessIndex(const SamplerDesc &sampler) override;

        virtual void DeleteSampledImageBindlessIndex(u32 index) override { m_sampled_image_bindless_pending_indexes.push_back(index); }
        virtual void DeleteStorageImageBindlessIndex(u32 index) override { m_storage_image_bindless_pending_indexes.push_back(index); }
        virtual void DeleteSamplerBindlessIndex(u32 index) override { m_sampler_bindless_pending_indexes.push_back(index); }

        virtual void SetUniformBuffer(Buffer &buffer, u32 index, u32 offset, u32 size) const override;
        [[nodiscard]] virtual vk::PipelineLayout GetPipelineLayout() const noexcept override { return m_pipeline_layout; }

        virtual void PushConstants(vk::CommandBuffer cmd_buffer, const void *data, u32 size, u32 offset) const noexcept override;
        virtual void BindCommandBuffer(vk::CommandBuffer cmd_buffer) const noexcept override;
    protected:
        [[nodiscard]] u8 *GetUboHostPtr() const noexcept { return m_resource_buffer->GetMappedPtr<u8>(); }
        [[nodiscard]] u8 *GetStorageImageHostPtr() const noexcept { return GetUboHostPtr() + m_uniform_buffer_layout_size; }
        [[nodiscard]] u8 *GetSampledImageHostPtr() const noexcept { return GetStorageImageHostPtr() + m_storage_image_layout_size; }
        [[nodiscard]] u8 *GetSamplerHostPtr() const noexcept { return m_sampler_buffer->GetMappedPtr<u8>(); }

        std::vector<vk::Sampler> m_samplers{};
        vk::PipelineLayout m_pipeline_layout{};

        vk::DescriptorSetLayout m_uniform_buffer_set_layout{};
        vk::DescriptorSetLayout m_sampled_image_set_layout{};
        vk::DescriptorSetLayout m_storage_image_set_layout{};
        vk::DescriptorSetLayout m_sampler_set_layout{};

        u32 m_uniform_buffer_layout_size{};
        u32 m_sampled_image_layout_size{};
        u32 m_storage_image_layout_size{};
        u32 m_sampler_layout_size{};

        u32 m_sampled_image_bindless_index{};
        u32 m_storage_image_bindless_index{};
        u32 m_sampler_bindless_index{};

        std::vector<u32> m_sampled_image_bindless_pending_indexes{};
        std::vector<u32> m_storage_image_bindless_pending_indexes{};
        std::vector<u32> m_sampler_bindless_pending_indexes{};
    };

    class DescriptorManagerHeap final : public DescriptorManager {
    public:
        DescriptorManagerHeap(Vk::Device &device);
        virtual ~DescriptorManagerHeap() override = default;

        [[nodiscard]] virtual u32 GetSampledImageBindlessIndex(Image &image, ImageSubresource subresource) override;
        [[nodiscard]] virtual u32 GetStorageImageBindlessIndex(Image &image, ImageSubresource subresource) override;
        [[nodiscard]] virtual u32 GetSamplerBindlessIndex(const SamplerDesc &sampler) override;

        virtual void DeleteSampledImageBindlessIndex(u32 index) override { m_resource_bindless_pending_indexes.push_back(index); }
        virtual void DeleteStorageImageBindlessIndex(u32 index) override { m_resource_bindless_pending_indexes.push_back(index); }
        virtual void DeleteSamplerBindlessIndex(u32 index) override { m_sampler_bindless_pending_indexes.push_back(index); }

        virtual void SetUniformBuffer(Buffer &buffer, u32 index, u32 offset, u32 size) const override;
        [[nodiscard]] virtual vk::PipelineLayout GetPipelineLayout() const noexcept override { return {}; }

        virtual void PushConstants(vk::CommandBuffer cmd_buffer, const void *data, u32 size, u32 offset) const noexcept override;
        virtual void BindCommandBuffer(vk::CommandBuffer cmd_buffer) const noexcept override;
    private:
        [[nodiscard]] u8 *GetUboHostPtr() const noexcept { return m_resource_buffer->GetMappedPtr<u8>(); }
        [[nodiscard]] u8 *GetStorageImageHostPtr() const noexcept { return GetUboHostPtr() + (m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap * Uniform_Buffer_Count); }
        [[nodiscard]] u8 *GetSampledImageHostPtr() const noexcept { return GetStorageImageHostPtr() + (m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap * (Storage_Image_Count + Uniform_Buffer_Count)); }
        [[nodiscard]] u8 *GetSamplerHostPtr() const noexcept { return m_sampler_buffer->GetMappedPtr<u8>(); }

        [[nodiscard]] u64 GetUboDeviceAddress() const noexcept { return m_resource_buffer_address; }
        [[nodiscard]] u64 GetStorageImageDeviceAddress() const noexcept { return GetUboDeviceAddress() + (m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap * Uniform_Buffer_Count); }
        [[nodiscard]] u64 GetSampledImageDeviceAddress() const noexcept { return GetStorageImageDeviceAddress() + (m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap * (Storage_Image_Count + Uniform_Buffer_Count)); }
        [[nodiscard]] u64 GetSamplerDeviceAddress() const noexcept { return m_sampler_buffer_address; }

        [[nodiscard]] u64 GetSamplerHeapReservedOffset() const noexcept { return (m_device.GetVulkanDeviceProperties().sampler_descriptor_size_for_heap * Sampler_Count); }
        [[nodiscard]] u64 GetResourceHeapReservedOffset() const noexcept { return (m_device.GetVulkanDeviceProperties().resource_descriptor_size_for_heap * Uniform_Buffer_Count + Storage_Image_Count + Sampled_Image_Count); }

        [[nodiscard]] u64 GetSamplerHeapReservedDeviceAddress() const noexcept { return GetSamplerDeviceAddress() + GetSamplerHeapReservedOffset(); }
        [[nodiscard]] u64 GetResourceHeapReservedDeviceAddress() const noexcept { return GetUboDeviceAddress() + GetResourceHeapReservedOffset(); }

        u32 m_resource_bindless_index{15}; // first 15 index reserved for ubo
        u32 m_sampler_bindless_index{};

        std::vector<u32> m_resource_bindless_pending_indexes{};
        std::vector<u32> m_sampler_bindless_pending_indexes{};
    };
}