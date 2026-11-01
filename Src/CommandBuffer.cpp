#include <DnmRHI/CommandBuffer/CommandBuffer.hpp>

namespace DnmRHI {
    Result CommandBuffer::Begin(bool one_time_submit) noexcept {
        if (m_open) {
            return Result::eInvalidUsage;
        }
        
        m_open = true;
        return IBegin(one_time_submit);
    }

    Result CommandBuffer::End() noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        m_open = false;
        return IEnd();
    }
    
    Result CommandBuffer::Reset() noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        return IReset();
    }

    // General Queue Commands
    Result CommandBuffer::BeginRenderPass(const RenderPassDesc &desc) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_in_render_pass) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        m_in_render_pass = true;
        return IBeginRenderPass(desc);
    }

    Result CommandBuffer::SetViewport(std::span<const Viewport> viewports) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        return ISetViewport(viewports);
    }
    Result CommandBuffer::SetScissor(std::span<const Scissor> scissor) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        return ISetScissor(scissor);
    }
    Result CommandBuffer::SetStencilRef(u32 front, u32 back) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        return ISetStencilRef(front, back);
    }

    Result CommandBuffer::Draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (!m_in_render_pass) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        if (!m_bound_compute_pipeline) {
            return Result::eInvalidUsage;
        }

        return IDraw(vertex_count, instance_count, first_vertex, first_instance);
    }
    Result CommandBuffer::DrawIndexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (!m_in_render_pass) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        if (!m_bound_compute_pipeline) {
            return Result::eInvalidUsage;
        }

        return IDrawIndexed(index_count, instance_count, first_index, vertex_offset, first_instance);
    }
    Result CommandBuffer::DrawIndirect(const DrawIndirectDesc &desc) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (!m_in_render_pass) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        if (!m_bound_compute_pipeline) {
            return Result::eInvalidUsage;
        }

        return IDrawIndirect(desc);
    }
    Result CommandBuffer::DrawIndexedIndirect(const DrawIndirectDesc &desc) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (!m_in_render_pass) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        if (!m_bound_compute_pipeline) {
            return Result::eInvalidUsage;
        }

        return IDrawIndexedIndirect(desc);
    }

    Result CommandBuffer::EndRenderPass() noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (!m_in_render_pass) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        m_in_render_pass = false;
        return IEndRenderPass();
    }
    
    Result CommandBuffer::BlitImage(const BlitImageDesc &desc) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        return IBlitImage(desc);
    }
    Result CommandBuffer::ResolveImage(const ResolveImageDesc &desc) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        return IResolveImage(desc);
    }
    
    Result CommandBuffer::BindIndexBuffer(Buffer &buffer, u64 offset, IndexType index_type) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        return IBindIndexBuffer(buffer, offset, index_type);
    }
    Result CommandBuffer::BindPipeline(GraphicsPipeline &pipeline) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        m_bound_graphics_pipeline = &pipeline;
        return IBindPipeline(pipeline);
    }

    Result CommandBuffer::PrepareForPresent(AccessFlags before_access, Image &swapchain_image) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type != QueueType::eGeneral) {
            return Result::eInvalidUsage;
        }

        if (m_in_render_pass) {
            return Result::eInvalidUsage;
        }

        return IPrepareForPresent(before_access, swapchain_image);
    }
    // General Queue Commands
    
    // Compute Queue Commands
    Result CommandBuffer::Dispatch(u32 x, u32 y, u32 z) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type > QueueType::eCompute) {
            return Result::eInvalidUsage;
        }

        if (!m_bound_compute_pipeline) {
            return Result::eInvalidUsage;
        }

        return IDispatch(x, y, z);
    }
    Result CommandBuffer::DispatchIndirect(Buffer &buffer, u32 offset) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type > QueueType::eCompute) {
            return Result::eInvalidUsage;
        }

        if (!m_bound_compute_pipeline) {
            return Result::eInvalidUsage;
        }

        return IDispatchIndirect(buffer, offset);
    }

    Result CommandBuffer::BeginConditionalRendering(Buffer &buffer, u32 offset, bool is_inverted) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type > QueueType::eCompute) {
            return Result::eInvalidUsage;
        }

        m_in_condition = true;
        return IBeginConditionalRendering(buffer, offset, is_inverted);
    }
    Result CommandBuffer::EndConditionalRendering() noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type > QueueType::eCompute) {
            return Result::eInvalidUsage;
        }

        m_in_condition = false;
        return IEndConditionalRendering();
    }

    Result CommandBuffer::PushConstant(const void *data, u32 size, u32 offset) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type > QueueType::eCompute) {
            return Result::eInvalidUsage;
        }

        if (size + offset > c_Push_Constant_Size) {
            return Result::eInvalidParameter;
        }

        return IPushConstant(data, size, offset);
    }
    Result CommandBuffer::BindPipeline(ComputePipeline &pipeline) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_desc.queue_type > QueueType::eCompute) {
            return Result::eInvalidUsage;
        }

        m_bound_compute_pipeline = &pipeline;
        return IBindPipeline(pipeline);
    }
    // Compute Queue Commands

    // Copy Queue Commands
    Result CommandBuffer::CopyBuffer(const BufferCopyDesc &desc) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        return ICopyBuffer(desc);
    }
    Result CommandBuffer::CopyImage(const ImageCopyDesc &desc) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        return ICopyImage(desc);
    }
    Result CommandBuffer::CopyBufferToImage(const BufferImageCopyDesc &desc) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        return ICopyBufferToImage(desc);
    }
    Result CommandBuffer::CopyImageToBuffer(const BufferImageCopyDesc &desc) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        return ICopyImageToBuffer(desc);
    }
    // Copy Queue Commands

    Result CommandBuffer::Barrier(std::span<const BarrierDesc> barriers) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_in_render_pass) {
            return Result::eInvalidUsage;
        }

        return IBarrier(barriers);
    }

    Result CommandBuffer::InitImage(std::span<const InitImageDesc> descs) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_in_render_pass) {
            return Result::eInvalidUsage;
        }

        return IInitImage(descs);
    }

    Result CommandBuffer::BeginDebugMarker(std::string_view name, std::array<u8, 4> color) noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (m_in_debug_marker) {
            return Result::eInvalidUsage;
        }

        m_in_debug_marker = true;
        return IBeginDebugMarker(name, color);
    }
    Result CommandBuffer::EndDebugMarker() noexcept {
        if (!m_open) {
            return Result::eInvalidUsage;
        }

        if (!m_in_debug_marker) {
            return Result::eInvalidUsage;
        }

        m_in_debug_marker = false;
        return IEndDebugMarker();
    }
}