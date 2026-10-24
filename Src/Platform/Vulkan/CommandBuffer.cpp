#include <DnmRHI/Platform/Vulkan/CommandBuffer.hpp>
#include <DnmRHI/Platform/Vulkan/GraphicsPipeline.hpp>
#include <DnmRHI/Platform/Vulkan/ComputePipeline.hpp>
#include <DnmRHI/Platform/Vulkan/DescriptorManager.hpp>

namespace DnmRHI::Vk {
    namespace Utility {
        static constexpr Sync GetSyncFromImageLayout(vk::ImageLayout image_layout, ImageFlags flags, b8 color_format) noexcept {
            switch (image_layout) {
                case vk::ImageLayout::eGeneral: {
                    Sync sync{};
                    if (flags.Has(DnmRHI::ImageFlagBit::eAttachment)) {
                        sync.stage |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;
                        sync.access |= vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
                    }
                    if (flags.Has(DnmRHI::ImageFlagBit::eVariableRateShadingAttachment)) {
                        sync.stage |= vk::PipelineStageFlagBits2::eFragmentShadingRateAttachmentKHR;
                        sync.access |= vk::AccessFlagBits2::eFragmentShadingRateAttachmentReadKHR;
                    }
                    if (flags.Has(DnmRHI::ImageFlagBit::eCopyDst)) {
                        sync.stage |= vk::PipelineStageFlagBits2::eTransfer;
                        sync.access |= vk::AccessFlagBits2::eTransferWrite;
                    }
                    if (flags.Has(DnmRHI::ImageFlagBit::eCopySrc)) {
                        sync.stage |= vk::PipelineStageFlagBits2::eTransfer;
                        sync.access |= vk::AccessFlagBits2::eTransferRead;
                    }
                    return sync;
                }
                case vk::ImageLayout::eTransferSrcOptimal: return {
                                    vk::PipelineStageFlagBits2::eTransfer, 
                                    vk::AccessFlagBits2::eTransferRead
                                };
                case vk::ImageLayout::eTransferDstOptimal: return {
                                    vk::PipelineStageFlagBits2::eTransfer, 
                                    vk::AccessFlagBits2::eTransferWrite
                                };
                case vk::ImageLayout::eReadOnlyOptimal: return {
                                    color_format ? vk::PipelineStageFlagBits2::eAllGraphics : 
                                                        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests | vk::PipelineStageFlagBits2::eAllGraphics, 
                                    color_format ? vk::AccessFlagBits2::eShaderSampledRead : 
                                                        vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eShaderSampledRead
                                };
                case vk::ImageLayout::eAttachmentOptimal: return {
                                    color_format ? vk::PipelineStageFlagBits2::eColorAttachmentOutput : 
                                                        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, 
                                    color_format ? vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite : 
                                                        vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite
                                };
                case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR: return {
                                    vk::PipelineStageFlagBits2::eFragmentShadingRateAttachmentKHR, 
                                    vk::AccessFlagBits2::eFragmentShadingRateAttachmentReadKHR
                                };
            default: std::unreachable();
            }
        }
    }

    CommandBuffer::CommandBuffer(Vk::Device &device, const CommandBufferDesc &desc) noexcept
    : DnmRHI::CommandBuffer(device, desc) {
        m_command_pool = GetVkDevice.GetDevice().createCommandPool(
            vk::CommandPoolCreateInfo(
                vk::CommandPoolCreateFlagBits{}, 
                GetVkDevice.GetQueueFamilyIndex(desc.queue_type)));

        m_command_buffer = GetVkDevice.GetDevice().allocateCommandBuffers(
            vk::CommandBufferAllocateInfo(
                m_command_pool, 
                To_Vk(desc.type), 
                1))[0];

        if (!desc.name.empty()) SET_DEBUG_NAME_VK_OBJECT(GetVkDevice.GetDevice(), m_command_pool, std::format("{} (command pool)", m_desc.name).c_str());
        if (!desc.name.empty()) SET_DEBUG_NAME_VK_OBJECT(GetVkDevice.GetDevice(), m_command_buffer, std::format("{} (command buffer)", m_desc.name).c_str());
    }

    void CommandBuffer::IBegin(bool one_time_submit) noexcept {
        m_command_buffer.begin(
            vk::CommandBufferBeginInfo{}
                .setFlags(one_time_submit ? vk::CommandBufferUsageFlagBits::eOneTimeSubmit : vk::CommandBufferUsageFlagBits{})
        );

        // TODO: this really needed?
        // for safe buffer read
        {
            const auto mem_barrier
                = vk::MemoryBarrier2{}
                    .setSrcStageMask(vk::PipelineStageFlagBits2::eHost)
                    .setSrcAccessMask(vk::AccessFlagBits2::eHostWrite)
                    ;
    
            const auto dependency_info 
                = vk::DependencyInfo{}
                    .setMemoryBarriers(mem_barrier);
    
            m_command_buffer.pipelineBarrier2(dependency_info);
        }

        GetVkDevice.GetDescriptorManager()->BindCommandBuffer(m_command_buffer);
        m_used_swapchain_images.clear();
    }

    void CommandBuffer::IEnd() noexcept {
        std::vector<ImageBarrierDesc> image_barriers;
        image_barriers.reserve(m_image_layouts.size() * 2);
        for (const auto &[image, layouts] : m_image_layouts) {
            if (image->c_IsSwapchainImage) continue;
            // color/depth
            if (image->c_DefaultLayout != layouts[0] && vk::ImageLayout::eUndefined != layouts[0]) {
                image_barriers.emplace_back(
                    Utility::GetSyncFromImageLayout(layouts[0], image->GetDesc().flags, 
                        IsColorFormat(image->GetDesc().format)),
                    Sync{},
                    image,
                    ImageSubresource{}.SetLayerCount(0).SetMipmapCount(0),
                    layouts[0],
                    image->c_DefaultLayout
                );
            }
            // stencil
            if (image->c_DefaultLayout != layouts[1] && vk::ImageLayout::eUndefined != layouts[1]) {
                image_barriers.emplace_back(
                    Utility::GetSyncFromImageLayout(layouts[0], image->GetDesc().flags, false),
                    Sync{},
                    image,
                    ImageSubresource{}.SetLayerCount(0).SetMipmapCount(0),
                    layouts[0],
                    image->c_DefaultLayout
                );
            }
        }
        InternalBarrier({}, image_barriers);
        m_command_buffer.end();
        m_image_layouts.clear();
    }

    void CommandBuffer::IReset() noexcept {
        GetVkDevice.GetDevice().resetCommandPool(m_command_pool);
    }

    // General Queue Commands
    void CommandBuffer::IBeginRenderPass(const RenderPassDesc &desc) noexcept {
        const auto read_only_depth = desc.flags.Has(RenderPassFlagBit::eReadOnlyDepth);
        const auto read_only_stencil = desc.flags.Has(RenderPassFlagBit::eReadOnlyStencil);

        if (!GetVkDeviceFeatures.unified_image_layouts) {
            std::vector<ImageBarrierDesc> barriers;
            barriers.reserve((desc.attachments.size() + 2) * 2);

            for (const auto &attachment : desc.attachments) {
                auto *vk_image = static_cast<Vk::Image *>(attachment.image);
                auto *vk_resolve_image = static_cast<Vk::Image *>(attachment.resolve_image);

                const auto image_layout = GetImageLayout(vk_image);

                if (image_layout != vk_image->c_AttachmentImageLayout) {
                    barriers.emplace_back(
                            //TODO:
                            Sync{}, 
                            Sync{
                                vk::PipelineStageFlagBits2::eColorAttachmentOutput, 
                                attachment.load_op == AttachmentLoadOp::eLoad ? vk::AccessFlagBits2::eColorAttachmentWrite | vk::AccessFlagBits2::eColorAttachmentRead : vk::AccessFlagBits2::eColorAttachmentRead},
                            vk_image,
                            ImageSubresource{}
                                .SetAspectFlags(DnmRHI::AspectFlagBit::eColor)
                                .SetBaseLayer(0)
                                .SetBaseMipmap(0)
                                .SetLayerCount(vk_image->GetDesc().type == ImageType::e2D ? vk_image->GetDesc().extent.depth : 1)
                                .SetMipmapCount(vk_image->GetDesc().mipmap_count),
                            image_layout,
                            vk_image->c_AttachmentImageLayout
                    );
                    SetImageLayout(vk_image, vk_image->c_AttachmentImageLayout);
                }
                if (vk_resolve_image != nullptr && (image_layout != vk_resolve_image->c_AttachmentImageLayout)) {
                    barriers.emplace_back(
                            //TODO:
                            Sync{}, 
                            Sync{vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite},
                            vk_resolve_image,
                            ImageSubresource{}
                                .SetAspectFlags(DnmRHI::AspectFlagBit::eColor)
                                .SetBaseLayer(0)
                                .SetBaseMipmap(0)
                                .SetLayerCount(vk_resolve_image->GetDesc().type == ImageType::e2D ? vk_resolve_image->GetDesc().extent.depth : 1)
                                .SetMipmapCount(vk_resolve_image->GetDesc().mipmap_count),
                            image_layout,
                            vk_resolve_image->c_AttachmentImageLayout
                    );
                    SetImageLayout(vk_resolve_image, vk_resolve_image->c_AttachmentImageLayout);
                }
            }

            if (desc.depth_stencil_attachment != nullptr) {
                vk::PipelineStageFlags2 stage_flags{};

                if (desc.flags.Has(DnmRHI::RenderPassFlagBit::eEarlyDepthStencil)) {
                    stage_flags |= vk::PipelineStageFlagBits2::eEarlyFragmentTests;
                }
                if (desc.flags.Has(DnmRHI::RenderPassFlagBit::eLateDepthStencil)) {
                    stage_flags |= vk::PipelineStageFlagBits2::eLateFragmentTests;
                }
                if (!desc.flags.Has(DnmRHI::RenderPassFlagBit::eLateDepthStencil) && !desc.flags.Has(DnmRHI::RenderPassFlagBit::eLateDepthStencil)) {
                    stage_flags |= vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
                }

                auto *vk_image = static_cast<Vk::Image *>(desc.depth_stencil_attachment->image);
                auto *vk_depth_resolve_image = static_cast<Vk::Image *>(desc.depth_stencil_attachment->resolve_image);
                auto *vk_stencil_resolve_image = static_cast<Vk::Image *>(desc.depth_stencil_attachment->stencil_resolve_image);

                const auto curr_depth_image_layout = GetImageLayout(vk_image);
                const auto curr_stencil_image_layout = GetImageLayout(vk_image, true);
                const auto curr_depth_resolve_image_layout = GetImageLayout(vk_depth_resolve_image);
                const auto curr_stencil_resolve_image_layout = GetImageLayout(vk_stencil_resolve_image, true);

                const auto depth_image_layout = read_only_depth ? vk_image->c_ReadOnlyAttachmentImageLayout : vk_image->c_AttachmentImageLayout;
                const auto stencil_image_layout = read_only_stencil ? vk_image->c_ReadOnlyAttachmentImageLayout : vk_image->c_AttachmentImageLayout;

                if (curr_depth_image_layout != depth_image_layout) {
                    barriers.emplace_back(
                            //TODO:
                            Sync{}, 
                            Sync{
                                stage_flags, 
                                read_only_depth ? vk::AccessFlagBits2::eDepthStencilAttachmentRead : vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite
                            },
                            vk_image,
                            ImageSubresource{}
                                .SetAspectFlags(DnmRHI::AspectFlagBit::eDepth)
                                .SetBaseLayer(0)
                                .SetBaseMipmap(0)
                                .SetLayerCount(vk_image->GetDesc().type == ImageType::e2D ? vk_image->GetDesc().extent.depth : 1)
                                .SetMipmapCount(vk_image->GetDesc().mipmap_count),
                            curr_depth_image_layout,
                            depth_image_layout
                    );
                    SetImageLayout(vk_image, depth_image_layout);
                }
                if (curr_stencil_image_layout != stencil_image_layout) {
                    barriers.emplace_back(
                            //TODO:
                            Sync{}, 
                            Sync{
                                stage_flags, 
                                read_only_stencil ? vk::AccessFlagBits2::eDepthStencilAttachmentRead : vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite
                            },
                            vk_image,
                            ImageSubresource{}
                                .SetAspectFlags(DnmRHI::AspectFlagBit::eStencil)
                                .SetBaseLayer(0)
                                .SetBaseMipmap(0)
                                .SetLayerCount(vk_image->GetDesc().type == ImageType::e2D ? vk_image->GetDesc().extent.depth : 1)
                                .SetMipmapCount(vk_image->GetDesc().mipmap_count),
                            curr_depth_image_layout,
                            stencil_image_layout
                    );
                    SetImageLayout(vk_image, stencil_image_layout, true);
                }
                if (curr_depth_resolve_image_layout != vk_depth_resolve_image->c_AttachmentImageLayout) {
                    barriers.emplace_back(
                            //TODO:
                            Sync{}, 
                            Sync{
                                stage_flags,
                                vk::AccessFlagBits2::eDepthStencilAttachmentWrite
                            },
                            vk_depth_resolve_image,
                            ImageSubresource{}
                                .SetAspectFlags(DnmRHI::AspectFlagBit::eDepth)
                                .SetBaseLayer(0)
                                .SetBaseMipmap(0)
                                .SetLayerCount(vk_depth_resolve_image->GetDesc().type == ImageType::e2D ? vk_depth_resolve_image->GetDesc().extent.depth : 1)
                                .SetMipmapCount(vk_depth_resolve_image->GetDesc().mipmap_count),
                            curr_depth_resolve_image_layout,
                            vk_depth_resolve_image->c_AttachmentImageLayout
                    );
                    SetImageLayout(vk_depth_resolve_image, vk_depth_resolve_image->c_AttachmentImageLayout);
                }
                if (curr_stencil_resolve_image_layout != vk_stencil_resolve_image->c_AttachmentImageLayout) {
                    barriers.emplace_back(
                            //TODO:
                            Sync{}, 
                            Sync{
                                stage_flags, 
                                vk::AccessFlagBits2::eDepthStencilAttachmentWrite
                            },
                            vk_stencil_resolve_image,
                            ImageSubresource{}
                                .SetAspectFlags(DnmRHI::AspectFlagBit::eStencil)
                                .SetBaseLayer(0)
                                .SetBaseMipmap(0)
                                .SetLayerCount(vk_stencil_resolve_image->GetDesc().type == ImageType::e2D ? vk_stencil_resolve_image->GetDesc().extent.depth : 1)
                                .SetMipmapCount(vk_stencil_resolve_image->GetDesc().mipmap_count),
                            curr_depth_image_layout,
                            vk_stencil_resolve_image->c_AttachmentImageLayout
                    );
                    SetImageLayout(vk_stencil_resolve_image, vk_stencil_resolve_image->c_AttachmentImageLayout, true);
                }
            }
        }

        std::vector<vk::RenderingAttachmentInfo> color_attachments;
        vk::RenderingAttachmentInfo depth_attachment;
        vk::RenderingAttachmentInfo stencil_attachment;

        for (const auto &attachment : desc.attachments) {
            auto *vk_image = static_cast<Vk::Image *>(attachment.image);
            auto *vk_resolve_image = static_cast<Vk::Image *>(attachment.resolve_image);
            color_attachments.emplace_back(
                vk_image->GetImageView(attachment.subresource),
                vk_image->c_AttachmentImageLayout,
                To_Vk(attachment.resolve_mode),
                vk_resolve_image ? vk_resolve_image->GetImageView(attachment.resolve_subresource) : vk::ImageView{},
                vk_resolve_image ? vk_resolve_image->c_AttachmentImageLayout : vk::ImageLayout{},
                To_Vk(attachment.load_op),
                To_Vk(attachment.store_op),
                vk::ClearValue{{
                    attachment.clear_value[0], 
                    attachment.clear_value[1], 
                    attachment.clear_value[2],
                    attachment.clear_value[3]}}
            );
        }

        if (desc.depth_stencil_attachment != nullptr) {
            assert("unimplemented yet" && false);
            // Warn("depth stencil attachment is not supported yet");
            auto *vk_image = static_cast<Vk::Image *>(desc.depth_stencil_attachment->image);
            auto *vk_resolve_image = static_cast<Vk::Image *>(desc.depth_stencil_attachment->resolve_image);
            auto *vk_stencil_resolve_image = static_cast<Vk::Image *>(desc.depth_stencil_attachment->stencil_resolve_image);

            depth_attachment = vk::RenderingAttachmentInfo{
                vk_image->GetImageView(desc.depth_stencil_attachment->subresource),
                read_only_depth ? vk_image->c_ReadOnlyAttachmentImageLayout : vk_image->c_AttachmentImageLayout,
                To_Vk(desc.depth_stencil_attachment->resolve_mode),
                vk_resolve_image->GetImageView(desc.depth_stencil_attachment->resolve_subresource),
                vk_resolve_image->c_AttachmentImageLayout,
                To_Vk(desc.depth_stencil_attachment->load_op),
                To_Vk(desc.depth_stencil_attachment->store_op),
                vk::ClearValue{}.setDepthStencil(desc.depth_stencil_attachment->depth_clear_value)
            };

            stencil_attachment = vk::RenderingAttachmentInfo{
                vk_image->GetImageView(desc.depth_stencil_attachment->subresource),
                read_only_stencil ? vk_image->c_ReadOnlyAttachmentImageLayout : vk_image->c_AttachmentImageLayout,
                To_Vk(desc.depth_stencil_attachment->stencil_resolve_mode),
                vk_stencil_resolve_image->GetImageView(desc.depth_stencil_attachment->stencil_resolve_subresource),
                vk_stencil_resolve_image->c_AttachmentImageLayout,
                To_Vk(desc.depth_stencil_attachment->stencil_load_op),
                To_Vk(desc.depth_stencil_attachment->stencil_store_op),
                vk::ClearValue{}.setDepthStencil(desc.depth_stencil_attachment->stencil_clear_value)
            };
        }

        vk::Rect2D render_area;

        {
            Extent3D extent;
            u8 mipmap;

            if (!desc.attachments.empty()) {
                extent = desc.attachments[0].image->GetDesc().extent;
                mipmap = desc.attachments[0].subresource.base_layer;
            }
            else {
                extent = desc.depth_stencil_attachment->image->GetDesc().extent;
                mipmap = desc.depth_stencil_attachment->subresource.base_layer;
            }

            render_area = vk::Rect2D{
                vk::Offset2D{0, 0},
                vk::Extent2D{static_cast<uint32_t>(extent.width >> mipmap), static_cast<uint32_t>(extent.height >> mipmap)}
            };
        }

        m_command_buffer.beginRendering(
            vk::RenderingInfo{
                // TODO: flags
                vk::RenderingFlagBits{},
                render_area,
                desc.layer_count,
                0,
                static_cast<uint32_t>(color_attachments.size()),
                color_attachments.data(),
                &depth_attachment,
                &stencil_attachment
            }
        );
    }

    void CommandBuffer::ISetViewport(std::span<const Viewport> viewport) noexcept {
        std::vector<vk::Viewport> vk_viewports;
        vk_viewports.reserve(viewport.size());
        for (const auto &viewport : viewport)
            vk_viewports.emplace_back(
                viewport.x,
                viewport.height + viewport.y,
                viewport.width,
                -viewport.height,
                viewport.min_depth,
                viewport.max_depth
            );

        m_command_buffer.setViewport(0, vk_viewports);
    }
    void CommandBuffer::ISetScissor(std::span<const Scissor> scissor) noexcept {
        std::vector<vk::Rect2D> vk_scissors;
        vk_scissors.reserve(scissor.size());
        for (const auto &scissor : scissor)
            vk_scissors.emplace_back(VkRect2D{
                vk::Offset2D{scissor.x, scissor.y},
                vk::Extent2D{scissor.width, scissor.height}
            });

        m_command_buffer.setScissor(0, vk_scissors);
    }
    void CommandBuffer::ISetStencilRef(u32 front, u32 back) noexcept {
        m_command_buffer.setStencilReference(vk::StencilFaceFlagBits::eFront, front);
        m_command_buffer.setStencilReference(vk::StencilFaceFlagBits::eBack, back);
    }

    void CommandBuffer::IDraw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) noexcept {
        m_command_buffer.draw(
            vertex_count,
            instance_count,
            first_vertex,
            first_instance
        );
    }
    void CommandBuffer::IDrawIndexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) noexcept {
        m_command_buffer.drawIndexed(
            index_count,
            instance_count,
            first_index,
            vertex_offset,
            first_instance
        );
    }
    void CommandBuffer::IDrawIndirect(const DrawIndirectDesc &desc) noexcept {
        const auto indirect_buffer = static_cast<const Buffer *>(desc.indirect_buffer)->GetBuffer();
        if (desc.count_buffer == nullptr)
            m_command_buffer.drawIndirect(
                indirect_buffer, 
                desc.indirect_offset, 
                desc.max_draw_or_draw_count,
                desc.command_stride);
        else
            m_command_buffer.drawIndirectCount(
                indirect_buffer,
                desc.indirect_offset,
                static_cast<const Buffer *>(desc.count_buffer)->GetBuffer(),
                desc.count_offset,
                desc.max_draw_or_draw_count,
                desc.command_stride
            );
    }
    void CommandBuffer::IDrawIndexedIndirect(const DrawIndirectDesc &desc) noexcept {
        const auto indirect_buffer = static_cast<const Buffer *>(desc.indirect_buffer)->GetBuffer();
        if (desc.count_buffer == nullptr)
            m_command_buffer.drawIndexedIndirect(
                indirect_buffer, 
                desc.indirect_offset, 
                desc.max_draw_or_draw_count,
                desc.command_stride);
        else
            m_command_buffer.drawIndexedIndirectCount(
                indirect_buffer,
                desc.indirect_offset,
                static_cast<const Buffer *>(desc.count_buffer)->GetBuffer(),
                desc.count_offset,
                desc.max_draw_or_draw_count,
                desc.command_stride
            );
    }

    void CommandBuffer::IEndRenderPass() noexcept {
        m_command_buffer.endRendering();
    }
    
    void CommandBuffer::IBlitImage(const BlitImageDesc &desc) noexcept {
        const auto *vk_src_image = static_cast<const Image *>(desc.src_image);
        const auto *vk_dst_image = static_cast<const Image *>(desc.dst_image);
        const auto src_subresource = To_Vk(desc.src_subresource);
        const auto dst_subresource = To_Vk(desc.dst_subresource);

        if (!GetVkDevice.GetVulkanDeviceFeatures().unified_image_layouts) {
            std::array<ImageBarrierDesc, 2> barrier;

            const auto src_layout = GetImageLayout(vk_src_image);
            const auto dst_layout = GetImageLayout(vk_dst_image);

            const b8 src_need_layout_translation = vk_src_image->c_CopySrcImageLayout != src_layout;
            const b8 dst_need_layout_translation = vk_dst_image->c_CopyDstImageLayout != dst_layout;

            if (src_need_layout_translation) {
                barrier[0] = ImageBarrierDesc{
                        //TODO:
                        Sync{}, 
                        Sync{vk::PipelineStageFlagBits2::eBlit, vk::AccessFlagBits2::eTransferRead},
                        vk_src_image,
                        ImageSubresource{}
                            .SetAspectFlags(desc.src_subresource.aspect_flags)
                            .SetBaseLayer(0)
                            .SetBaseMipmap(0)
                            .SetLayerCount(vk_src_image->GetDesc().type == ImageType::e2D ? vk_src_image->GetDesc().extent.depth : 1)
                            .SetMipmapCount(vk_src_image->GetDesc().mipmap_count),
                        src_layout,
                        vk_src_image->c_CopySrcImageLayout
                    };
            }
            if (dst_need_layout_translation) {
                barrier[src_need_layout_translation] = ImageBarrierDesc{
                        //TODO:
                        Sync{}, 
                        Sync{vk::PipelineStageFlagBits2::eBlit, vk::AccessFlagBits2::eTransferWrite},
                        vk_dst_image,
                        ImageSubresource{}
                            .SetAspectFlags(desc.dst_subresource.aspect_flags)
                            .SetBaseLayer(0)
                            .SetBaseMipmap(0)
                            .SetLayerCount(vk_dst_image->GetDesc().type == ImageType::e2D ? vk_dst_image->GetDesc().extent.depth : 1)
                            .SetMipmapCount(vk_dst_image->GetDesc().mipmap_count),
                        dst_layout,
                        vk_dst_image->c_CopyDstImageLayout
                    };
            }

            InternalBarrier({}, 
                {barrier.data(), u32(src_need_layout_translation) + dst_need_layout_translation});

            SetImageLayout(vk_src_image, vk_src_image->c_CopySrcImageLayout);
            SetImageLayout(vk_dst_image, vk_dst_image->c_CopyDstImageLayout);
        }

        const auto blit =
            vk::ImageBlit{}
                .setSrcSubresource(src_subresource)
                .setDstSubresource(dst_subresource)
                .setSrcOffsets(std::array<vk::Offset3D, 2>{
                   vk::Offset3D{desc.src_offset[0].x, desc.src_offset[0].y, desc.src_offset[0].z}, 
                    vk::Offset3D{desc.src_offset[1].x, desc.src_offset[1].y, desc.src_offset[1].z}
                })
                .setDstOffsets(std::array<vk::Offset3D, 2>{
                    vk::Offset3D{desc.dst_offset[0].x, desc.dst_offset[0].y, desc.dst_offset[0].z}, 
                    vk::Offset3D{desc.dst_offset[1].x, desc.dst_offset[1].y, desc.dst_offset[1].z}})
                ;

        m_command_buffer.blitImage(
            vk_src_image->GetImage(), vk_src_image->c_CopySrcImageLayout, 
            vk_dst_image->GetImage(), vk_dst_image->c_CopyDstImageLayout,
            blit,
            To_Vk(desc.filter)
        );
    }
    // TODO:
    void CommandBuffer::IResolveImage(const ResolveImageDesc &desc) noexcept {
        assert(false && "vulkan IResolveImage not implamented");
    }
    
    void CommandBuffer::IBindIndexBuffer(DnmRHI::Buffer &buffer, u64 offset, IndexType index_type) noexcept {
        m_command_buffer.bindIndexBuffer(   
            static_cast<const Buffer &>(buffer).GetBuffer(),
            offset,
            To_Vk(index_type)
        );
    }
    void CommandBuffer::IBindPipeline(DnmRHI::GraphicsPipeline &pipeline) noexcept {
        m_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, static_cast<const Vk::GraphicsPipeline &>(pipeline).GetPipeline());
    }
    void CommandBuffer::IPrepareForPresent(AccessFlags before_access, DnmRHI::Image &swapchain_image) noexcept {
        const auto *vk_image_ptr = static_cast<const Vk::Image *>(&swapchain_image);
        if (!m_image_layouts.contains(vk_image_ptr)) return;

        const ImageBarrierDesc barrier_desc {
                To_Vk(before_access),
                Sync{},
                vk_image_ptr,
                ImageSubresource{}.SetLayerCount(0).SetMipmapCount(0),
                m_image_layouts[vk_image_ptr][0],
                vk::ImageLayout::ePresentSrcKHR
            };

        InternalBarrier({}, {&barrier_desc, 1});
        m_used_swapchain_images.emplace_back(vk_image_ptr->GetSwapchain(), vk_image_ptr->GetSwapchainImageIndex());
    }
    // General Queue Commands
    
    // Compute Queue Commands
    void CommandBuffer::IDispatch(u32 x, u32 y, u32 z) noexcept {
        m_command_buffer.dispatch(
            x, y, z
        );
    }
    void CommandBuffer::IDispatchIndirect(DnmRHI::Buffer &buffer, u32 offset) noexcept {
        m_command_buffer.dispatchIndirect(
            static_cast<const Buffer &>(buffer).GetBuffer(),
            offset
        );
    }

    void CommandBuffer::IBeginConditionalRendering(DnmRHI::Buffer &buffer, u32 offset, bool is_inverted) noexcept {
        m_command_buffer.beginConditionalRenderingEXT(
            vk::ConditionalRenderingBeginInfoEXT{}
                .setBuffer(static_cast<const Buffer &>(buffer).GetBuffer())
                .setOffset(offset)
                .setFlags(is_inverted ? vk::ConditionalRenderingFlagBitsEXT::eInverted : vk::ConditionalRenderingFlagBitsEXT{})
        );
    }
    void CommandBuffer::IEndConditionalRendering() noexcept {
        m_command_buffer.endConditionalRenderingEXT();
    }

    void CommandBuffer::IPushConstant(const void *data, u32 size, u32 offset) noexcept {
        GetVkDevice.GetDescriptorManager()->PushConstants(m_command_buffer, data, size, offset);
    }
    void CommandBuffer::IBindPipeline(DnmRHI::ComputePipeline &pipeline) noexcept {
        m_command_buffer.bindPipeline(vk::PipelineBindPoint::eCompute, static_cast<const Vk::ComputePipeline &>(pipeline).GetPipeline());
    }
    // Compute Queue Commands

    // Copy Queue Commands
    void CommandBuffer::ICopyBuffer(const BufferCopyDesc &desc) noexcept {
        const auto copy_region
            = vk::BufferCopy{}
                .setSize(desc.size)
                .setDstOffset(desc.dst_offset)
                .setSrcOffset(desc.src_offset);

        m_command_buffer.copyBuffer(
            static_cast<const Buffer *>(desc.src_buffer)->GetBuffer(),
            static_cast<const Buffer *>(desc.dst_buffer)->GetBuffer(),
            copy_region
        );
    }
    void CommandBuffer::ICopyImage(const ImageCopyDesc &desc) noexcept {
        const auto *vk_src_image = static_cast<const Image *>(desc.src_image);
        const auto *vk_dst_image = static_cast<const Image *>(desc.dst_image);
        const auto src_subresource = To_Vk(desc.src_subresource);
        const auto dst_subresource = To_Vk(desc.dst_subresource);

        if (!GetVkDevice.GetVulkanDeviceFeatures().unified_image_layouts) {
            std::array<ImageBarrierDesc, 2> barrier;

            const auto src_layout = GetImageLayout(vk_src_image);
            const auto dst_layout = GetImageLayout(vk_dst_image);

            const b8 src_need_layout_translation = vk_src_image->c_CopySrcImageLayout != src_layout;
            const b8 dst_need_layout_translation = vk_dst_image->c_CopyDstImageLayout != dst_layout;

            if (src_need_layout_translation) {
                barrier[0] = ImageBarrierDesc{
                        //TODO:
                        Sync{}, 
                        Sync{vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferRead},
                        vk_src_image,
                        ImageSubresource{}
                            .SetAspectFlags(desc.src_subresource.aspect_flags)
                            .SetBaseLayer(0)
                            .SetBaseMipmap(0)
                            .SetLayerCount(vk_src_image->GetDesc().type == ImageType::e2D ? vk_src_image->GetDesc().extent.depth : 1)
                            .SetMipmapCount(vk_src_image->GetDesc().mipmap_count),
                        src_layout,
                        vk_src_image->c_CopySrcImageLayout
                    };
            }
            if (dst_need_layout_translation) {
                barrier[src_need_layout_translation] = ImageBarrierDesc{
                        //TODO:
                        Sync{}, 
                        Sync{vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite},
                        vk_dst_image,
                        ImageSubresource{}
                            .SetAspectFlags(desc.dst_subresource.aspect_flags)
                            .SetBaseLayer(0)
                            .SetBaseMipmap(0)
                            .SetLayerCount(vk_dst_image->GetDesc().type == ImageType::e2D ? vk_dst_image->GetDesc().extent.depth : 1)
                            .SetMipmapCount(vk_dst_image->GetDesc().mipmap_count),
                        dst_layout,
                        vk_dst_image->c_CopyDstImageLayout
                    };
            }

            InternalBarrier({}, 
                {barrier.data(), u32(src_need_layout_translation) + dst_need_layout_translation});

            SetImageLayout(vk_src_image, vk_src_image->c_CopySrcImageLayout);
            SetImageLayout(vk_dst_image, vk_dst_image->c_CopyDstImageLayout);
        }

        const auto copy =
            vk::ImageCopy{}
                .setSrcSubresource(src_subresource)
                .setDstSubresource(dst_subresource)
                .setSrcOffset({desc.src_offset.x, desc.src_offset.y, desc.src_offset.z})
                .setDstOffset({desc.dst_offset.x, desc.dst_offset.y, desc.dst_offset.z})
                .setExtent({desc.extent.width, desc.extent.height, desc.extent.depth})
                ;

        m_command_buffer.copyImage(
            vk_src_image->GetImage(), vk_src_image->c_CopySrcImageLayout, 
            vk_dst_image->GetImage(), vk_dst_image->c_CopyDstImageLayout,
            copy
        );
    }
    void CommandBuffer::ICopyBufferToImage(const BufferImageCopyDesc &desc) noexcept {
        const auto *vk_buffer = static_cast<const Buffer *>(desc.buffer);
        const auto *vk_image = static_cast<const Image *>(desc.image);
        const auto image_subresource = To_Vk(desc.subresource);

        if (!GetVkDevice.GetVulkanDeviceFeatures().unified_image_layouts) {
            const auto image_layout = GetImageLayout(vk_image);

            const b8 image_need_layout_translation = vk_image->c_CopyDstImageLayout != image_layout;

            if (image_need_layout_translation) {
                const auto barrier = ImageBarrierDesc{
                        //TODO:
                        Sync{}, 
                        Sync{vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite},
                        vk_image,
                        ImageSubresource{}
                            .SetAspectFlags(desc.subresource.aspect_flags)
                            .SetBaseLayer(0)
                            .SetBaseMipmap(0)
                            .SetLayerCount(vk_image->GetDesc().type == ImageType::e2D ? vk_image->GetDesc().extent.depth : 1)
                            .SetMipmapCount(vk_image->GetDesc().mipmap_count),
                        image_layout,
                        vk_image->c_CopySrcImageLayout
                    };

                InternalBarrier({}, 
                    {&barrier, 1});
                SetImageLayout(vk_image, vk_image->c_CopyDstImageLayout);
            }
        }

        const auto copy =
            vk::BufferImageCopy{}
                .setImageSubresource(image_subresource)
                .setImageOffset({desc.image_offset.x, desc.image_offset.y, desc.image_offset.z})
                .setBufferOffset(desc.buffer_offset)
                .setImageExtent({desc.extent.width, desc.extent.height, desc.extent.depth})
                ;

        m_command_buffer.copyBufferToImage(
            vk_buffer->GetBuffer(),
            vk_image->GetImage(), vk_image->c_CopyDstImageLayout,
            copy
        );
    }
    void CommandBuffer::ICopyImageToBuffer(const BufferImageCopyDesc &desc) noexcept {
        const auto *vk_buffer = static_cast<const Buffer *>(desc.buffer);
        const auto *vk_image = static_cast<const Image *>(desc.image);
        const auto image_subresource = To_Vk(desc.subresource);

        if (!GetVkDevice.GetVulkanDeviceFeatures().unified_image_layouts) {
            const auto image_layout = GetImageLayout(vk_image);

            const b8 image_need_layout_translation = vk_image->c_CopySrcImageLayout != image_layout;

            if (image_need_layout_translation) {
                const auto barrier = ImageBarrierDesc{
                        //TODO:
                        Sync{}, 
                        Sync{vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferRead},
                        vk_image,
                        ImageSubresource{}
                            .SetAspectFlags(desc.subresource.aspect_flags)
                            .SetBaseLayer(0)
                            .SetBaseMipmap(0)
                            .SetLayerCount(vk_image->GetDesc().type == ImageType::e2D ? vk_image->GetDesc().extent.depth : 1)
                            .SetMipmapCount(vk_image->GetDesc().mipmap_count),
                        image_layout,
                        vk_image->c_CopySrcImageLayout
                    };

                InternalBarrier({}, 
                    {&barrier, 1});
                SetImageLayout(vk_image, vk_image->c_CopySrcImageLayout);
            }
        }

        const auto copy =
            vk::BufferImageCopy{}
                .setImageSubresource(image_subresource)
                .setImageOffset({desc.image_offset.x, desc.image_offset.y, desc.image_offset.z})
                .setBufferOffset(desc.buffer_offset)
                .setImageExtent({desc.extent.width, desc.extent.height, desc.extent.depth})
                ;

        m_command_buffer.copyImageToBuffer(
            vk_image->GetImage(), vk_image->c_CopySrcImageLayout,
            vk_buffer->GetBuffer(),
            copy
        );
    }
    // Copy Queue Commands

    void CommandBuffer::IBarrier(std::span<const BarrierDesc> barriers) noexcept {
        std::vector<ImageBarrierDesc> image_barriers;
        std::vector<MemoryBarrierDesc> memory_barriers;
        image_barriers.reserve(barriers.size());
        memory_barriers.reserve(barriers.size());

        for (const auto barrier : barriers) {
            const auto *resource = barrier.resource;

            if (barrier.src_access.None() && barrier.dst_access.None()) continue;

            const auto src_barrier_is_read_only = !b8(barrier.src_access & DnmRHI::AccessFlagBit::eAllWrite);
            const auto src_barrier_is_write_only = !b8(barrier.src_access & DnmRHI::AccessFlagBit::eAllRead);
            const auto dst_barrier_is_read_only = !b8(barrier.dst_access & DnmRHI::AccessFlagBit::eAllWrite);
            const auto dst_barrier_is_write_only = !b8(barrier.dst_access & DnmRHI::AccessFlagBit::eAllRead);

            if (src_barrier_is_read_only && dst_barrier_is_read_only)
                continue;

            const b8 is_image = !resource ? false : resource->c_ResourceType == ResourceType::eImage;

            auto src_barrier = To_Vk(barrier.src_access);
            auto dst_barrier = To_Vk(barrier.dst_access);

            if (src_barrier_is_read_only && dst_barrier_is_write_only) {
                src_barrier.access = {};
                dst_barrier.access = {};
            }

            if (resource && resource->c_ResourceType == ResourceType::eImage) {
                const auto *vk_image = static_cast<const Image *>(resource);
                image_barriers.emplace_back(
                    src_barrier,
                    dst_barrier,
                    vk_image,
                    static_cast<ImageSubresource>(barrier.subresource),
                    vk_image->c_DefaultLayout,
                    vk_image->c_DefaultLayout
                );
                continue;
            }
            memory_barriers.emplace_back(
                src_barrier,
                dst_barrier
            );
        }

        InternalBarrier(memory_barriers, image_barriers);
    }

    void CommandBuffer::IInitImage(std::span<const InitImageDesc> descs) noexcept {
        std::vector<ImageBarrierDesc> image_barriers;
        image_barriers.reserve(descs.size());
        
        for (const auto &desc : descs) {
            // TODO: select layout from desc.dst_access_flags
            const auto &vk_image = static_cast<const Vk::Image &>(*desc.image);

            image_barriers.emplace_back(
                Sync{},
                To_Vk(desc.dst_access_flags),
                static_cast<Vk::Image *>(desc.image),
                ImageSubresource{}.SetLayerCount(0).SetMipmapCount(0),
                vk_image.c_IsSwapchainImage ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eUndefined,
                vk_image.c_DefaultLayout
            );

            if (vk_image.c_IsSwapchainImage) m_image_layouts[&vk_image][0] = vk_image.c_DefaultLayout;
        }

        InternalBarrier({}, image_barriers);
    }

    void CommandBuffer::IBeginDebugMarker(std::string_view name, std::array<u8, 4> color) noexcept {
        m_command_buffer.beginDebugUtilsLabelEXT(
            vk::DebugUtilsLabelEXT{}
                .setPLabelName(name.data())
                .setColor({
                    static_cast<float>(color[0]), 
                    static_cast<float>(color[1]), 
                    static_cast<float>(color[2]), 
                    static_cast<float>(color[3])})
        );
    }
    void CommandBuffer::IEndDebugMarker() noexcept {
        m_command_buffer.endDebugUtilsLabelEXT();
    }

    void CommandBuffer::InternalBarrier(std::span<const MemoryBarrierDesc> memory_barriers, std::span<const ImageBarrierDesc> image_barriers) {
        std::vector<vk::ImageMemoryBarrier2> vk_image_barriers;
        std::vector<vk::MemoryBarrier2> vk_memory_barriers;
        vk_memory_barriers.reserve(memory_barriers.size());
        vk_image_barriers.reserve(image_barriers.size());

        for (const auto barrier : memory_barriers) {
            vk_memory_barriers.emplace_back(
                barrier.src_sync.stage,
                barrier.src_sync.access,
                barrier.dst_sync.stage,
                barrier.dst_sync.access
            );
        }

        for (const auto &barrier : image_barriers) {
            vk_image_barriers.emplace_back(
                barrier.src_sync.stage,
                barrier.src_sync.access,
                barrier.dst_sync.stage,
                barrier.dst_sync.access,
                barrier.old_layout,
                barrier.new_layout,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                barrier.image->GetImage(),
                To_Vk(barrier.subresource)
            );
        }

        m_command_buffer.pipelineBarrier2(
            vk::DependencyInfo{}
                .setMemoryBarriers(vk_memory_barriers)
                .setImageMemoryBarriers(vk_image_barriers)
        );
    }
}