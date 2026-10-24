#pragma once

#include <DnmRHI/CommandBuffer/CommandBuffer.hpp>
#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Platform/Vulkan/ToVk.hpp>
#include <DnmRHI/Platform/Vulkan/Image.hpp>
#include <unordered_map>

namespace DnmRHI::Vk {
    class CommandBuffer final : public DnmRHI::CommandBuffer {
    public:
        explicit CommandBuffer(Vk::Device &device, const CommandBufferDesc &desc) noexcept;
        virtual ~CommandBuffer() noexcept override {
            GetVkDevice.GetDevice().waitIdle();
            GetVkDevice.GetDevice().destroy(m_command_pool);
        }

        virtual void IBegin(bool one_time_submit) noexcept override;
        virtual void IEnd() noexcept override;
        virtual void IReset() noexcept override;

        // General Queue Commands
        virtual void IBeginRenderPass(const RenderPassDesc &desc) noexcept override;

        virtual void ISetViewport(std::span<const Viewport> viewport) noexcept override;
        virtual void ISetScissor(std::span<const Scissor> scissor) noexcept override;
        virtual void ISetStencilRef(u32 value, u32 back) noexcept override;

        virtual void IDraw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) noexcept override;
        virtual void IDrawIndexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) noexcept override;
        virtual void IDrawIndirect(const DrawIndirectDesc &desc) noexcept override;
        virtual void IDrawIndexedIndirect(const DrawIndirectDesc &desc) noexcept override;

        virtual void IEndRenderPass() noexcept override;
        
        virtual void IBlitImage(const BlitImageDesc &desc) noexcept override;
        virtual void IResolveImage(const ResolveImageDesc &desc) noexcept override;
        
        virtual void IBindIndexBuffer(DnmRHI::Buffer &buffer, u64 offset, IndexType index_type) noexcept override;
        virtual void IBindPipeline(DnmRHI::GraphicsPipeline &pipeline) noexcept override;
        virtual void IPrepareForPresent(AccessFlags before_access, DnmRHI::Image &swapchain_image) noexcept override;
        // General Queue Commands
        
        // Compute Queue Commands
        virtual void IDispatch(u32 x, u32 y, u32 z) noexcept override;
        virtual void IDispatchIndirect(DnmRHI::Buffer &buffer, u32 offset) noexcept override;

        virtual void IBeginConditionalRendering(DnmRHI::Buffer &buffer, u32 offset, bool is_inverted) noexcept override;
        virtual void IEndConditionalRendering() noexcept override;

        virtual void IPushConstant(const void *data, u32 size, u32 offset) noexcept override;
        virtual void IBindPipeline(DnmRHI::ComputePipeline &pipeline) noexcept override;
        // Compute Queue Commands

        // Copy Queue Commands
        virtual void ICopyBuffer(const BufferCopyDesc &desc) noexcept override;
        virtual void ICopyImage(const ImageCopyDesc &desc) noexcept override;
        virtual void ICopyBufferToImage(const BufferImageCopyDesc &desc) noexcept override;
        virtual void ICopyImageToBuffer(const BufferImageCopyDesc &desc) noexcept override;
        // Copy Queue Commands

        virtual void IBarrier(std::span<const BarrierDesc> barriers) noexcept override;
        virtual void IInitImage(std::span<const InitImageDesc> descs) noexcept override;
        
        virtual void IBeginDebugMarker(std::string_view name, std::array<u8, 4> color) noexcept override;
        virtual void IEndDebugMarker() noexcept override;

        [[nodiscard]] auto GetCommandBuffer() const noexcept { return m_command_buffer; }
        [[nodiscard]] auto GetCommandPool() const noexcept { return m_command_pool; }

        [[nodiscard]] auto GetUsedSwapchainImages() const noexcept { return std::span(m_used_swapchain_images); }
    private:
        vk::ImageLayout GetImageLayout(const Vk::Image *image, bool stencil = false) const noexcept {
            if (image->GetDesc().flags.Has(ImageFlagBit::eStorage)) return vk::ImageLayout::eGeneral;
            if (m_image_layouts.contains(image))
                return m_image_layouts.at(image)[stencil];
            return image->c_DefaultLayout;
        }

        void SetImageLayout(const Vk::Image *image, vk::ImageLayout layout, bool stencil = false) noexcept {
            if (image->GetDesc().flags.Has(ImageFlagBit::eStorage)) return;
            m_image_layouts[image][stencil] = layout;
        }

        struct ImageBarrierDesc {
            Sync src_sync;
            Sync dst_sync;
            const Vk::Image *image;
            ImageSubresource subresource;
            vk::ImageLayout old_layout;
            vk::ImageLayout new_layout;
        };

        struct MemoryBarrierDesc {
            Sync src_sync;
            Sync dst_sync;
        };

        void InternalBarrier(std::span<const MemoryBarrierDesc> memory_barriers, std::span<const ImageBarrierDesc> image_barriers);

        vk::CommandBuffer m_command_buffer;
        vk::CommandPool m_command_pool;

        std::vector<DnmRHI::Object *> m_used_objects;
        // 1. layout for color/depth, 2. layout for stencil
        std::unordered_map<const Vk::Image *, std::array<vk::ImageLayout, 2>> m_image_layouts;

        struct UsedSwapchainImageInfo {
            Swapchain *swapchain;
            u8 image_index;
        };
        std::vector<UsedSwapchainImageInfo> m_used_swapchain_images;
    };
};