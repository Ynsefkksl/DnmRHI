#pragma once

#include <DnmRHI/DnmRHI.hpp>

#ifdef DnmRHI_Platform_Win32
    #define VK_USE_PLATFORM_WIN32_KHR
#else
    #define VK_USE_PLATFORM_XCB_KHR
    #define VK_USE_PLATFORM_XLIB_KHR
    #define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#include <vulkan/vulkan.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <vma/vk_mem_alloc.h>
#pragma clang diagnostic pop

namespace DnmRHI::Vk {
    struct Sync {
        vk::PipelineStageFlags2 stage;
        vk::AccessFlags2 access;
    };

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::Format format) noexcept -> vk::Format {
        switch (format) {
            case Format::eUndefined:               return vk::Format::eUndefined;
            case Format::eB8G8R8A8_Srgb:           return vk::Format::eB8G8R8A8Srgb;
            case Format::eB8G8R8A8_Unorm:          return vk::Format::eB8G8R8A8Unorm;
            case Format::eR8G8B8A8_Unorm:          return vk::Format::eR8G8B8A8Unorm;
            case Format::eR8G8B8A8_Snorm:          return vk::Format::eR8G8B8A8Snorm;
            case Format::eR8G8B8A8_Srgb:           return vk::Format::eR8G8B8A8Srgb;
            case Format::eR8G8B8A8_Int:            return vk::Format::eR8G8B8A8Sint;
            case Format::eR8G8B8A8_Uint:           return vk::Format::eR8G8B8A8Uint;
            case Format::eR8G8_Unorm:              return vk::Format::eR8G8Unorm;
            case Format::eR8G8_Snorm:              return vk::Format::eR8G8Snorm;
            case Format::eR8G8_Int:                return vk::Format::eR8G8Sint;
            case Format::eR8G8_Uint:               return vk::Format::eR8G8Uint;
            case Format::eR8_Unorm:                return vk::Format::eR8Unorm;
            case Format::eR8_Snorm:                return vk::Format::eR8Snorm;
            case Format::eR8_Int:                  return vk::Format::eR8Sint;
            case Format::eR8_Uint:                 return vk::Format::eR8Uint;
            case Format::eR16G16B16A16_Unorm:      return vk::Format::eR16G16B16A16Unorm;
            case Format::eR16G16B16A16_Snorm:      return vk::Format::eR16G16B16A16Snorm;
            case Format::eR16G16B16A16_Int:        return vk::Format::eR16G16B16A16Sint;
            case Format::eR16G16B16A16_Uint:       return vk::Format::eR16G16B16A16Uint;
            case Format::eR16G16B16A16_Float:      return vk::Format::eR16G16B16A16Sfloat;
            case Format::eR16G16_Unorm:            return vk::Format::eR16G16Unorm;
            case Format::eR16G16_Snorm:            return vk::Format::eR16G16Snorm;
            case Format::eR16G16_Uint:             return vk::Format::eR16G16Uint;
            case Format::eR16G16_Int:              return vk::Format::eR16G16Sint;
            case Format::eR16G16_Float:            return vk::Format::eR16G16Sfloat;
            case Format::eR16_Uint:                return vk::Format::eR16Uint;
            case Format::eR16_Int:                 return vk::Format::eR16Sint;
            case Format::eR16_Float:               return vk::Format::eR16Sfloat;
            case Format::eR16_Unorm:               return vk::Format::eR16Unorm;
            case Format::eR16_Snorm:               return vk::Format::eR16Snorm;
            case Format::eR32G32B32A32_Int:        return vk::Format::eR32G32B32A32Sint;
            case Format::eR32G32B32A32_Uint:       return vk::Format::eR32G32B32A32Uint;
            case Format::eR32G32B32A32_Float:      return vk::Format::eR32G32B32A32Sfloat;
            case Format::eR32G32_Int:              return vk::Format::eR32G32Sint;
            case Format::eR32G32_Uint:             return vk::Format::eR32G32Uint;
            case Format::eR32G32_Float:            return vk::Format::eR32G32Sfloat;
            case Format::eR32_Int:                 return vk::Format::eR32Sint;
            case Format::eR32_Uint:                return vk::Format::eR32Uint;
            case Format::eR32_Float:                return vk::Format::eR32Sfloat;
            case Format::eR11G11B10_Ufloat:        return vk::Format::eB10G11R11UfloatPack32;
            case Format::eR10G10B10A2_Unorm:       return vk::Format::eA2B10G10R10UnormPack32;
            case Format::eR10G10B10A2_Uint:        return vk::Format::eA2B10G10R10UintPack32;
            case Format::eBC1_Rgba_Unorm:          return vk::Format::eBc1RgbaUnormBlock;
            case Format::eBC1_Rgba_Srgb:           return vk::Format::eBc1RgbaSrgbBlock;
            case Format::eBC1_Rgb_Unorm:           return vk::Format::eBc1RgbUnormBlock;
            case Format::eBC1_Rgb_Srgb:            return vk::Format::eBc1RgbSrgbBlock;
            case Format::eBC2_Unorm:               return vk::Format::eBc2UnormBlock;
            case Format::eBC2_Srgb:                return vk::Format::eBc2SrgbBlock;
            case Format::eBC3_Unorm:               return vk::Format::eBc3UnormBlock;
            case Format::eBC3_Srgb:                return vk::Format::eBc3SrgbBlock;
            case Format::eBC4_Unorm:               return vk::Format::eBc4UnormBlock;
            case Format::eBC4_Snorm:               return vk::Format::eBc4SnormBlock;
            case Format::eBC5_Unorm:               return vk::Format::eBc5UnormBlock;
            case Format::eBC5_Snorm:               return vk::Format::eBc5SnormBlock;
            case Format::eBC6H_Float:              return vk::Format::eBc6HSfloatBlock;
            case Format::eBC6H_Ufloat:             return vk::Format::eBc6HUfloatBlock;
            case Format::eBC7_Unorm:               return vk::Format::eBc7UnormBlock;
            case Format::eBC7_Srgb:                return vk::Format::eBc7SrgbBlock;
            case Format::eS8_Uint:                 return vk::Format::eS8Uint;
            case Format::eD32_Float:               return vk::Format::eD32Sfloat;
            case Format::eD16_Unorm:               return vk::Format::eD16Unorm;
            case Format::eD16_Unorm_S8_Uint:       return vk::Format::eD16UnormS8Uint;
            case Format::eD24_Unorm_S8_Uint:       return vk::Format::eD24UnormS8Uint;
            case Format::eD32_Float_S8_Uint:       return vk::Format::eD32SfloatS8Uint;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::CommandBufferType type) noexcept {
        switch (type) {
            case CommandBufferType::ePrimary: return vk::CommandBufferLevel::ePrimary;
            case CommandBufferType::eSecondary: return vk::CommandBufferLevel::eSecondary;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::QueueType type) noexcept {
        switch (type) {
            case QueueType::eNone: return vk::QueueFlagBits{};
            case QueueType::eGeneral: return vk::QueueFlagBits::eGraphics;
            case QueueType::eCompute: return vk::QueueFlagBits::eCompute;
            case QueueType::eCopy: return vk::QueueFlagBits::eTransfer;
            default: std::unreachable();
        }
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::ImageType type) noexcept {
        switch (type) {
            case ImageType::e2D: return vk::ImageType::e2D;
            case ImageType::e3D: return vk::ImageType::e3D;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::ImageSubresourceType type) noexcept {
        switch (type) {
            case ImageSubresourceType::e2D: return vk::ImageViewType::e2D;
            case ImageSubresourceType::e2DArray: return vk::ImageViewType::e2DArray;
            case ImageSubresourceType::e3D: return vk::ImageViewType::e3D;
            case ImageSubresourceType::eCube: return vk::ImageViewType::eCube;
            case ImageSubresourceType::eCubeArray: return vk::ImageViewType::eCubeArray;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::CompareOp op) noexcept {
        switch (op) {
            case CompareOp::eNone: return vk::CompareOp::eNever;
            case CompareOp::eNever: return vk::CompareOp::eNever;
            case CompareOp::eAlways: return vk::CompareOp::eAlways;
            case CompareOp::eEqual: return vk::CompareOp::eEqual;
            case CompareOp::eNotEqual: return vk::CompareOp::eNotEqual;
            case CompareOp::eLess: return vk::CompareOp::eLess;
            case CompareOp::eLessOrEqual: return vk::CompareOp::eLessOrEqual;
            case CompareOp::eGreater: return vk::CompareOp::eGreater;
            case CompareOp::eGreaterOrEqual: return vk::CompareOp::eGreaterOrEqual;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::FilterType type) noexcept {
        switch (type) {
            case FilterType::eNearest: return vk::Filter::eNearest;
            case FilterType::eLinear: return vk::Filter::eLinear;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::SamplerAddressMode mode) noexcept {
        switch (mode) {
            case SamplerAddressMode::eRepeat: return vk::SamplerAddressMode::eRepeat;
            case SamplerAddressMode::eMirroredRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
            case SamplerAddressMode::eClampToEdge: return vk::SamplerAddressMode::eClampToEdge;
            case SamplerAddressMode::eClampToBorder: return vk::SamplerAddressMode::eClampToBorder;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::PrimitiveTopology topology) noexcept {
        switch (topology) {
            case PrimitiveTopology::ePointList: return vk::PrimitiveTopology::ePointList;
            case PrimitiveTopology::eLineList: return vk::PrimitiveTopology::eLineList;
            case PrimitiveTopology::eLineStrip: return vk::PrimitiveTopology::eLineStrip;
            case PrimitiveTopology::eTriangleList: return vk::PrimitiveTopology::eTriangleList;
            case PrimitiveTopology::eTriangleStrip: return vk::PrimitiveTopology::eTriangleStrip;
            case PrimitiveTopology::eTriangleFan: return vk::PrimitiveTopology::eTriangleFan;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::PolygonMode mode) noexcept {
        switch (mode) {
            case PolygonMode::eFill: return vk::PolygonMode::eFill;
            case PolygonMode::eLine: return vk::PolygonMode::eLine;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::CullMode mode) noexcept {
        switch (mode) {
            case CullMode::eNone: return vk::CullModeFlagBits::eNone;
            case CullMode::eFront: return vk::CullModeFlagBits::eFront;
            case CullMode::eBack: return vk::CullModeFlagBits::eBack;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::AccessFlags access_flags) noexcept {
        Sync barrier;
        if (access_flags.Has(AccessFlagBit::eCopyRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eCopy;
            barrier.access |= vk::AccessFlagBits2::eTransferRead;
        }
        if (access_flags.Has(AccessFlagBit::eCopyWrite)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eCopy;
            barrier.access |= vk::AccessFlagBits2::eTransferWrite;
        }
        if (access_flags.Has(AccessFlagBit::eBlitRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eBlit;
            barrier.access |= vk::AccessFlagBits2::eTransferRead;
        }
        if (access_flags.Has(AccessFlagBit::eBlitWrite)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eBlit;
            barrier.access |= vk::AccessFlagBits2::eTransferWrite;
        }
        if (access_flags.Has(AccessFlagBit::eResolveRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eResolve;
            barrier.access |= vk::AccessFlagBits2::eTransferRead;
        }
        if (access_flags.Has(AccessFlagBit::eResolveWrite)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eResolve;
            barrier.access |= vk::AccessFlagBits2::eTransferWrite;
        }
        if (access_flags.Has(AccessFlagBit::eVertexRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eVertexAttributeInput;
            barrier.access |= vk::AccessFlagBits2::eVertexAttributeRead;
        }
        if (access_flags.Has(AccessFlagBit::eIndexRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eIndexInput;
            barrier.access |= vk::AccessFlagBits2::eIndexRead;
        }
        if (access_flags.Has(AccessFlagBit::eIndirectRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eDrawIndirect;
            barrier.access |= vk::AccessFlagBits2::eIndirectCommandRead;
        }
        if (access_flags.Has(AccessFlagBit::eColorAttachmentWrite)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            barrier.access |= vk::AccessFlagBits2::eColorAttachmentWrite;
        }
        if (access_flags.Has(AccessFlagBit::eColorAttachmentRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            barrier.access |= vk::AccessFlagBits2::eColorAttachmentRead;
        }
        if (access_flags.Has(AccessFlagBit::eEarlyFragmentWrite)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eEarlyFragmentTests;
            barrier.access |= vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
        }
        if (access_flags.Has(AccessFlagBit::eEarlyFragmentRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eEarlyFragmentTests;
            barrier.access |= vk::AccessFlagBits2::eDepthStencilAttachmentRead;
        }
        if (access_flags.Has(AccessFlagBit::eLateFragmentWrite)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eLateFragmentTests;
            barrier.access |= vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
        }
        if (access_flags.Has(AccessFlagBit::eLateFragmentRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eLateFragmentTests;
            barrier.access |= vk::AccessFlagBits2::eDepthStencilAttachmentRead;
        }
        if (access_flags.Has(AccessFlagBit::eStorageWriteRayTracingShaders)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eRayTracingShaderKHR;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageWrite;
        }
        if (access_flags.Has(AccessFlagBit::eStorageReadRayTracingShaders)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eRayTracingShaderKHR;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageRead;
        }
        if (access_flags.Has(AccessFlagBit::eSampledReadRayTracingShaders)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eRayTracingShaderKHR;
            barrier.access |= vk::AccessFlagBits2::eShaderSampledRead;
        }
        if (access_flags.Has(AccessFlagBit::eUniformReadRayTracingShaders)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eRayTracingShaderKHR;
            barrier.access |= vk::AccessFlagBits2::eUniformRead;
        }
        if (access_flags.Has(AccessFlagBit::eAccelerationStructureReadRayTracingShaders)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eRayTracingShaderKHR;
            barrier.access |= vk::AccessFlagBits2::eAccelerationStructureReadKHR;
        }
        if (access_flags.Has(AccessFlagBit::eStorageWriteTaskShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eTaskShaderEXT;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageWrite;
        }
        if (access_flags.Has(AccessFlagBit::eStorageReadTaskShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eTaskShaderEXT;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageRead;
        }
        if (access_flags.Has(AccessFlagBit::eSampledReadTaskShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eTaskShaderEXT;
            barrier.access |= vk::AccessFlagBits2::eShaderSampledRead;
        }
        if (access_flags.Has(AccessFlagBit::eUniformReadTaskShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eTaskShaderEXT;
            barrier.access |= vk::AccessFlagBits2::eUniformRead;
        }
        if (access_flags.Has(AccessFlagBit::eAccelerationStructureReadTaskShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eTaskShaderEXT;
            barrier.access |= vk::AccessFlagBits2::eAccelerationStructureReadKHR;
        }
        if (access_flags.Has(AccessFlagBit::eStorageWriteMeshShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eMeshShaderEXT;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageWrite;
        }
        if (access_flags.Has(AccessFlagBit::eStorageReadMeshShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eMeshShaderEXT;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageRead;
        }
        if (access_flags.Has(AccessFlagBit::eSampledReadMeshShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eMeshShaderEXT;
            barrier.access |= vk::AccessFlagBits2::eShaderSampledRead;
        }
        if (access_flags.Has(AccessFlagBit::eUniformReadMeshShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eMeshShaderEXT;
            barrier.access |= vk::AccessFlagBits2::eUniformRead;
        }
        if (access_flags.Has(AccessFlagBit::eAccelerationStructureReadMeshShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eMeshShaderEXT;
            barrier.access |= vk::AccessFlagBits2::eAccelerationStructureReadKHR;
        }
        if (access_flags.Has(AccessFlagBit::eStorageWriteVertexShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eVertexShader;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageWrite;
        }
        if (access_flags.Has(AccessFlagBit::eStorageReadVertexShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eVertexShader;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageRead;
        }
        if (access_flags.Has(AccessFlagBit::eSampledReadVertexShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eVertexShader;
            barrier.access |= vk::AccessFlagBits2::eShaderSampledRead;
        }
        if (access_flags.Has(AccessFlagBit::eUniformReadVertexShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eVertexShader;
            barrier.access |= vk::AccessFlagBits2::eUniformRead;
        }
        if (access_flags.Has(AccessFlagBit::eAccelerationStructureReadVertexShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eVertexShader;
            barrier.access |= vk::AccessFlagBits2::eAccelerationStructureReadKHR;
        }
        if (access_flags.Has(AccessFlagBit::eStorageWriteFragmentShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eFragmentShader;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageWrite;
        }
        if (access_flags.Has(AccessFlagBit::eStorageReadFragmentShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eFragmentShader;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageRead;
        }
        if (access_flags.Has(AccessFlagBit::eSampledReadFragmentShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eFragmentShader;
            barrier.access |= vk::AccessFlagBits2::eShaderSampledRead;
        }
        if (access_flags.Has(AccessFlagBit::eUniformReadFragmentShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eFragmentShader;
            barrier.access |= vk::AccessFlagBits2::eUniformRead;
        }
        if (access_flags.Has(AccessFlagBit::eAccelerationStructureReadFragmentShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eFragmentShader;
            barrier.access |= vk::AccessFlagBits2::eAccelerationStructureReadKHR;
        }
        if (access_flags.Has(AccessFlagBit::eStorageWriteComputeShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eComputeShader;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageWrite;
        }
        if (access_flags.Has(AccessFlagBit::eStorageReadComputeShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eComputeShader;
            barrier.access |= vk::AccessFlagBits2::eShaderStorageRead;
        }
        if (access_flags.Has(AccessFlagBit::eSampledReadComputeShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eComputeShader;
            barrier.access |= vk::AccessFlagBits2::eShaderSampledRead;
        }
        if (access_flags.Has(AccessFlagBit::eUniformReadComputeShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eComputeShader;
            barrier.access |= vk::AccessFlagBits2::eUniformRead;
        }
        if (access_flags.Has(AccessFlagBit::eAccelerationStructureReadComputeShader)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eComputeShader;
            barrier.access |= vk::AccessFlagBits2::eAccelerationStructureReadKHR;
        }
        if (access_flags.Has(AccessFlagBit::eVariableRateShadingRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eFragmentShadingRateAttachmentKHR;
            barrier.access |= vk::AccessFlagBits2::eFragmentShadingRateAttachmentReadKHR;
        }
        if (access_flags.Has(AccessFlagBit::eConditionalRenderingRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eConditionalRenderingEXT;
            barrier.access |= vk::AccessFlagBits2::eConditionalRenderingReadEXT;
        }
        if (access_flags.Has(AccessFlagBit::eShaderBindingTableRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eConditionalRenderingEXT;
            barrier.access |= vk::AccessFlagBits2::eConditionalRenderingReadEXT;
        }
        if (access_flags.Has(AccessFlagBit::eAccelerationStructureBuildRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR;
            barrier.access |= vk::AccessFlagBits2::eAccelerationStructureReadKHR;
        }
        if (access_flags.Has(AccessFlagBit::eAccelerationStructureBuildWrite)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR;
            barrier.access |= vk::AccessFlagBits2::eAccelerationStructureWriteKHR;
        }
        if (access_flags.Has(AccessFlagBit::eAccelerationStructureCopyRead)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eAccelerationStructureCopyKHR;
            barrier.access |= vk::AccessFlagBits2::eAccelerationStructureReadKHR;
        }
        if (access_flags.Has(AccessFlagBit::eAccelerationStructureCopyWrite)) {
            barrier.stage |= vk::PipelineStageFlagBits2::eAccelerationStructureCopyKHR;
            barrier.access |= vk::AccessFlagBits2::eAccelerationStructureWriteKHR;
        }
        return barrier;
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::FrontFace face) noexcept {
        switch (face) {
            case FrontFace::eCounterClockwise: return vk::FrontFace::eCounterClockwise;
            case FrontFace::eClockwise: return vk::FrontFace::eClockwise;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::AttachmentLoadOp op) noexcept {
        switch (op) {
            case AttachmentLoadOp::eNone: return vk::AttachmentLoadOp::eNone;
            case AttachmentLoadOp::eLoad: return vk::AttachmentLoadOp::eLoad;
            case AttachmentLoadOp::eClear: return vk::AttachmentLoadOp::eClear;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::AttachmentStoreOp op) noexcept {
        switch (op) {
            case AttachmentStoreOp::eNone: return vk::AttachmentStoreOp::eNone;
            case AttachmentStoreOp::eStore: return vk::AttachmentStoreOp::eStore;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::StencilOp op) noexcept {
        switch (op) {
            case StencilOp::eKeep: return vk::StencilOp::eKeep;
            case StencilOp::eZero: return vk::StencilOp::eZero;
            case StencilOp::eReplace: return vk::StencilOp::eReplace;
            case StencilOp::eIncrementAndClamp: return vk::StencilOp::eIncrementAndClamp;
            case StencilOp::eDecrementAndClamp: return vk::StencilOp::eDecrementAndClamp;
            case StencilOp::eInvert: return vk::StencilOp::eInvert;
            case StencilOp::eIncrementAndWrap: return vk::StencilOp::eIncrementAndWrap;
            case StencilOp::eDecrementAndWrap: return vk::StencilOp::eDecrementAndWrap;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::IndexType type) noexcept {
        switch (type) {
            case IndexType::eUint16: return vk::IndexType::eUint16;
            case IndexType::eUint32: return vk::IndexType::eUint32;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::BlendFactor factor) noexcept {
        switch (factor) {
            case BlendFactor::eZero: return vk::BlendFactor::eZero;
            case BlendFactor::eOne: return vk::BlendFactor::eOne;
            case BlendFactor::eSrcColor: return vk::BlendFactor::eSrcColor;
            case BlendFactor::eOneMinusSrcColor: return vk::BlendFactor::eOneMinusSrcColor;
            case BlendFactor::eDstColor: return vk::BlendFactor::eDstColor;
            case BlendFactor::eOneMinusDstColor: return vk::BlendFactor::eOneMinusDstColor;
            case BlendFactor::eSrcAlpha: return vk::BlendFactor::eSrcAlpha;
            case BlendFactor::eOneMinusSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
            case BlendFactor::eDstAlpha: return vk::BlendFactor::eDstAlpha;
            case BlendFactor::eOneMinusDstAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
            case BlendFactor::eConstantColor: return vk::BlendFactor::eConstantColor;
            case BlendFactor::eOneMinusConstantColor: return vk::BlendFactor::eOneMinusConstantColor;
            case BlendFactor::eConstantAlpha: return vk::BlendFactor::eConstantAlpha;
            case BlendFactor::eOneMinusConstantAlpha: return vk::BlendFactor::eOneMinusConstantAlpha;
            case BlendFactor::eSrcAlphaSaturate: return vk::BlendFactor::eSrcAlphaSaturate;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::BlendOp op) noexcept {
        switch (op) {
            case BlendOp::eAdd: return vk::BlendOp::eAdd;
            case BlendOp::eSubtract: return vk::BlendOp::eSubtract;
            case BlendOp::eReverseSubtract: return vk::BlendOp::eReverseSubtract;
            case BlendOp::eMinimum: return vk::BlendOp::eMin;
            case BlendOp::eMaximum: return vk::BlendOp::eMax;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::ResolveModeFlagBit flags) noexcept {
        switch (flags) {
            case DnmRHI::ResolveModeFlagBit::eNone: return vk::ResolveModeFlagBits::eNone;
            case DnmRHI::ResolveModeFlagBit::eZero: return vk::ResolveModeFlagBits::eSampleZero;
            case DnmRHI::ResolveModeFlagBit::eAverage: return vk::ResolveModeFlagBits::eAverage;
            case DnmRHI::ResolveModeFlagBit::eMin: return vk::ResolveModeFlagBits::eMin;
            case DnmRHI::ResolveModeFlagBit::eMax: return vk::ResolveModeFlagBits::eMax;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::ResolveModeFlags flags) noexcept {
        vk::ResolveModeFlags vk_flags{};
        if (flags & DnmRHI::ResolveModeFlagBit::eNone) vk_flags |= vk::ResolveModeFlagBits::eNone;
        if (flags & DnmRHI::ResolveModeFlagBit::eZero) vk_flags |= vk::ResolveModeFlagBits::eSampleZero;
        if (flags & DnmRHI::ResolveModeFlagBit::eAverage) vk_flags |= vk::ResolveModeFlagBits::eAverage;
        if (flags & DnmRHI::ResolveModeFlagBit::eMin) vk_flags |= vk::ResolveModeFlagBits::eMin;
        if (flags & DnmRHI::ResolveModeFlagBit::eMax) vk_flags |= vk::ResolveModeFlagBits::eMax;
        return vk_flags;
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::ColorComponentFlags flags) noexcept {
        vk::ColorComponentFlags vk_flags{};
        if (flags & DnmRHI::ColorComponentFlagBit::eR) vk_flags |= vk::ColorComponentFlagBits::eR;
        if (flags & DnmRHI::ColorComponentFlagBit::eG) vk_flags |= vk::ColorComponentFlagBits::eG;
        if (flags & DnmRHI::ColorComponentFlagBit::eB) vk_flags |= vk::ColorComponentFlagBits::eB;
        if (flags & DnmRHI::ColorComponentFlagBit::eA) vk_flags |= vk::ColorComponentFlagBits::eA;
        return vk_flags;
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::AspectFlags flags) noexcept {
        vk::ImageAspectFlags vk_flags{};
        if (flags & DnmRHI::AspectFlagBit::eColor) vk_flags |= vk::ImageAspectFlagBits::eColor;
        if (flags & DnmRHI::AspectFlagBit::eDepth) vk_flags |= vk::ImageAspectFlagBits::eDepth;
        if (flags & DnmRHI::AspectFlagBit::eStencil) vk_flags |= vk::ImageAspectFlagBits::eStencil;
        return vk_flags;
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::ImageSubresourceLayers subresource) noexcept {
        return vk::ImageSubresourceLayers{}
            .setAspectMask(To_Vk(subresource.aspect_flags))
            .setBaseArrayLayer(subresource.base_layer)
            .setLayerCount(subresource.layer_count)
            .setMipLevel(subresource.mipmap_level)
            ;
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::ImageSubresource subresource) noexcept {
        return vk::ImageSubresourceRange{}
            .setAspectMask(To_Vk(subresource.aspect_flags))
            .setBaseArrayLayer(subresource.base_layer)
            .setLayerCount(subresource.layer_count ? subresource.layer_count : VK_REMAINING_ARRAY_LAYERS)
            .setBaseMipLevel(subresource.base_mipmap)
            .setLevelCount(subresource.mipmap_count ? subresource.mipmap_count : VK_REMAINING_MIP_LEVELS)
            ;
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::ShaderStageFlagBit bit) noexcept {
        switch (bit) {
            case DnmRHI::ShaderStageFlagBit::eVertex:       return vk::ShaderStageFlagBits::eVertex;
            case DnmRHI::ShaderStageFlagBit::eFragment:     return vk::ShaderStageFlagBits::eFragment;
            case DnmRHI::ShaderStageFlagBit::eCompute:      return vk::ShaderStageFlagBits::eCompute;
            case DnmRHI::ShaderStageFlagBit::eMesh:         return vk::ShaderStageFlagBits::eMeshEXT;
            case DnmRHI::ShaderStageFlagBit::eTask:         return vk::ShaderStageFlagBits::eTaskEXT;
            case DnmRHI::ShaderStageFlagBit::eAnyHit:          return vk::ShaderStageFlagBits::eAnyHitKHR;
            case DnmRHI::ShaderStageFlagBit::eCallable:     return vk::ShaderStageFlagBits::eCallableKHR;
            case DnmRHI::ShaderStageFlagBit::eClosestHit:   return vk::ShaderStageFlagBits::eClosestHitKHR;
            case DnmRHI::ShaderStageFlagBit::eIntersection: return vk::ShaderStageFlagBits::eIntersectionKHR;
            case DnmRHI::ShaderStageFlagBit::eMiss:         return vk::ShaderStageFlagBits::eMissKHR;
            case DnmRHI::ShaderStageFlagBit::eRaygen:       return vk::ShaderStageFlagBits::eRaygenKHR;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(DnmRHI::ShaderStageFlags flags) noexcept {
        vk::ShaderStageFlags vk_flags{};
        if (flags & DnmRHI::ShaderStageFlagBit::eVertex)       vk_flags |= vk::ShaderStageFlagBits::eVertex;
        if (flags & DnmRHI::ShaderStageFlagBit::eFragment)     vk_flags |= vk::ShaderStageFlagBits::eFragment;
        if (flags & DnmRHI::ShaderStageFlagBit::eCompute)      vk_flags |= vk::ShaderStageFlagBits::eCompute;
        if (flags & DnmRHI::ShaderStageFlagBit::eMesh)         vk_flags |= vk::ShaderStageFlagBits::eMeshEXT;
        if (flags & DnmRHI::ShaderStageFlagBit::eTask)         vk_flags |= vk::ShaderStageFlagBits::eTaskEXT;
        if (flags & DnmRHI::ShaderStageFlagBit::eAnyHit)          vk_flags |= vk::ShaderStageFlagBits::eAnyHitKHR;
        if (flags & DnmRHI::ShaderStageFlagBit::eCallable)     vk_flags |= vk::ShaderStageFlagBits::eCallableKHR;
        if (flags & DnmRHI::ShaderStageFlagBit::eClosestHit)   vk_flags |= vk::ShaderStageFlagBits::eClosestHitKHR;
        if (flags & DnmRHI::ShaderStageFlagBit::eIntersection) vk_flags |= vk::ShaderStageFlagBits::eIntersectionKHR;
        if (flags & DnmRHI::ShaderStageFlagBit::eMiss)         vk_flags |= vk::ShaderStageFlagBits::eMissKHR;
        if (flags & DnmRHI::ShaderStageFlagBit::eRaygen)       vk_flags |= vk::ShaderStageFlagBits::eRaygenKHR;
        return vk_flags;
    }

    [[nodiscard]] constexpr auto To_Vk(VSync vsync) noexcept {
        switch (vsync) {
            case VSync::eImmediate: return vk::PresentModeKHR::eImmediate;
            case VSync::eFifo: return vk::PresentModeKHR::eFifo;
            case VSync::eMailbox: return vk::PresentModeKHR::eMailbox;
            case VSync::eFifoRelexed: return vk::PresentModeKHR::eFifoRelaxed;
            case VSync::eFifoLastestReady: return vk::PresentModeKHR::eFifoLatestReady;
        }
        std::unreachable();
    }

    [[nodiscard]] constexpr auto To_Vk(ImageFlags usage, b8 color_format) noexcept {
        vk::ImageUsageFlags vk_usage{};
        if (usage & ImageFlagBit::eSampled) vk_usage |= vk::ImageUsageFlagBits::eSampled;
        if (usage & ImageFlagBit::eStorage) vk_usage |= vk::ImageUsageFlagBits::eStorage;
        if (usage & ImageFlagBit::eAttachment) color_format ? vk_usage |= vk::ImageUsageFlagBits::eColorAttachment 
                                            : vk_usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        if (usage & ImageFlagBit::eCopySrc) vk_usage |= vk::ImageUsageFlagBits::eTransferSrc;
        if (usage & ImageFlagBit::eCopyDst) vk_usage |= vk::ImageUsageFlagBits::eTransferDst;
        if (usage & ImageFlagBit::eVariableRateShadingAttachment) vk_usage |= vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR;
        return vk_usage;
    }

}