#include <DnmRHI/CommandBuffer/CommandBuffer.hpp>

namespace DnmRHI {
    void CommandBuffer::Begin(bool one_time_submit) noexcept {
        // TODO: check somethings
        
        IBegin(one_time_submit);
        m_open = true;
    }

    void CommandBuffer::End() noexcept {
        // TODO: check somethings

        IEnd();
        m_open = false;
    }
    
    void CommandBuffer::Reset() noexcept {
        // TODO: check somethings

        IReset();
    }

    // General Queue Commands
    void CommandBuffer::BeginRenderPass(const RenderPassDesc &desc) noexcept {
        // TODO: check somethings

        IBeginRenderPass(desc);
        m_in_render_pass = true;
    }

    void CommandBuffer::SetViewport(std::span<const Viewport> viewports) noexcept {
        // TODO: check somethings

        ISetViewport(viewports);
    }
    void CommandBuffer::SetScissor(std::span<const Scissor> scissor) noexcept {
        // TODO: check somethings

        ISetScissor(scissor);
    }
    void CommandBuffer::SetStencilRef(u32 front, u32 back) noexcept {
        // TODO: check somethings

        ISetStencilRef(front, back);
    }

    void CommandBuffer::Draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) noexcept {
        // TODO: check somethings

        IDraw(vertex_count, instance_count, first_vertex, first_instance);
    }
    void CommandBuffer::DrawIndexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) noexcept {
        // TODO: check somethings

        IDrawIndexed(index_count, instance_count, first_index, vertex_offset, first_instance);
    }
    void CommandBuffer::DrawIndirect(const DrawIndirectDesc &desc) noexcept {
        // TODO: check somethings

        IDrawIndirect(desc);
    }
    void CommandBuffer::DrawIndexedIndirect(const DrawIndirectDesc &desc) noexcept {
        // TODO: check somethings

        IDrawIndexedIndirect(desc);
    }

    void CommandBuffer::EndRenderPass() noexcept {
        // TODO: check somethings

        IEndRenderPass();
        m_in_render_pass = false;
    }
    
    void CommandBuffer::BlitImage(const BlitImageDesc &desc) noexcept {
        // TODO: check somethings

        IBlitImage(desc);
    }
    void CommandBuffer::ResolveImage(const ResolveImageDesc &desc) noexcept {
        // TODO: check somethings

        IResolveImage(desc);
    }
    
    void CommandBuffer::BindIndexBuffer(Buffer &buffer, u64 offset, IndexType index_type) noexcept {
        // TODO: check somethings

        IBindIndexBuffer(buffer, offset, index_type);
    }
    void CommandBuffer::BindPipeline(GraphicsPipeline &pipeline) noexcept {
        // TODO: check somethings
        
        IBindPipeline(pipeline);
    }

    void CommandBuffer::PrepareForPresent(AccessFlags before_access, Image &swapchain_image) noexcept {
        // TODO: check somethings

        IPrepareForPresent(before_access, swapchain_image);
    }
    // General Queue Commands
    
    // Compute Queue Commands
    void CommandBuffer::Dispatch(u32 x, u32 y, u32 z) noexcept {
        // TODO: check somethings
        
        IDispatch(x, y, z);
    }
    void CommandBuffer::DispatchIndirect(Buffer &buffer, u32 offset) noexcept {
        // TODO: check somethings

        IDispatchIndirect(buffer, offset);
    }

    void CommandBuffer::BeginConditionalRendering(Buffer &buffer, u32 offset, bool is_inverted) noexcept {
        // TODO: check somethings

        IBeginConditionalRendering(buffer, offset, is_inverted);
        m_in_condition = true;
    }
    void CommandBuffer::EndConditionalRendering() noexcept {
        // TODO: check somethings

        IEndConditionalRendering();
        m_in_condition = false;
    }

    void CommandBuffer::PushConstant(const void *data, u32 size, u32 offset) noexcept {
        // TODO: check somethings

        IPushConstant(data, size, offset);
    }
    void CommandBuffer::BindPipeline(ComputePipeline &pipeline) noexcept {
        // TODO: check somethings

        IBindPipeline(pipeline);
    }
    // Compute Queue Commands

    // Copy Queue Commands
    void CommandBuffer::CopyBuffer(const BufferCopyDesc &desc) noexcept {
        // TODO: check somethings

        ICopyBuffer(desc);
    }
    void CommandBuffer::CopyImage(const ImageCopyDesc &desc) noexcept {
        // TODO: check somethings

        ICopyImage(desc);
    }
    void CommandBuffer::CopyBufferToImage(const BufferImageCopyDesc &desc) noexcept {
        // TODO: check somethings

        ICopyBufferToImage(desc);
    }
    void CommandBuffer::CopyImageToBuffer(const BufferImageCopyDesc &desc) noexcept {
        // TODO: check somethings

        ICopyImageToBuffer(desc);
    }
    // Copy Queue Commands

    void CommandBuffer::Barrier(std::span<const BarrierDesc> barriers) noexcept {
        // TODO: check somethings

        IBarrier(barriers);
    }

    void CommandBuffer::InitImage(std::span<const InitImageDesc> descs) noexcept {
        // TODO: check somethings

        IInitImage(descs);
    }

    void CommandBuffer::BeginDebugMarker(std::string_view name, std::array<u8, 4> color) noexcept {
        // TODO: check somethings

        IBeginDebugMarker(name, color);
        m_in_debug_marker = true;
    }
    void CommandBuffer::EndDebugMarker() noexcept {
        // TODO: check somethings

        IEndDebugMarker();
        m_in_debug_marker = false;
    }
}