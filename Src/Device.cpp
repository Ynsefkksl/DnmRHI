#include <DnmRHI/Common/FormatInfo.hpp>
#include <DnmRHI/Device/Device.hpp>
#include <DnmRHI/Device/D3D12Settings.hpp>
#include <DnmRHI/Device/VulkanSettings.hpp>
#include <DnmRHI/Shader/Desc.hpp>
#include <DnmRHI/Buffer/Buffer.hpp>
#include <DnmRHI/Image/Image.hpp>
#include <DnmRHI/CommandBuffer/CommandBuffer.hpp>
#include <DnmRHI/GraphicsPipeline/GraphicsPipeline.hpp>
#include <DnmRHI/ComputePipeline/ComputePipeline.hpp>
#include <DnmRHI/Semaphore/Semaphore.hpp>
#include <DnmRHI/Swapchain/Swapchain.hpp>
#include <DnmRHI/ImageView/ImageView.hpp>
#include <DnmRHI/Shader/ByteCodeHeader.hpp>

#include <slang/slang.h>
#include <slang/slang-com-helper.h>
#include <slang/slang-com-ptr.h>

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

    static constexpr ShaderStageFlagBit ToSlangShaderStageFlagBits(SlangStage stage) noexcept {
        switch (stage) {
            case SLANG_STAGE_VERTEX:        return ShaderStageFlagBit::eVertex;
            case SLANG_STAGE_FRAGMENT:      return ShaderStageFlagBit::eFragment;
            case SLANG_STAGE_COMPUTE:       return ShaderStageFlagBit::eCompute;
            case SLANG_STAGE_MESH:          return ShaderStageFlagBit::eMesh;
            case SLANG_STAGE_AMPLIFICATION: return ShaderStageFlagBit::eTask;
            case SLANG_STAGE_ANY_HIT:       return ShaderStageFlagBit::eAnyHit;
            case SLANG_STAGE_CALLABLE:      return ShaderStageFlagBit::eCallable;
            case SLANG_STAGE_CLOSEST_HIT:   return ShaderStageFlagBit::eClosestHit;
            case SLANG_STAGE_INTERSECTION:  return ShaderStageFlagBit::eIntersection;
            case SLANG_STAGE_MISS:          return ShaderStageFlagBit::eMiss;
            case SLANG_STAGE_RAY_GENERATION:return ShaderStageFlagBit::eRaygen;
            default:                        return ShaderStageFlagBit{};
        }
    }

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

        if (desc.shader_byte_code.empty()) {
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

        {
            ShaderStageFlags stages;

            for (const auto shader : desc.shaders) {
                if (shader.empty()) {
                    is_valid = false;
                    break;
                }

                const auto *shader_header = reinterpret_cast<const DnmRHI::ShaderByteCodeHeader *>(shader.data());
                if (shader_header->magic != device.ShaderHeaderMagic()) {
                    is_valid = false;
                    break;
                }

                if (stages.Has(shader_header->entry_point_name_length == 0)) {
                    is_valid = false;
                    break;
                }

                if (stages.Has(shader_header->stage)) {
                    is_valid = false;
                    break;
                }

                stages |= shader_header->stage;
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
        
        if (desc.queue_type == QueueType::Count) {
            is_valid = false;
        }

        return is_valid;
    }

    static constexpr b8 CheckDesc(Device &device, const SemaphoreDesc &desc) noexcept {
        b8 is_valid = true;
        return is_valid;
    }

    // TODO: check swapchain desc
    static constexpr b8 CheckDesc(Device &device, const SwapchainDesc &desc) noexcept {
        b8 is_valid = true;
        return is_valid;
    }

    static constexpr b8 CheckDesc(Device &device, const ShaderDesc &desc) noexcept {
        b8 is_valid = true;
        
        if (desc.byte_code.empty()) {
            is_valid = false;
        }

        if (desc.entry_point.empty()) {
            is_valid = false;
        }

        if (desc.module_name.empty()) {
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

    static constexpr b8 CheckDesc(Device &device, ImageViewDesc &desc) noexcept {
        b8 is_valid = true;

        if (desc.image == nullptr) {
            is_valid = false;
        }

        if (CheckImageSubresource(device, *desc.image, desc.subresource)) {
            is_valid = false;
        }

        // TODO: check image view type

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
        m_compiler_global_session->release();
        m_compiler_session->release();
    }

    Result Device::Init() noexcept {
        if (GetSettings().rhi_validation) {
            // is already initialized
            if (m_compiler_session)
                return Result::eInvalidUsage;
        }

        return IInit(); 
    }

    Result Device::WaitIdle() noexcept {
        return IWaitIdle();
    }

    ResultOr<Buffer *> Device::CreateBuffer(const BufferDesc &desc) noexcept {
        if (GetSettings().rhi_validation) {
            if (!CheckDesc(*this, desc))
                return std::unexpected(Result::eInvalidParameter);
        }

        return ICreateBuffer(desc);
    }

    ResultOr<Image *> Device::CreateImage(const ImageDesc &desc) noexcept {
        if (GetSettings().rhi_validation) {
            if (!CheckDesc(*this, desc))
                return std::unexpected(Result::eInvalidParameter);
        }

        return ICreateImage(desc);
    }

    ResultOr<CommandBuffer *> Device::CreateCommandBuffer(const CommandBufferDesc &desc) noexcept {
        if (GetSettings().rhi_validation) {
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
        if (GetSettings().rhi_validation) {
            if (!CheckDesc(*this, desc))
                return std::unexpected(Result::eInvalidParameter);
        }
        return ICreateSemaphore(desc);
    }

    ResultOr<Swapchain *> Device::CreateSwapchain(const SwapchainDesc &desc) noexcept {
        if (GetSettings().rhi_validation) {
            if (!CheckDesc(*this, desc))
                return std::unexpected(Result::eInvalidParameter);
        }
        return ICreateSwapchain(desc);
    }

    ResultOr<ImageView *> Device::CreateImageView(const ImageViewDesc &desc) noexcept {
        ImageViewDesc desc_copy{desc};
        if (GetSettings().rhi_validation) {
            if (!CheckDesc(*this, desc_copy))
                return std::unexpected(Result::eInvalidParameter);
        }
        return ICreateImageView(desc_copy);
    }

    ResultOr<std::vector<u8>> Device::CompileShader(const ShaderDesc &desc) noexcept {
        Slang::ComPtr<slang::IModule> slang_module;

        {
            Slang::ComPtr<slang::IBlob> diagnostics_blob;
            slang_module = m_compiler_session->loadModuleFromSourceString(
                desc.module_name.data(), 
                nullptr, 
                reinterpret_cast<const char*>(desc.byte_code.data()), 
                diagnostics_blob.writeRef());
            if (diagnostics_blob) {
                // TODO: fix massage ids
                Message(!slang_module ? MessageSeverity::eError : MessageSeverity::eWarning, MessageCatagory::eShaderCompiler, 
                    MessageID{0}, std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
                if (!slang_module) {
                    return std::unexpected(Result::eCompileError);
                }
            }
        }

        Slang::ComPtr<slang::IEntryPoint> entry_point;
        slang_module->findEntryPointByName(desc.entry_point.data(), entry_point.writeRef());

        slang::IComponentType *component_types[] = {
            slang_module,
            entry_point
        };

        Slang::ComPtr<slang::IComponentType> composed_program;
        {
            Slang::ComPtr<slang::IBlob> diagnostics_blob;
            
            m_compiler_session->createCompositeComponentType(
                component_types,
                2,
                composed_program.writeRef(),
                diagnostics_blob.writeRef());

            if (diagnostics_blob) {
                // TODO: fix massage ids
                Message(!composed_program ? MessageSeverity::eError : MessageSeverity::eWarning, MessageCatagory::eShaderCompiler, 
                    MessageID{0}, std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
                if (!composed_program) {
                    return std::unexpected(Result::eCompileError);
                }
            }
        }

        Slang::ComPtr<slang::IComponentType> linked_program;
        {
            Slang::ComPtr<slang::IBlob> diagnostics_blob;
            SlangResult result = composed_program->link(
                linked_program.writeRef(),
                diagnostics_blob.writeRef());

            if (diagnostics_blob) {
                // TODO: fix massage ids
                Message(!linked_program ? MessageSeverity::eError : MessageSeverity::eWarning, MessageCatagory::eShaderCompiler, 
                    MessageID{0}, std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
                if (!linked_program) {
                    return std::unexpected(Result::eCompileError);
                }
            }
        }

        auto *entry_point_refl = linked_program->getLayout()->findEntryPointByName(desc.entry_point.data());
        // for mangling
        u32 entry_point_name_size = strlen(entry_point_refl->getName());

        Slang::ComPtr<slang::IBlob> code_blob;
        linked_program->getEntryPointCode(0, 0, code_blob.writeRef());

        std::vector<u8> out(code_blob->getBufferSize() + sizeof(ShaderByteCodeHeader) + entry_point_name_size + 1 /* for null terminator */);

        auto *shader_header = reinterpret_cast<ShaderByteCodeHeader *>(out.data());
        shader_header->magic = ShaderHeaderMagic();
        shader_header->stage = ToSlangShaderStageFlagBits(entry_point_refl->getStage());
        shader_header->entry_point_name_length = entry_point_name_size;

        std::memcpy(shader_header->entry_point_name, entry_point_refl->getName(), entry_point_name_size);
        // for null terminator
        std::memcpy(shader_header->entry_point_name + entry_point_name_size, 0, 1);

        const u32 total_header_size = sizeof(ShaderByteCodeHeader) + entry_point_name_size + 1 /* for null terminator*/;
        std::memcpy(out.data() + total_header_size, code_blob->getBufferPointer(), code_blob->getBufferSize());

        return out;
    }

    Result Device::WriteUniformBufferDescriptor(u32 index, DnmRHI::Buffer &buffer, Subresource subresource) noexcept {
        BufferSubresource subres(subresource);
        
        if (GetSettings().rhi_validation) {
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

        return IWriteUniformBufferDescriptor(index, buffer, subres);
    }

    Result Device::WriteSampledImageDescriptor(u32 index, DnmRHI::ImageView &image_view) noexcept {
        if (GetSettings().rhi_validation) {
            if (m_settings.resource_count <= index) {
                return Result::eInvalidParameter;
            }
        }

        return IWriteSampledImageDescriptor(index, image_view);
    }

    Result Device::WriteStorageImageDescriptor(u32 index, DnmRHI::ImageView &image_view) noexcept {
        if (GetSettings().rhi_validation) {
            if (m_settings.resource_count <= index) {
                return Result::eInvalidParameter;
            }
        }

        return IWriteStorageImageDescriptor(index, image_view);
    }

    Result Device::WriteSamplerDescriptor(u32 index, const SamplerDesc &sampler) noexcept {
        if (GetSettings().rhi_validation) {
            if (m_settings.sampler_count <= index) {
                return Result::eInvalidParameter;
            }
        }

        return IWriteSamplerDescriptor(index, sampler);
    }

    Result Device::SubmitCommandBuffer(const CommandBufferSubmitDesc &desc) noexcept {
        if (GetSettings().rhi_validation) {
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
        if (GetSettings().rhi_validation) {
            for (const auto &wait_semaphore : desc.semaphores) {
                if (!wait_semaphore.semaphore) {
                    return Result::eInvalidParameter;
                }
            }
        }
        
        return IWaitSemaphore(desc);
    }

    Result Device::Present(const PresentDesc &desc) noexcept {
        if (GetSettings().rhi_validation) {
            if (!desc.swapchain) {
                return Result::eInvalidParameter;
            }
        }

        return IPresent(desc);
    }
}