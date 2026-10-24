#include <DnmRHI/Device/Device.hpp>
#include <DnmRHI/Shader/Compiler.hpp>
#include <DnmRHI/Shader/Desc.hpp>

namespace DnmRHI {
    Device::~Device() {
        delete m_shader_compiler;
    }

    void Device::Init() noexcept { 
        // TODO: check something
        m_shader_compiler = new ShaderCompiler(*this); 
        IInit(); 
    }

    Buffer *Device::CreateBuffer(const BufferDesc &desc) noexcept {
        // TODO: check if desc is valid
        return ICreateBuffer(desc);
    }

    Image *Device::CreateImage(const ImageDesc &desc) noexcept {
        // TODO: check if desc is valid
        return ICreateImage(desc);
    }

    CommandBuffer *Device::CreateCommandBuffer(const CommandBufferDesc &desc) noexcept {
        // TODO: check if desc is valid
        return ICreateCommandBuffer(desc);
    }

    GraphicsPipeline *Device::CreateGraphicsPipeline(const GraphicsPipelineDesc &desc) noexcept {
        // TODO: check if desc is valid
        return ICreateGraphicsPipeline(desc);
    }

    ComputePipeline *Device::CreateComputePipeline(const ComputePipelineDesc &desc) noexcept {
        // TODO: check if desc is valid
        return ICreateComputePipeline(desc);
    }

    Semaphore *Device::CreateSemaphore(const SemaphoreDesc &desc) noexcept {
        // TODO: check if desc is valid
        return ICreateSemaphore(desc);
    }

    Swapchain *Device::CreateSwapchain(const SwapchainDesc &desc) noexcept {
        // TODO: check if desc is valid
        return ICreateSwapchain(desc);
    }

    Shader *Device::CreateShader(const ShaderDesc &desc) noexcept {
        // TODO: check if desc is valid
        return ICreateShader(m_shader_compiler->GetEntryPointBinary(desc.file, desc.entry_point));
    }

    u32 Device::GetBindlessStorage(Image &image, ImageSubresource subresource) noexcept {
        // TODO: check if parameters is valid
        return IGetBindlessStorage(image, subresource);
    }

    u32 Device::GetBindlessSampled(Image &image, ImageSubresource subresource) noexcept {
        // TODO: check if parameters is valid
        return IGetBindlessSampled(image, subresource);
    }

    u32 Device::GetBindlessSampler(const SamplerDesc &desc) noexcept {
        // TODO: check if desc is valid
        return IGetBindlessSampler(desc);
    }

    void Device::SubmitCommandBuffer(const CommandBufferSubmitDesc &desc) noexcept {
        // TODO: check if desc is valid
        ISubmitCommandBuffer(desc);
    }

    void Device::WaitSemaphore(const WaitSemaphoreDesc &desc) noexcept {
        // TODO: check if parameters is valid
        
        IWaitSemaphore(desc);
    }

    void Device::Present(const PresentDesc &desc) noexcept {
        // TODO: check if parameters is valid
        IPresent(desc);
    }
}