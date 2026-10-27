#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Platform/Vulkan/Buffer.hpp>
#include <DnmRHI/Platform/Vulkan/Image.hpp>

namespace DnmRHI::Vk {
    class DescriptorManager {
    public:
        explicit DescriptorManager(Vk::Device &device) noexcept : m_device(device) {}
        virtual ~DescriptorManager() = default;

        virtual void SetUniformBufferBindlessIndex(u32 index, Buffer &buffer, BufferSubresource subresource) = 0;
        virtual void SetSampledImageBindlessIndex(u32 index, Image &image, ImageSubresource subresource) = 0;
        virtual void SetStorageImageBindlessIndex(u32 index, Image &image, ImageSubresource subresource) = 0;
        virtual void SetSamplerBindlessIndex(u32 index, const SamplerDesc &sampler) = 0;

        [[nodiscard]] virtual vk::PipelineLayout GetPipelineLayout() const noexcept = 0;

        virtual void PushConstants(vk::CommandBuffer cmd_buffer, const void *data, u32 size, u32 offset) const noexcept = 0;
        virtual void BindCommandBuffer(vk::CommandBuffer cmd_buffer) const noexcept = 0;
    protected:
        static vk::SamplerCreateInfo GetSamplerCreateInfo(const SamplerDesc &sampler) noexcept;

        [[nodiscard]] u8 *GetResourceHostPtr() const noexcept { return m_resource_buffer->GetMappedPtr<u8>(); }
        [[nodiscard]] u8 *GetSamplerHostPtr() const noexcept { return m_sampler_buffer->GetMappedPtr<u8>(); }

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

        virtual void SetUniformBufferBindlessIndex(u32 index, Buffer &buffer, BufferSubresource subresource) override;
        virtual void SetSampledImageBindlessIndex(u32 index, Image &image, ImageSubresource subresource) override;
        virtual void SetStorageImageBindlessIndex(u32 index, Image &image, ImageSubresource subresource) override;
        virtual void SetSamplerBindlessIndex(u32 index, const SamplerDesc &sampler) override;

        [[nodiscard]] virtual vk::PipelineLayout GetPipelineLayout() const noexcept override { return m_pipeline_layout; }

        virtual void PushConstants(vk::CommandBuffer cmd_buffer, const void *data, u32 size, u32 offset) const noexcept override;
        virtual void BindCommandBuffer(vk::CommandBuffer cmd_buffer) const noexcept override;
    protected:
        void WriteResourceDescriptor(u32 index, vk::DescriptorType type, vk::DescriptorDataEXT data);

        std::vector<vk::Sampler> m_samplers{};
        vk::PipelineLayout m_pipeline_layout{};

        vk::DescriptorSetLayout m_resource_set_layout{};
        vk::DescriptorSetLayout m_sampler_set_layout{};

        u32 m_resource_layout_size{};
        u32 m_sampler_layout_size{};
    };

    class DescriptorManagerHeap final : public DescriptorManager {
    public:
        DescriptorManagerHeap(Vk::Device &device);
        virtual ~DescriptorManagerHeap() override = default;

        virtual void SetUniformBufferBindlessIndex(u32 index, Buffer &buffer, BufferSubresource subresource) override;
        virtual void SetSampledImageBindlessIndex(u32 index, Image &image, ImageSubresource subresource) override;
        virtual void SetStorageImageBindlessIndex(u32 index, Image &image, ImageSubresource subresource) override;
        virtual void SetSamplerBindlessIndex(u32 index, const SamplerDesc &sampler) override;

        [[nodiscard]] virtual vk::PipelineLayout GetPipelineLayout() const noexcept override { return {}; }

        virtual void PushConstants(vk::CommandBuffer cmd_buffer, const void *data, u32 size, u32 offset) const noexcept override;
        virtual void BindCommandBuffer(vk::CommandBuffer cmd_buffer) const noexcept override;
    private:
        [[nodiscard]] u64 GetSamplerHeapReservedDeviceAddress() const noexcept { return m_sampler_buffer_address + c_SamplerHeapOffset; }
        [[nodiscard]] u64 GetResourceHeapReservedDeviceAddress() const noexcept { return m_resource_buffer_address + c_ResourceHeapOffset; }

        const u32 c_SamplerHeapOffset;
        const u32 c_ResourceHeapOffset;

        u32 m_resource_bindless_index{15}; // first 15 index reserved for ubo
        u32 m_sampler_bindless_index{};

        std::vector<u32> m_resource_bindless_pending_indexes{};
        std::vector<u32> m_sampler_bindless_pending_indexes{};
    };
}