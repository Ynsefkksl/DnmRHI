#pragma once

#include <DnmRHI/Types.hpp>
#include <DnmRHI/Common/CompareOp.hpp>
#include <DnmRHI/Common/Format.hpp>
#include <DnmRHI/Common/ColorComponents.hpp>
#include <DnmRHI/Image/SampleCount.hpp>
#include <span>
#include <string>
#include <vector>

namespace DnmRHI {
    enum class PrimitiveTopology : u8 {
        ePointList,
        eLineList,
        eLineStrip,
        eTriangleList,
        eTriangleStrip,
        eTriangleFan,
    };

    enum class PolygonMode : u8 {
        eFill,
        eLine,
    };

    enum class CullMode : u8 {
        eNone,
        eFront,
        eBack,
    };

    enum class FrontFace : u8 {
        eCounterClockwise,
        eClockwise,
    };

    enum class StencilOp : u8 {
        eKeep,
        eZero,
        eReplace,
        eIncrementAndClamp,
        eDecrementAndClamp,
        eInvert,
        eIncrementAndWrap,
        eDecrementAndWrap,
    };

    enum class BlendFactor : u8 {
        eZero,
        eOne,
        eSrcColor,
        eOneMinusSrcColor,
        eDstColor,
        eOneMinusDstColor,
        eSrcAlpha,
        eOneMinusSrcAlpha,
        eDstAlpha,
        eOneMinusDstAlpha,
        eConstantColor,
        eOneMinusConstantColor,
        eConstantAlpha,
        eOneMinusConstantAlpha,
        eSrcAlphaSaturate,
    };

    enum class BlendOp : u8 {
        eAdd,
        eSubtract,
        eReverseSubtract,
        eMinimum,
        eMaximum,
    };

    struct RasterizationState {
        PrimitiveTopology primitive_topology = PrimitiveTopology::eTriangleFan;
        FrontFace front_face = FrontFace::eClockwise;
        PolygonMode polygon_mode = PolygonMode::eFill;
        CullMode cull_mode = CullMode::eNone;
        SampleCount sample_count = SampleCount::e1;
        b8 sample_shading_enable = false;
        u8 viewport_count{1};
        u8 scissor_count{1};

        constexpr auto &SetPrimitiveTopology(PrimitiveTopology topology) noexcept { this->primitive_topology = topology; return *this; }
        constexpr RasterizationState &SetFrontFace(FrontFace face) noexcept { this->front_face = face; return *this; }
        constexpr RasterizationState &SetPolygonMode(PolygonMode mode) noexcept { this->polygon_mode = mode; return *this; }
        constexpr RasterizationState &SetCullMode(CullMode mode) noexcept { this->cull_mode = mode; return *this; }
        constexpr RasterizationState &SetSampleCount(SampleCount count) noexcept { this->sample_count = count; return *this; }
        constexpr RasterizationState &SetSampleShadingEnable(b8 enable) noexcept { this->sample_shading_enable = enable; return *this; }
        constexpr RasterizationState &SetScissorCount(u8 count) noexcept { this->viewport_count = count; return *this; }
        constexpr RasterizationState &SetViewportCount(u8 count) noexcept { this->scissor_count = count; return *this; }
    };

    struct StencilOpState {
        StencilOp depth_fail_op;
        StencilOp fail_op;
        StencilOp pass_op;
        CompareOp compare_op;
        u8 compare_mask;
        u8 write_mask;

        constexpr auto &SetDepthFailOp(StencilOp op) noexcept { this->depth_fail_op = op; return *this; }
        constexpr auto &SetFailOp(StencilOp op) noexcept { this->fail_op = op; return *this; }
        constexpr auto &SetPassOp(StencilOp op) noexcept { this->pass_op = op; return *this; }
        constexpr auto &SetCompareOp(CompareOp op) noexcept { this->compare_op = op; return *this; }
        constexpr auto &SetCompareMask(u8 mask) noexcept { this->compare_mask = mask; return *this; }
        constexpr auto &SetWriteMask(u8 mask) noexcept { this->write_mask = mask; return *this; }
    };

    struct DepthStencilState {
        Format depth_attachment;
        Format stencil_attachment;
        CompareOp depth_test_compare_op;
        StencilOpState stencil_front_op;
        StencilOpState stencil_back_op;
        b8 depth_test_enable;
        b8 depth_write_enable;
        b8 stencil_test_enable;

        constexpr auto &SetDepthAttachment(Format format) noexcept { this->depth_attachment = format; return *this; }
        constexpr auto &SetStencilAttachment(Format format) noexcept { this->stencil_attachment = format; return *this; }
        constexpr auto &SetDepthTestCompareOp(CompareOp op) noexcept { this->depth_test_compare_op = op; return *this; }
        constexpr auto &SetStencilFrontOp(const StencilOpState &op) noexcept { this->stencil_front_op = op; return *this; }
        constexpr auto &SetStencilBackOp(const StencilOpState &op) noexcept { this->stencil_back_op = op; return *this; }
        constexpr auto &SetDepthTestEnable(b8 enable) noexcept { this->depth_test_enable = enable; return *this; }
        constexpr auto &SetDepthWriteEnable(b8 enable) noexcept { this->depth_write_enable = enable; return *this; }
        constexpr auto &SetStencilTestEnable(b8 enable) noexcept { this->stencil_test_enable = enable; return *this; }
    };

    struct ColorOutput {
        Format format;
        ColorComponentFlags color_write_mask{ColorComponentFlagBit::eR | ColorComponentFlagBit::eG | ColorComponentFlagBit::eB | ColorComponentFlagBit::eA};
        
        b8 blend_enable;

        BlendFactor src_color_blend_factor;
        BlendFactor dst_color_blend_factor;
        BlendOp color_blend_op;
        BlendFactor src_alpha_blend_factor;
        BlendFactor dst_alpha_blend_factor;
        BlendOp alpha_blend_op;

        constexpr auto &SetFormat(Format format) noexcept { this->format = format; return *this; }
        constexpr auto &SetBlendEnable(b8 enable) noexcept { this->blend_enable = enable; return *this; }
        constexpr auto &SetSrcColorBlendFactor(BlendFactor factor) noexcept { this->src_color_blend_factor = factor; return *this; }
        constexpr auto &SetDstColorBlendFactor(BlendFactor factor) noexcept { this->dst_color_blend_factor = factor; return *this; }
        constexpr auto &SetColorBlendOp(BlendOp op) noexcept { this->color_blend_op = op; return *this; }
        constexpr auto &SetSrcAlphaBlendFactor(BlendFactor factor) noexcept { this->src_alpha_blend_factor = factor; return *this; }
        constexpr auto &SetDstAlphaBlendFactor(BlendFactor factor) noexcept { this->dst_alpha_blend_factor = factor; return *this; }
        constexpr auto &SetAlphaBlendOp(BlendOp op) noexcept { this->alpha_blend_op = op; return *this; }
        constexpr auto &SetColorWriteMask(ColorComponentFlags mask) noexcept { this->color_write_mask = mask; return *this; }
    };

    struct ColorOutputState {
        std::vector<ColorOutput> color_attachment_formats;

        constexpr auto &SetColorAttachmentFormats(std::span<const ColorOutput> formats) noexcept { this->color_attachment_formats = std::vector<ColorOutput>(formats.begin(), formats.end()); return *this; }
        constexpr auto &SetColorAttachmentFormats(std::initializer_list<ColorOutput> formats) noexcept { this->color_attachment_formats = std::vector<ColorOutput>(formats); return *this; }
    };

    struct GraphicsPipelineDesc {
        std::string name;
        std::vector<Shader *> shaders;
        RasterizationState rasterization_state;
        ColorOutputState color_output_state;
        DepthStencilState depth_stencil_state;
        
        constexpr auto &SetName(std::string_view name) noexcept { this->name = name; return *this; }
        constexpr auto &SetShaders(std::span<Shader * const> shaders) noexcept { this->shaders = std::vector<Shader *>(shaders.begin(), shaders.end()); return *this; }
        constexpr auto &SetShaders(std::initializer_list<Shader *> shaders) noexcept { this->shaders = std::vector<Shader *>(shaders); return *this; }
        constexpr auto &SetRasterizationState(const RasterizationState &state) noexcept { this->rasterization_state = state; return *this; }
        constexpr auto &SetColorOutputState(const ColorOutputState &state) noexcept { this->color_output_state = state; return *this; }
        constexpr auto &SetDepthStencilState(const DepthStencilState &state) noexcept { this->depth_stencil_state = state; return *this; }
    };
}