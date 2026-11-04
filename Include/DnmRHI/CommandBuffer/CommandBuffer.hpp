#pragma once

#include <DnmRHI/Common/Subresource.hpp>
#include <DnmRHI/Common/FilterType.hpp>
#include <DnmRHI/Common/ResolveModes.hpp>
#include <DnmRHI/Image/SubresourceLayers.hpp>
#include <DnmRHI/CommandBuffer/Desc.hpp>
#include <initializer_list>
#include <string_view>
#include <array>
#include <span>
#include <vector>

namespace DnmRHI {
    enum class IndexType : u8 {
        eUint16,
        eUint32,
    };

    enum class AttachmentLoadOp : u8 {
        eNone,
        eLoad,
        eClear,
    };

    enum class AttachmentStoreOp : u8 {
        eNone,
        eStore,
    };

    enum class PipelineFlagBit : u32 {
        eCopy = 1 << 0,
        eBlit = 1 << 1,
        eResolve = 1 << 2,
        eIndexInput = 1 << 3,
        eIndirect = 1 << 4,
        eColorAttachmentOutput = 1 << 5,
        eEarlyFragmentTests = 1 << 6,
        eLateFragmentTests = 1 << 7,
        eConditionalRendering = 1 << 8,
        eVertexShader = 1 << 9,
        eFragmentShader = 1 << 10,
        eComputeShader = 1 << 11,
        eTaskShader = 1 << 12,
        eMeshShader = 1 << 13,
        eRayTracingShader = 1 << 14,
        eVariableRateShading = 1 << 15,
        eAccelerationStructureBuild = 1 << 16,
        eAccelerationStructureCopy = 1 << 17
    };
    using PipelineFlags = Flags<PipelineFlagBit>;

    enum class AccessFlagBit : u32 {
        eTransferRead = 1 << 0,
        eTransferWrite = 1 << 1,
        eIndexRead = 1 << 2,
        eIndirectRead = 1 << 3,
        eColorAttachmentWrite = 1 << 4,
        eColorAttachmentRead = 1 << 5,
        eDepthStencilWrite = 1 << 6,
        eDepthStencilRead = 1 << 7,
        eUniformRead = 1 << 8,
        eSampledRead = 1 << 9,
        eStorageRead = 1 << 10,
        eStorageWrite = 1 << 11,
        eAccelerationStructureRead = 1 << 12,
        eAccelerationStructureWrite = 1 << 13,
        eShaderBindingTableRead = 1 << 14,
        eVariableRateShadingRead = 1 << 15,

        eAllRead = eTransferRead | eIndexRead | eIndirectRead | eColorAttachmentRead | eDepthStencilRead | eUniformRead | eSampledRead | eStorageRead | eAccelerationStructureRead | eShaderBindingTableRead | eVariableRateShadingRead,
        eAllWrite = eTransferWrite | eColorAttachmentWrite | eDepthStencilWrite | eStorageWrite | eAccelerationStructureWrite
    };
    using AccessFlags = Flags<AccessFlagBit>;

    // TODO: name?
    struct SyncInfo {
        AccessFlags access{};
        PipelineFlags stages{};

        constexpr auto &SetAccess(AccessFlags access) noexcept { this->access = access; return *this; }
        constexpr auto &SetStages(PipelineFlags stages) noexcept { this->stages = stages; return *this; }

        SyncInfo operator|(const SyncInfo &other) const noexcept {
            return {
                access | other.access,
                stages | other.stages
            };
        }
    };

    enum class RenderPassFlagBit : u8 {
        eAllowStorageWrite = 1 << 0,
        eReadOnlyDepth = 1 << 1,
        eReadOnlyStencil = 1 << 2,
        eEarlyDepthStencil = 1 << 3,
        eLateDepthStencil = 1 << 4,
    };
    using RenderPassFlags = Flags<RenderPassFlagBit>;

    struct InitImageDesc {
        Image *image;
        // keep minimal
        SyncInfo access_flags;

        constexpr auto &SetImage(Image &image) noexcept { this->image = &image; return *this; }
        constexpr auto &SetSyncInfo(SyncInfo access_flags) noexcept { this->access_flags = access_flags; return *this; }
    };

    struct BarrierDesc {
        Resource *resource;
        // in image subresource, subresource type is unnecessary
        Subresource subresource;
        SyncInfo src_access;
        SyncInfo dst_access;

        constexpr auto &SetResource(Resource *resource) noexcept { this->resource = resource; return *this; }
        constexpr auto &SetSubresource(Subresource subresource) noexcept { this->subresource = subresource; return *this; }
        constexpr auto &SetSrcSyncInfo(SyncInfo sync_info) noexcept { this->src_access = sync_info; return *this; }
        constexpr auto &SetDstSyncInfo(SyncInfo sync_info) noexcept { this->dst_access = sync_info; return *this; }
    };

    struct ColorAttachment {
        Image *image;
        ImageSubresource subresource;
        Image *resolve_image;
        ImageSubresource resolve_subresource;
        ResolveModeFlagBit resolve_mode;
        AttachmentLoadOp load_op;
        AttachmentStoreOp store_op;
        std::array<u32, 4> clear_value;

        constexpr auto &SetImage(Image *image) noexcept { this->image = image; return *this; }
        constexpr auto &SetSubresource(ImageSubresource subresource) noexcept { this->subresource = subresource; return *this; }
        constexpr auto &SetResolveImage(Image *image) noexcept { this->resolve_image = image; return *this; }
        constexpr auto &SetResolveSubresource(ImageSubresource subresource) noexcept { this->resolve_subresource = subresource; return *this; }
        constexpr auto &SetLoadOp(AttachmentLoadOp op) noexcept { this->load_op = op; return *this; }
        constexpr auto &SetStoreOp(AttachmentStoreOp op) noexcept { this->store_op = op; return *this; }
        constexpr auto &SetClearValue(std::array<u32, 4> value) noexcept { this->clear_value = value; return *this; }
    };

    struct DepthStencilAttachment {
        Image *image;
        ImageSubresource subresource;
        Image *resolve_image;
        ImageSubresource resolve_subresource;
        Image *stencil_resolve_image;
        ImageSubresource stencil_resolve_subresource;
        ResolveModeFlagBit resolve_mode;
        ResolveModeFlagBit stencil_resolve_mode;
        AttachmentLoadOp load_op;
        AttachmentLoadOp stencil_load_op;
        AttachmentStoreOp store_op;
        AttachmentStoreOp stencil_store_op;
        float depth_clear_value;
        u8 stencil_clear_value;
        
        constexpr auto &SetImage(Image *image) noexcept { this->image = image; return *this; }
        constexpr auto &SetSubresource(ImageSubresource subresource) noexcept { this->subresource = subresource; return *this; }
        constexpr auto &SetResolveImage(Image *image) noexcept { this->resolve_image = image; return *this; }
        constexpr auto &SetResolveSubresource(ImageSubresource subresource) noexcept { this->resolve_subresource = subresource; return *this; }
        constexpr auto &SetLoadOp(AttachmentLoadOp op) noexcept { this->load_op = op; return *this; }
        constexpr auto &SetStoreOp(AttachmentStoreOp op) noexcept { this->store_op = op; return *this; }
        constexpr auto &SetDepthClearValue(float value) noexcept { this->depth_clear_value = value; return *this; }
        constexpr auto &SetStencilClearValue(u8 value) noexcept { this->stencil_clear_value = value; return *this; }
        constexpr auto &SetStencilResolveMode(ResolveModeFlagBit mode) noexcept { this->stencil_resolve_mode = mode; return *this; }
        constexpr auto &SetStencilResolveImage(Image *image) noexcept { this->stencil_resolve_image = image; return *this; }
        constexpr auto &SetStencilResolveSubresource(ImageSubresource subresource) noexcept { this->stencil_resolve_subresource = subresource; return *this; }
        constexpr auto &SetStencilLoadOp(AttachmentLoadOp op) noexcept { this->stencil_load_op = op; return *this; }
        constexpr auto &SetStencilStoreOp(AttachmentStoreOp op) noexcept { this->stencil_store_op = op; return *this; }
        constexpr auto &SetResolveMode(ResolveModeFlagBit mode) noexcept { this->resolve_mode = mode; return *this; }
    };

    struct RenderPassDesc {
        std::span<const ColorAttachment> attachments;
        DepthStencilAttachment *depth_stencil_attachment;
        u32 layer_count = 1;
        RenderPassFlags flags;

        constexpr auto &SetAttachments(std::span<const ColorAttachment> attachments) noexcept { this->attachments = attachments; return *this; }
        constexpr auto &SetAttachments(std::initializer_list<const ColorAttachment> attachments) noexcept { this->attachments = attachments; return *this; }
        constexpr auto &SetDepthStencilAttachment(DepthStencilAttachment *attachment) noexcept { this->depth_stencil_attachment = attachment; return *this; }
        constexpr auto &SetLayerCount(u32 count) noexcept { this->layer_count = count; return *this; }
        constexpr auto &SetFlags(RenderPassFlags flags) noexcept { this->flags = flags; return *this; }
    };
    
    struct Viewport {
        f32 x;
        f32 y;
        f32 width;
        f32 height;
        f32 min_depth;
        f32 max_depth;

        constexpr auto &SetX(f32 x) noexcept { this->x = x; return *this; }
        constexpr auto &SetY(f32 y) noexcept { this->y = y; return *this; }
        constexpr auto &SetWidth(f32 width) noexcept { this->width = width; return *this; }
        constexpr auto &SetHeight(f32 height) noexcept { this->height = height; return *this; }
        constexpr auto &SetMinDepth(f32 min_depth) noexcept { this->min_depth = min_depth; return *this; }
        constexpr auto &SetMaxDepth(f32 max_depth) noexcept { this->max_depth = max_depth; return *this; }
    };

    struct Scissor {
        i32 x;
        i32 y;
        u32 width;
        u32 height;

        constexpr auto &SetX(i32 x) noexcept { this->x = x; return *this; }
        constexpr auto &SetY(i32 y) noexcept { this->y = y; return *this; }
        constexpr auto &SetWidth(u32 width) noexcept { this->width = width; return *this; }
        constexpr auto &SetHeight(u32 height) noexcept { this->height = height; return *this; }
    };

    struct DrawIndirectDesc {
        Buffer *indirect_buffer;
        Buffer *count_buffer; // optional
        u32 indirect_offset;
        u32 count_offset; // optional
        u32 command_stride;
        u32 max_draw_or_draw_count; // if count_buffer is null, this is the max draw count

        constexpr auto &SetIndirectBuffer(Buffer *buffer) noexcept { indirect_buffer = buffer; return *this; }
        constexpr auto &SetCountBuffer(Buffer *buffer) noexcept { count_buffer = buffer; return *this; }
        constexpr auto &SetIndirectOffset(u32 offset) noexcept { indirect_offset = offset; return *this; }
        constexpr auto &SetCountOffset(u32 offset) noexcept { count_offset = offset; return *this; }
        constexpr auto &SetCommandStride(u32 stride) noexcept { command_stride = stride; return *this; }
        constexpr auto &SetMaxDrawOrDrawCount(u32 count) noexcept { max_draw_or_draw_count = count; return *this; }
    };

    struct BlitImageDesc {
        Image *src_image;
        Image *dst_image;
        ImageSubresourceLayers src_subresource;
        ImageSubresourceLayers dst_subresource;
        std::array<Offset3D, 2> src_offset;
        std::array<Offset3D, 2> dst_offset;
        FilterType filter;

        constexpr auto &SetSrcImage(Image *image) noexcept { src_image = image; return *this; }
        constexpr auto &SetDstImage(Image *image) noexcept { dst_image = image; return *this; }
        constexpr auto &SetSrcSubresourceLayers(ImageSubresourceLayers subresource) noexcept { src_subresource = subresource; return *this; }
        constexpr auto &SetDstSubresourceLayers(ImageSubresourceLayers subresource) noexcept { dst_subresource = subresource; return *this; }
        constexpr auto &SetSrcOffset(std::array<Offset3D, 2> offset) noexcept { src_offset = offset; return *this; }
        constexpr auto &SetDstOffset(std::array<Offset3D, 2> offset) noexcept { dst_offset = offset; return *this; }
        constexpr auto &SetFilter(FilterType filter) noexcept { this->filter = filter; return *this; }
    };

    struct BufferCopyDesc {
        Buffer *src_buffer;
        Buffer *dst_buffer;
        u32 src_offset;
        u32 dst_offset;
        u32 size;

        constexpr auto &SetSrcBuffer(Buffer *buffer) noexcept { src_buffer = buffer; return *this; }
        constexpr auto &SetDstBuffer(Buffer *buffer) noexcept { dst_buffer = buffer; return *this; }
        constexpr auto &SetSrcOffset(u32 offset) noexcept { src_offset = offset; return *this; }
        constexpr auto &SetDstOffset(u32 offset) noexcept { dst_offset = offset; return *this; }
        constexpr auto &SetSize(u32 size) noexcept { this->size = size; return *this; }
    };

    struct ImageCopyDesc {
        Image *src_image;
        Image *dst_image;
        ImageSubresourceLayers src_subresource;
        ImageSubresourceLayers dst_subresource;
        Offset3D src_offset;
        Offset3D dst_offset;
        Extent3D extent;

        constexpr auto &SetSrcImage(Image *image) noexcept { src_image = image; return *this; }
        constexpr auto &SetDstImage(Image *image) noexcept { dst_image = image; return *this; }
        constexpr auto &SetSrcSubresourceLayers(ImageSubresourceLayers subresource) noexcept { src_subresource = subresource; return *this; }
        constexpr auto &SetDstSubresourceLayers(ImageSubresourceLayers subresource) noexcept { dst_subresource = subresource; return *this; }
        constexpr auto &SetSrcOffset(Offset3D offset) noexcept { src_offset = offset; return *this; }
        constexpr auto &SetDstOffset(Offset3D offset) noexcept { dst_offset = offset; return *this; }
        constexpr auto &SetExtent(Extent3D extent) noexcept { this->extent = extent; return *this; }
    };

    struct BufferImageCopyDesc {
        Buffer *buffer;
        Image *image;
        Offset3D image_offset;
        Extent3D extent;
        u32 buffer_offset;
        ImageSubresourceLayers subresource;

        constexpr auto &SetBuffer(Buffer *buffer) noexcept { this->buffer = buffer; return *this; }
        constexpr auto &SetImage(Image *image) noexcept { this->image = image; return *this; }
        constexpr auto &SetImageOffset(Offset3D offset) noexcept { image_offset = offset; return *this; }
        constexpr auto &SetExtent(Extent3D extent) noexcept { this->extent = extent; return *this; }
        constexpr auto &SetBufferOffset(u32 offset) noexcept { buffer_offset = offset; return *this; }   
        constexpr auto &SetSubresourceLayers(ImageSubresourceLayers subresource) noexcept { this->subresource = subresource; return *this; }
    };

    struct ResolveImageDesc {
        Image *src_image;
        Image *dst_image;
        ImageSubresourceLayers src_subresource;
        ImageSubresourceLayers dst_subresource;
        Offset3D src_offset;
        Offset3D dst_offset;
        Extent3D extent;
        ResolveModeFlagBit resolve_mode;

        constexpr auto &SetSrcImage(Image *image) noexcept { this->src_image = image; return *this; }
        constexpr auto &SetDstImage(Image *image) noexcept { this->dst_image = image; return *this; }
        constexpr auto &SetSrcSubresourceLayers(ImageSubresourceLayers subresource) noexcept { this->src_subresource = subresource; return *this; }
        constexpr auto &SetDstSubresourceLayers(ImageSubresourceLayers subresource) noexcept { this->dst_subresource = subresource; return *this; }
        constexpr auto &SetSrcOffset(Offset3D offset) noexcept { this->src_offset = offset; return *this; }
        constexpr auto &SetDstOffset(Offset3D offset) noexcept { this->dst_offset = offset; return *this; }
        constexpr auto &SetExtent(Extent3D extent) noexcept { this->extent = extent; return *this; }
        constexpr auto &SetMode(ResolveModeFlagBit mode) noexcept { this->resolve_mode = mode; return *this; }
    };

    class CommandBuffer : public Object {
    protected:
        explicit CommandBuffer(Device &device, const CommandBufferDesc &desc) noexcept : m_desc(desc), Object(device, ObjectType::eCommandBuffer) {}
    public:
        Result Begin(bool one_time_submit) noexcept;
        Result End() noexcept;
        Result Reset() noexcept;

        // General Queue Commands
        Result BeginRenderPass(const RenderPassDesc &desc) noexcept;

        Result SetViewport(std::initializer_list<const Viewport> viewport) noexcept { return SetViewport(std::span(viewport)); }
        Result SetScissor(std::initializer_list<const Scissor> scissor) noexcept { return SetScissor(std::span(scissor)); }
        Result SetViewport(std::span<const Viewport> viewport) noexcept;
        Result SetScissor(std::span<const Scissor> scissor) noexcept;
        Result SetStencilRef(u32 front, u32 back) noexcept;

        Result Draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) noexcept;
        Result DrawIndexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) noexcept;
        Result DrawIndirect(const DrawIndirectDesc &desc) noexcept;
        Result DrawIndexedIndirect(const DrawIndirectDesc &desc) noexcept;

        Result EndRenderPass() noexcept;
        
        Result BlitImage(const BlitImageDesc &desc) noexcept;
        Result ResolveImage(const ResolveImageDesc &desc) noexcept;

        Result BindIndexBuffer(Buffer &buffer, u64 offset, IndexType index_type) noexcept;
        Result BindPipeline(GraphicsPipeline &pipeline) noexcept;

        Result PrepareForPresent(SyncInfo before_sync, Image &swapchain_image) noexcept;

        Result DispatchMesh(u32 x, u32 y, u32 z) noexcept;
        Result DispatchMeshIndirect(const DrawIndirectDesc &desc) noexcept;
        // General Queue Commands
        
        // Compute Queue Commands
        Result Dispatch(u32 x, u32 y, u32 z) noexcept;
        Result DispatchIndirect(Buffer &buffer, u32 offset) noexcept;

        Result BeginConditionalRendering(Buffer &buffer, u32 offset, bool is_inverted) noexcept;
        Result EndConditionalRendering() noexcept;

        Result PushConstant(const auto &data, u32 offset) noexcept {
            return PushConstant(&data, sizeof(PureType<decltype(data)>), offset);
        }
        Result PushConstant(const void *data, u32 size, u32 offset) noexcept;
        Result BindPipeline(ComputePipeline &pipeline) noexcept;
        // Compute Queue Commands

        // Copy Queue Commands
        Result CopyBuffer(const BufferCopyDesc &desc) noexcept;
        Result CopyImage(const ImageCopyDesc &desc) noexcept;
        Result CopyBufferToImage(const BufferImageCopyDesc &desc) noexcept;
        Result CopyImageToBuffer(const BufferImageCopyDesc &desc) noexcept;
        // Copy Queue Commands

        // for non-swapchain images
        // in Vulkan translate image layout Undefined to selected image default layout
        // TODO: D3D12
        // always swapchain images need initialize
        // in Vulkan translate image layout PresentSrc to selected image default layout
        // TODO: D3D12
        Result InitImage(std::initializer_list<const InitImageDesc> descs) noexcept {
            return InitImage(std::span(descs));
        }
        Result InitImage(std::span<const InitImageDesc> descs) noexcept;
        Result Barrier(std::initializer_list<const BarrierDesc> barriers) noexcept {
            return Barrier(std::span(barriers));
        }
        Result Barrier(std::span<const BarrierDesc> barriers) noexcept;

        Result BeginDebugMarker(std::string_view name, std::array<u8, 4> color) noexcept;
        Result EndDebugMarker() noexcept;

        [[nodiscard]] const auto &GetDesc() const noexcept { return m_desc; }
    protected:
        virtual Result IBegin(bool one_time_submit) noexcept = 0;
        virtual Result IEnd() noexcept = 0;
        virtual Result IReset() noexcept = 0;

        // General Queue Commands
        virtual Result IBeginRenderPass(const RenderPassDesc &desc) noexcept = 0;

        virtual Result ISetViewport(std::span<const Viewport> viewport) noexcept = 0;
        virtual Result ISetScissor(std::span<const Scissor> scissor) noexcept = 0;
        virtual Result ISetStencilRef(u32 front, u32 back) noexcept = 0;

        virtual Result IDraw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) noexcept = 0;
        virtual Result IDrawIndexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) noexcept = 0;
        virtual Result IDrawIndirect(const DrawIndirectDesc &desc) noexcept = 0;
        virtual Result IDrawIndexedIndirect(const DrawIndirectDesc &desc) noexcept = 0;

        virtual Result IEndRenderPass() noexcept = 0;
        
        virtual Result IBlitImage(const BlitImageDesc &desc) noexcept = 0;
        virtual Result IResolveImage(const ResolveImageDesc &desc) noexcept = 0;
        
        virtual Result IBindIndexBuffer(DnmRHI::Buffer &buffer, u64 offset, IndexType index_type) noexcept = 0;
        virtual Result IBindPipeline(DnmRHI::GraphicsPipeline &pipeline) noexcept = 0;
        virtual Result IPrepareForPresent(SyncInfo before_sync, DnmRHI::Image &swapchain_image) noexcept = 0;
        // General Queue Commands
        
        // Compute Queue Commands
        virtual Result IDispatch(u32 x, u32 y, u32 z) noexcept = 0;
        virtual Result IDispatchIndirect(DnmRHI::Buffer &buffer, u32 offset) noexcept = 0;

        virtual Result IBeginConditionalRendering(DnmRHI::Buffer &buffer, u32 offset, bool is_inverted) noexcept = 0;
        virtual Result IEndConditionalRendering() noexcept = 0;

        virtual Result IPushConstant(const void *data, u32 size, u32 offset) noexcept = 0;
        virtual Result IBindPipeline(DnmRHI::ComputePipeline &pipeline) noexcept = 0;
        // Compute Queue Commands

        // Copy Queue Commands
        virtual Result ICopyBuffer(const BufferCopyDesc &desc) noexcept = 0;
        virtual Result ICopyImage(const ImageCopyDesc &desc) noexcept = 0;
        virtual Result ICopyBufferToImage(const BufferImageCopyDesc &desc) noexcept = 0;
        virtual Result ICopyImageToBuffer(const BufferImageCopyDesc &desc) noexcept = 0;
        // Copy Queue Commands
        
        virtual Result IBarrier(std::span<const BarrierDesc> barriers) noexcept = 0;
        virtual Result IInitImage(std::span<const InitImageDesc> descs) noexcept = 0;

        virtual Result IBeginDebugMarker(std::string_view name, std::array<u8, 4> color) noexcept = 0;
        virtual Result IEndDebugMarker() noexcept = 0;

        const CommandBufferDesc m_desc;
        std::vector<DnmRHI::Object *> m_used_objects;
        bool m_open{};
        bool m_in_render_pass{};
        bool m_in_condition{};
        bool m_in_debug_marker{};

        DnmRHI::ComputePipeline *m_bound_compute_pipeline{};
        DnmRHI::GraphicsPipeline *m_bound_graphics_pipeline{};
    };
}

template <> struct DnmRHI::FlagTraits<DnmRHI::PipelineFlagBit> {
    static constexpr b8 is_bitmask = true;
};

template <> struct DnmRHI::FlagTraits<DnmRHI::AccessFlagBit> {
    static constexpr b8 is_bitmask = true;
};

template <> struct DnmRHI::FlagTraits<DnmRHI::RenderPassFlagBit> {
    static constexpr b8 is_bitmask = true;
};