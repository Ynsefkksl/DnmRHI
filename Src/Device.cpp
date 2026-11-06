#include "DnmRHI/Common/FormatInfo.hpp"
#include <DnmRHI/Device/Device.hpp>
#include <DnmRHI/Device/D3D12Settings.hpp>
#include <DnmRHI/Device/VulkanSettings.hpp>
#include <DnmRHI/Shader/Compiler.hpp>
#include <DnmRHI/Shader/Desc.hpp>

#include <DnmRHI/Buffer/Buffer.hpp>
#include <DnmRHI/Image/Image.hpp>
#include <DnmRHI/CommandBuffer/CommandBuffer.hpp>
#include <DnmRHI/GraphicsPipeline/GraphicsPipeline.hpp>
#include <DnmRHI/ComputePipeline/ComputePipeline.hpp>
#include <DnmRHI/Semaphore/Semaphore.hpp>
#include <DnmRHI/Swapchain/Swapchain.hpp>
#include <DnmRHI/Shader/Shader.hpp>

namespace DnmRHI {
    #if !defined(DnmRHI_Backend_D3D12)
        [[nodiscard]] std::vector<DeviceInfo> QuerySupportedD3D12Devices() noexcept {
            return {};
        }

        [[nodiscard]] ResultOr<Device *> CreateD3D12Device(const DeviceInfo &device_info, DeviceSettings device_settings, const D3D12Settings &d3d12_settings) noexcept {
            return std::unexpected(Result::eInvalidParameter);
        }
    #endif

    #if !defined(DnmRHI_Backend_Vulkan)
        [[nodiscard]] std::vector<DeviceInfo> QuerySupportedVulkanDevices() noexcept {
            return {};
        }

        [[nodiscard]] ResultOr<Device *> CreateVulkanDevice(const DeviceInfo &device_info, DeviceSettings device_settings, const VulkanSettings &vk_settings) noexcept {
            return std::unexpected(Result::eInvalidParameter);
        }
    #endif

    // TODO: maybe these have own file
    static constexpr b8 CheckDesc(Device &device, const BufferDesc &desc) noexcept {
        b8 is_valid = true;
        
        if (desc.size == 0) {
            is_valid = false;
        }
        if (desc.flags.None()) {
            is_valid = false;
        }
        // min align must 256 bytes for uniform buffer, d3d12 requirement
        if (desc.flags.Has(BufferFlagBit::eUniform) && (desc.align >> 8) != 0) {
            is_valid = false;
        }
        // min align must 64 bytes for storage buffer
        if (desc.flags.Has(BufferFlagBit::eStorage) && (desc.align >> 6) != 0) {
            is_valid = false;
        }

        return is_valid;
    }

    static constexpr b8 CheckDesc(Device &device, const ImageDesc &desc) noexcept {
        b8 is_valid = true;
        
        if (desc.extent.width == 0 || desc.extent.height == 0 || desc.extent.depth == 0) {
            is_valid = false;
        }
        if (desc.flags.None()) {
            is_valid = false;
        }
        if (desc.format == Format::eUndefined) {
            is_valid = false;
        }
        if (desc.mipmap_count == 0) {
            is_valid = false;
        }
        if (desc.type == ImageType::e3D && desc.flags.Has(ImageFlagBit::eStorage)) {
            is_valid = false;
        }
        if (
            desc.flags.Has(DnmRHI::ImageFlagBit::eGeneralQueue)
            | desc.flags.Has(DnmRHI::ImageFlagBit::eComputeQueue)
            | desc.flags.Has(DnmRHI::ImageFlagBit::eCopyQueue)
        ) {
            const b8 has_general_queue = desc.flags.Has(DnmRHI::ImageFlagBit::eGeneralQueue);
            const b8 has_compute_queue = desc.flags.Has(DnmRHI::ImageFlagBit::eComputeQueue);
            const b8 has_copy_queue = desc.flags.Has(DnmRHI::ImageFlagBit::eCopyQueue);

            if (!has_general_queue) {
                if (desc.flags.Has(ImageFlagBit::eAttachment) || desc.flags.Has(ImageFlagBit::eVariableRateShadingAttachment)) {
                    is_valid = false;
                }
            }

            if (!has_general_queue && !has_compute_queue) {
                if (desc.flags.Has(ImageFlagBit::eStorage) | desc.flags.Has(ImageFlagBit::eSampled)) {
                    is_valid = false;
                }
            }
        }

        return is_valid;
    }

    static constexpr b8 CheckDesc(Device &device, const ComputePipelineDesc &desc) noexcept {
        b8 is_valid = true;

        if (!desc.compute_shader) {
            is_valid = false;
        }
        
        return is_valid;
    }

    static constexpr b8 CheckDesc(Device &device, const GraphicsPipelineDesc &desc) noexcept {
        b8 is_valid = true;
        
        for (const auto &color_output_state : desc.color_output_state.color_attachment_formats) {
            if (color_output_state.format == Format::eUndefined) {
                is_valid = false;
                break;
            }
        }

        {
            if (desc.rasterization_state.viewport_count == 0) {
                is_valid = false;
            }

            if (desc.rasterization_state.scissor_count == 0) {
                is_valid = false;
            }
        }

        // TODO: shader input outputs is valid
        {
            ShaderStageFlags stages;

            for (const auto *shader : desc.shaders) {
                if (!shader) {
                    is_valid = false;
                    break;
                }

                if (stages.Has(shader->GetEntryPointInfo().flag_bit)) {
                    is_valid = false;
                    break;
                }

                stages |= shader->GetEntryPointInfo().flag_bit;
            }

            if (!(stages.Has(ShaderStageFlagBit::eVertex | ShaderStageFlagBit::eFragment)
                | stages.Has(ShaderStageFlagBit::eTask | ShaderStageFlagBit::eMesh | ShaderStageFlagBit::eFragment)
                | stages.Has(ShaderStageFlagBit::eMesh | ShaderStageFlagBit::eFragment)
            )) {
                is_valid = false;
            }
        }

        return is_valid;
    }

    static constexpr b8 CheckDesc(Device &device, const CommandBufferDesc &desc) noexcept {
        b8 is_valid = true;
        
        if (desc.queue_type == QueueType::eNone || desc.queue_type == QueueType::Count) {
            is_valid = false;
        }

        return is_valid;
    }

    static constexpr b8 CheckDesc(Device &device, const SemaphoreDesc &desc) noexcept {
        b8 is_valid = true;
        return is_valid;
    }

    static constexpr b8 CheckDesc(Device &device, const SwapchainDesc &desc) noexcept {
        b8 is_valid = true;
        return is_valid;
    }

    static constexpr b8 CheckDesc(Device &device, const ShaderDesc &desc) noexcept {
        b8 is_valid = true;
        
        if (!std::filesystem::exists(desc.file)) {
            is_valid = false;
        }

        return is_valid;
    }

    static constexpr b8 CheckBufferSubresource(Device &device, const Buffer &buffer, BufferSubresource &subres) noexcept {
        b8 is_valid = true;

        if (subres.offset + subres.size > buffer.GetDesc().size) {
            is_valid = false;
        }

        if (subres.size > buffer.GetDesc().size) {
            subres.size = buffer.GetDesc().size;
        }

        return is_valid;
    }

    static constexpr b8 CheckImageSubresource(Device &device, const Image &image, ImageSubresource &subres) noexcept {
        b8 is_valid = true;
        
        {
            if (subres.base_mipmap + subres.mipmap_count > image.GetDesc().mipmap_count) {
                is_valid = false;
            }
    
            if (subres.mipmap_count > image.GetDesc().mipmap_count) {
                subres.mipmap_count = image.GetDesc().mipmap_count;
            }
        }

        if (image.GetDesc().type == ImageType::e2D) {
            if (subres.base_layer + subres.layer_count > image.GetDesc().extent.depth) {
                is_valid = false;
            }
    
            if (subres.layer_count > image.GetDesc().extent.depth) {
                subres.layer_count = image.GetDesc().extent.depth;
            }
        }
        else {
            if (!(subres.base_layer == 0 && subres.layer_count == 1)) {
                is_valid = false;
            }
        }

        if (!(image.GetDesc().type == ImageType::e3D && subres.type == ImageSubresourceType::e3D)) {
            is_valid = false;
        }

        if (image.GetDesc().type == ImageType::e2D && subres.type == ImageSubresourceType::e3D) {
            is_valid = false;
        }

        if (subres.layer_count != 6 && subres.type == ImageSubresourceType::eCube) {
            is_valid = false;
        }

        if (subres.layer_count % 6 != 0 && subres.type == ImageSubresourceType::eCubeArray) {
            is_valid = false;
        }

        if (subres.aspect_flags.None()) {
            if (IsDepthStencilFormat(image.GetDesc().format)) {
                subres.aspect_flags = AspectFlagBit::eDepth | AspectFlagBit::eStencil;
            }
            else if (IsDepthFormat(image.GetDesc().format)) {
                subres.aspect_flags = AspectFlagBit::eDepth;
            }
            else {
                subres.aspect_flags = AspectFlagBit::eColor;
            }
        }

        return is_valid;
    }

    ResultOr<Device *> CreateDevice(const DeviceInfo &device_info, DeviceSettings device_settings) noexcept {
        switch (device_info.api) {
            case GraphicsApi::eVulkan: return CreateVulkanDevice(device_info, device_settings, {});
            case GraphicsApi::eD3D12: return CreateD3D12Device(device_info, device_settings, {});
        }
        std::unreachable();
    }

    Device::~Device() {
        delete m_shader_compiler;
    }

    Result Device::Init() noexcept {
        if (GetSettings().debug_mode) {
            // is already initialized
            if (m_shader_compiler)
                return Result::eInvalidUsage;
        }

        m_shader_compiler = new ShaderCompiler(*this); 
        return IInit(); 
    }

    Result Device::WaitIdle() noexcept {
        return IWaitIdle();
    }

    ResultOr<Buffer *> Device::CreateBuffer(const BufferDesc &desc) noexcept {
        if (GetSettings().debug_mode) {
            if (!CheckDesc(*this, desc))
                return std::unexpected(Result::eInvalidParameter);
        }

        return ICreateBuffer(desc);
    }

    ResultOr<Image *> Device::CreateImage(const ImageDesc &desc) noexcept {
        if (GetSettings().debug_mode) {
            if (!CheckDesc(*this, desc))
                return std::unexpected(Result::eInvalidParameter);
        }

        return ICreateImage(desc);
    }

    ResultOr<CommandBuffer *> Device::CreateCommandBuffer(const CommandBufferDesc &desc) noexcept {
        if (GetSettings().debug_mode) {
            if (!CheckDesc(*this, desc))
                return std::unexpected(Result::eInvalidParameter);
        }
        return ICreateCommandBuffer(desc);
    }

    ResultOr<GraphicsPipeline *> Device::CreateGraphicsPipeline(const GraphicsPipelineDesc &desc) noexcept {
        if (!CheckDesc(*this, desc))
            return std::unexpected(Result::eInvalidParameter);

        return ICreateGraphicsPipeline(desc);
    }

    ResultOr<ComputePipeline *> Device::CreateComputePipeline(const ComputePipelineDesc &desc) noexcept {
        if (!CheckDesc(*this, desc))
            return std::unexpected(Result::eInvalidParameter);

        return ICreateComputePipeline(desc);
    }

    ResultOr<Semaphore *> Device::CreateSemaphore(const SemaphoreDesc &desc) noexcept {
        if (GetSettings().debug_mode) {
            if (!CheckDesc(*this, desc))
                return std::unexpected(Result::eInvalidParameter);
        }
        return ICreateSemaphore(desc);
    }

    ResultOr<Swapchain *> Device::CreateSwapchain(const SwapchainDesc &desc) noexcept {
        if (GetSettings().debug_mode) {
            if (!CheckDesc(*this, desc))
                return std::unexpected(Result::eInvalidParameter);
        }
        return ICreateSwapchain(desc);
    }

    ResultOr<Shader *> Device::CreateShader(const ShaderDesc &desc) noexcept {
        if (GetSettings().debug_mode) {
            if (!CheckDesc(*this, desc))
                return std::unexpected(Result::eInvalidParameter);
        }

        const auto result_or_entry_point = m_shader_compiler->GetEntryPointBinary(desc.file, desc.entry_point);

        if (!result_or_entry_point.has_value()) {
            return std::unexpected(result_or_entry_point.error());
        }

        return ICreateShader(*result_or_entry_point.value());
    }

    Result Device::SetUniformBufferBindlessIndex(u32 index, DnmRHI::Buffer &buffer, Subresource subresource) noexcept {
        BufferSubresource subres(subresource);
        
        if (GetSettings().debug_mode) {
            if (m_settings.resource_count <= index) {
                return Result::eInvalidParameter;
            }

            if (!buffer.GetDesc().flags.Has(BufferFlagBit::eUniform)) {
                return Result::eInvalidParameter;
            }

            if (CheckBufferSubresource(*this, buffer, subres)) {
                return Result::eInvalidParameter;
            }
        }

        return ISetUniformBufferBindlessIndex(index, buffer, subres);
    }

    Result Device::SetSampledImageBindlessIndex(u32 index, DnmRHI::Image &image, Subresource subresource) noexcept {
        ImageSubresource subres(subresource);

        if (GetSettings().debug_mode) {
            if (m_settings.resource_count <= index) {
                return Result::eInvalidParameter;
            }
    
            if (!image.GetDesc().flags.Has(ImageFlagBit::eSampled)) {
                return Result::eInvalidParameter;
            }
    
            if (!CheckImageSubresource(*this, image, subres)) {
                return Result::eInvalidParameter;
            }
        }

        return ISetSampledImageBindlessIndex(index, image, subres);
    }

    Result Device::SetStorageImageBindlessIndex(u32 index, DnmRHI::Image &image, Subresource subresource) noexcept {
        if (GetSettings().debug_mode) {
            if (m_settings.resource_count <= index) {
                return Result::eInvalidParameter;
            }

            if (!image.GetDesc().flags.Has(ImageFlagBit::eStorage)) {
                return Result::eInvalidParameter;
            }

            ImageSubresource subres(subresource);
            if (!CheckImageSubresource(*this, image, subres)) {
                return Result::eInvalidParameter;
            }
        }

        return ISetStorageImageBindlessIndex(index, image, subresource);
    }

    Result Device::SetSamplerBindlessIndex(u32 index, const SamplerDesc &sampler) noexcept {
        if (GetSettings().debug_mode) {
            if (m_settings.sampler_count <= index) {
                return Result::eInvalidParameter;
            }
        }

        return ISetSamplerBindlessIndex(index, sampler);
    }

    Result Device::SubmitCommandBuffer(const CommandBufferSubmitDesc &desc) noexcept {
        if (GetSettings().debug_mode) {
            QueueType queue_type{};
            
            if (desc.command_buffers.size() > 0) {
                queue_type = desc.command_buffers[0]->GetDesc().queue_type;
            }

            for (const auto *command_buffer : desc.command_buffers) {
                if (!command_buffer) {
                    return Result::eInvalidParameter;
                }

                if (command_buffer->GetDesc().queue_type != queue_type) {
                    return Result::eInvalidParameter;
                }
            }

            for (const auto &wait_semaphore : desc.wait_semaphores) {
                if (!wait_semaphore.semaphore) {
                    return Result::eInvalidParameter;
                }
            }

            for (const auto &signal_semaphore : desc.signal_semaphores) {
                if (!signal_semaphore.semaphore) {
                    return Result::eInvalidParameter;
                }
            }
        }

        return ISubmitCommandBuffer(desc);
    }

    Result Device::WaitSemaphore(const WaitSemaphoreDesc &desc) noexcept {
        if (GetSettings().debug_mode) {
            for (const auto &wait_semaphore : desc.semaphores) {
                if (!wait_semaphore.semaphore) {
                    return Result::eInvalidParameter;
                }
            }
        }
        
        return IWaitSemaphore(desc);
    }

    Result Device::Present(const PresentDesc &desc) noexcept {
        if (GetSettings().debug_mode) {
            if (!desc.swapchain) {
                return Result::eInvalidParameter;
            }
        }

        return IPresent(desc);
    }
}