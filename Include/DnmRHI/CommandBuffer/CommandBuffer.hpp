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

    enum class AccessFlagBit : u64 {
        eCopyRead                       = 1ull << 0,
        eCopyWrite                      = 1ull << 1,
        eBlitRead                       = 1ull << 2,
        eBlitWrite                      = 1ull << 3,
        eResolveRead                    = 1ull << 4,
        eResolveWrite                   = 1ull << 5,
        eVertexRead                     = 1ull << 6,
        eIndexRead                      = 1ull << 7,
        eIndirectRead                   = 1ull << 8,
        eColorAttachmentWrite           = 1ull << 9,
        eColorAttachmentRead            = 1ull << 10,
        eEarlyFragmentWrite             = 1ull << 11,
        eEarlyFragmentRead              = 1ull << 12,
        eLateFragmentWrite              = 1ull << 13,
        eLateFragmentRead               = 1ull << 14,
        eVariableRateShadingRead        = 1ull << 15,
        eConditionalRenderingRead       = 1ull << 16,
        eShaderBindingTableRead         = 1ull << 17,
        eAccelerationStructureBuildRead = 1ull << 18,
        eAccelerationStructureBuildWrite= 1ull << 19,
        eAccelerationStructureCopyRead  = 1ull << 20,
        eAccelerationStructureCopyWrite = 1ull << 21,

        // Vertex shader
        eStorageWriteVertexShader               = 1ull << 22,
        eStorageReadVertexShader                = 1ull << 23,
        eSampledReadVertexShader                = 1ull << 24,
        eUniformReadVertexShader                = 1ull << 25,
        eAccelerationStructureReadVertexShader  = 1ull << 26,

        // Fragment shader
        eStorageWriteFragmentShader             = 1ull << 27,
        eStorageReadFragmentShader              = 1ull << 28,
        eSampledReadFragmentShader              = 1ull << 29,
        eUniformReadFragmentShader              = 1ull << 30,
        eAccelerationStructureReadFragmentShader= 1ull << 31,

        // Compute shader
        eStorageWriteComputeShader              = 1ull << 32,
        eStorageReadComputeShader               = 1ull << 33,
        eSampledReadComputeShader               = 1ull << 34,
        eUniformReadComputeShader               = 1ull << 35,
        eAccelerationStructureReadComputeShader = 1ull << 36,

        // Task shader
        eStorageWriteTaskShader                 = 1ull << 37,
        eStorageReadTaskShader                  = 1ull << 38,
        eSampledReadTaskShader                  = 1ull << 39,
        eUniformReadTaskShader                  = 1ull << 40,
        eAccelerationStructureReadTaskShader    = 1ull << 41,

        // Mesh shader
        eStorageWriteMeshShader                 = 1ull << 42,
        eStorageReadMeshShader                  = 1ull << 43,
        eSampledReadMeshShader                  = 1ull << 44,
        eUniformReadMeshShader                  = 1ull << 45,
        eAccelerationStructureReadMeshShader    = 1ull << 46,

        // Ray tracing shaders
        eStorageWriteRayTracingShaders          = 1ull << 47,
        eStorageReadRayTracingShaders           = 1ull << 48,
        eSampledReadRayTracingShaders           = 1ull << 49,
        eUniformReadRayTracingShaders           = 1ull << 50,
        eAccelerationStructureReadRayTracingShaders = 1ull << 51,

        eDepthStencilAttachmentWrite = eEarlyFragmentWrite | eLateFragmentWrite,
        eDepthStencilAttachmentRead  = eEarlyFragmentRead  | eLateFragmentRead,

        eStorageWriteAllStages =
            eStorageWriteVertexShader | eStorageWriteFragmentShader | eStorageWriteComputeShader |
            eStorageWriteTaskShader | eStorageWriteMeshShader | eStorageWriteRayTracingShaders,
        eStorageReadAllStages =
            eStorageReadVertexShader | eStorageReadFragmentShader | eStorageReadComputeShader |
            eStorageReadTaskShader | eStorageReadMeshShader | eStorageReadRayTracingShaders,
        eSampledReadAllStages =
            eSampledReadVertexShader | eSampledReadFragmentShader | eSampledReadComputeShader |
            eSampledReadTaskShader | eSampledReadMeshShader | eSampledReadRayTracingShaders,
        eUniformReadAllStages =
            eUniformReadVertexShader | eUniformReadFragmentShader | eUniformReadComputeShader |
            eUniformReadTaskShader | eUniformReadMeshShader | eUniformReadRayTracingShaders,
        eAccelerationStructureReadAllStages =
            eAccelerationStructureReadVertexShader | eAccelerationStructureReadFragmentShader |
            eAccelerationStructureReadComputeShader | eAccelerationStructureReadTaskShader |
            eAccelerationStructureReadMeshShader | eAccelerationStructureReadRayTracingShaders,

        eAllWrite =
            eColorAttachmentWrite | eDepthStencilAttachmentWrite | eStorageWriteAllStages |
            eBlitWrite | eResolveWrite | eCopyWrite |
            eAccelerationStructureBuildWrite | eAccelerationStructureCopyWrite,
        eAllRead =
            eColorAttachmentRead | eDepthStencilAttachmentRead | eStorageReadAllStages |
            eSampledReadAllStages | eUniformReadAllStages | eAccelerationStructureReadAllStages |
            eBlitRead | eResolveRead | eCopyRead |
            eVertexRead | eIndexRead | eIndirectRead |
            eVariableRateShadingRead | eConditionalRenderingRead | eShaderBindingTableRead |
            eAccelerationStructureBuildRead | eAccelerationStructureCopyRead,
    };
    using AccessFlags = Flags<AccessFlagBit>;

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
        AccessFlags dst_access_flags;

        constexpr auto &SetImage(Image &image) noexcept { this->image = &image; return *this; }
        constexpr auto &SetDstAccessFlags(AccessFlags access_flags) noexcept { this->dst_access_flags = access_flags; return *this; }
    };

    struct BarrierDesc {
        Resource *resource;
        // in image subresource, subresource type is unnecessary
        Subresource subresource;
        AccessFlags src_access;
        AccessFlags dst_access;

        constexpr auto &SetResource(Resource *resource) noexcept { this->resource = resource; return *this; }
        constexpr auto &SetSubresource(Subresource subresource) noexcept { this->subresource = subresource; return *this; }
        constexpr auto &SetSrcAccess(AccessFlags access) noexcept { this->src_access = access; return *this; }
        constexpr auto &SetDstAccess(AccessFlags access) noexcept { this->dst_access = access; return *this; }
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
        void Begin(bool one_time_submit) noexcept;
        void End() noexcept;
        void Reset() noexcept;

        // General Queue Commands
        void BeginRenderPass(const RenderPassDesc &desc) noexcept;

        void SetViewport(std::initializer_list<const Viewport> viewport) noexcept { SetViewport(std::span(viewport)); }
        void SetScissor(std::initializer_list<const Scissor> scissor) noexcept { SetScissor(std::span(scissor)); }
        void SetViewport(std::span<const Viewport> viewport) noexcept;
        void SetScissor(std::span<const Scissor> scissor) noexcept;
        void SetStencilRef(u32 front, u32 back) noexcept;

        void Draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) noexcept;
        void DrawIndexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) noexcept;
        void DrawIndirect(const DrawIndirectDesc &desc) noexcept;
        void DrawIndexedIndirect(const DrawIndirectDesc &desc) noexcept;

        void EndRenderPass() noexcept;
        
        void BlitImage(const BlitImageDesc &desc) noexcept;
        void ResolveImage(const ResolveImageDesc &desc) noexcept;

        void BindIndexBuffer(Buffer &buffer, u64 offset, IndexType index_type) noexcept;
        void BindPipeline(GraphicsPipeline &pipeline) noexcept;

        void PrepareForPresent(AccessFlags before_access, Image &swapchain_image) noexcept;

        void DispatchMesh(u32 x, u32 y, u32 z) noexcept;
        void DispatchMeshIndirect(const DrawIndirectDesc &desc) noexcept;
        // General Queue Commands
        
        // Compute Queue Commands
        void Dispatch(u32 x, u32 y, u32 z) noexcept;
        void DispatchIndirect(Buffer &buffer, u32 offset) noexcept;

        void BeginConditionalRendering(Buffer &buffer, u32 offset, bool is_inverted) noexcept;
        void EndConditionalRendering() noexcept;

        void PushConstant(const auto &data, u32 offset) noexcept {
            PushConstant(&data, sizeof(PureType<decltype(data)>), offset);
        }
        void PushConstant(const void *data, u32 size, u32 offset) noexcept;
        void BindPipeline(ComputePipeline &pipeline) noexcept;
        // Compute Queue Commands

        // Copy Queue Commands
        void CopyBuffer(const BufferCopyDesc &desc) noexcept;
        void CopyImage(const ImageCopyDesc &desc) noexcept;
        void CopyBufferToImage(const BufferImageCopyDesc &desc) noexcept;
        void CopyImageToBuffer(const BufferImageCopyDesc &desc) noexcept;
        // Copy Queue Commands

        // for non-swapchain images
        // in Vulkan translate image layout Undefined to selected image default layout
        // TODO: D3D12
        // always swapchain images need initialize
        // in Vulkan translate image layout PresentSrc to selected image default layout
        // TODO: D3D12
        void InitImage(std::initializer_list<const InitImageDesc> descs) noexcept {
            InitImage(std::span(descs));
        }
        void InitImage(std::span<const InitImageDesc> descs) noexcept;
        void Barrier(std::initializer_list<const BarrierDesc> barriers) noexcept {
            Barrier(std::span(barriers));
        }
        void Barrier(std::span<const BarrierDesc> barriers) noexcept;

        void BeginDebugMarker(std::string_view name, std::array<u8, 4> color) noexcept;
        void EndDebugMarker() noexcept;

        [[nodiscard]] const auto &GetDesc() const noexcept { return m_desc; }
    protected:
        virtual void IBegin(bool one_time_submit) noexcept = 0;
        virtual void IEnd() noexcept = 0;
        virtual void IReset() noexcept = 0;

        // General Queue Commands
        virtual void IBeginRenderPass(const RenderPassDesc &desc) noexcept = 0;

        virtual void ISetViewport(std::span<const Viewport> viewport) noexcept = 0;
        virtual void ISetScissor(std::span<const Scissor> scissor) noexcept = 0;
        virtual void ISetStencilRef(u32 front, u32 back) noexcept = 0;

        virtual void IDraw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) noexcept = 0;
        virtual void IDrawIndexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance) noexcept = 0;
        virtual void IDrawIndirect(const DrawIndirectDesc &desc) noexcept = 0;
        virtual void IDrawIndexedIndirect(const DrawIndirectDesc &desc) noexcept = 0;

        virtual void IEndRenderPass() noexcept = 0;
        
        virtual void IBlitImage(const BlitImageDesc &desc) noexcept = 0;
        virtual void IResolveImage(const ResolveImageDesc &desc) noexcept = 0;
        
        virtual void IBindIndexBuffer(DnmRHI::Buffer &buffer, u64 offset, IndexType index_type) noexcept = 0;
        virtual void IBindPipeline(DnmRHI::GraphicsPipeline &pipeline) noexcept = 0;
        virtual void IPrepareForPresent(AccessFlags before_access, DnmRHI::Image &swapchain_image) noexcept = 0;
        // General Queue Commands
        
        // Compute Queue Commands
        virtual void IDispatch(u32 x, u32 y, u32 z) noexcept = 0;
        virtual void IDispatchIndirect(DnmRHI::Buffer &buffer, u32 offset) noexcept = 0;

        virtual void IBeginConditionalRendering(DnmRHI::Buffer &buffer, u32 offset, bool is_inverted) noexcept = 0;
        virtual void IEndConditionalRendering() noexcept = 0;

        virtual void IPushConstant(const void *data, u32 size, u32 offset) noexcept = 0;
        virtual void IBindPipeline(DnmRHI::ComputePipeline &pipeline) noexcept = 0;
        // Compute Queue Commands

        // Copy Queue Commands
        virtual void ICopyBuffer(const BufferCopyDesc &desc) noexcept = 0;
        virtual void ICopyImage(const ImageCopyDesc &desc) noexcept = 0;
        virtual void ICopyBufferToImage(const BufferImageCopyDesc &desc) noexcept = 0;
        virtual void ICopyImageToBuffer(const BufferImageCopyDesc &desc) noexcept = 0;
        // Copy Queue Commands
        
        virtual void IBarrier(std::span<const BarrierDesc> barriers) noexcept = 0;
        virtual void IInitImage(std::span<const InitImageDesc> descs) noexcept = 0;

        virtual void IBeginDebugMarker(std::string_view name, std::array<u8, 4> color) noexcept = 0;
        virtual void IEndDebugMarker() noexcept = 0;

        const CommandBufferDesc m_desc;
        bool m_open{};
        bool m_in_render_pass{};
        bool m_in_condition{};
        bool m_in_debug_marker{};
    };
}

template <> struct DnmRHI::FlagTraits<DnmRHI::AccessFlagBit> {
    static constexpr b8 is_bitmask = true;
};

template <> struct DnmRHI::FlagTraits<DnmRHI::RenderPassFlagBit> {
    static constexpr b8 is_bitmask = true;
};