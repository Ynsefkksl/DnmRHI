#include <DnmRHI/Device/Device.hpp>
#include <DnmRHI/Device/D3D12Settings.hpp>
#include <DnmRHI/Device/VulkanSettings.hpp>
#include <DnmRHI/Shader/Compiler.hpp>
#include <DnmRHI/Shader/Desc.hpp>

namespace DnmRHI {
    #if !defined(DnmRHI_Backend_D3D12)
    [[nodiscard]] Device *CreateD3D12Device(const DeviceInfo &device_info, DeviceSettings device_settings, const D3D12Settings &d3d12_settings) {
        return nullptr;
    }
    #endif

    #if !defined(DnmRHI_Backend_Vulkan)
        [[nodiscard]] Device *CreateVulkanDevice(const DeviceInfo &device_info, DeviceSettings device_settings, const VulkanSettings &vk_settings) {
            return nullptr;
        }
    #endif

    Device *CreateDevice(const DeviceInfo &device_info, DeviceSettings device_settings) {
        switch (device_info.api) {
            case GraphicsApi::eVulkan: return CreateVulkanDevice(device_info, device_settings, {});
            case GraphicsApi::eD3D12: return CreateD3D12Device(device_info, device_settings, {});
        }
        std::unreachable();
    }

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

    void Device::SetUniformBufferBindlessIndex(u32 index, DnmRHI::Buffer &buffer, Subresource subresource) noexcept {
        // TODO: check if desc is valid

        ISetUniformBufferBindlessIndex(index, buffer, subresource);
    }

    void Device::SetSampledImageBindlessIndex(u32 index, DnmRHI::Image &image, Subresource subresource) noexcept {
        // TODO: check if desc is valid

        ISetSampledImageBindlessIndex(index, image, subresource);
    }

    void Device::SetStorageImageBindlessIndex(u32 index, DnmRHI::Image &image, Subresource subresource) noexcept {
        // TODO: check if desc is valid

        ISetStorageImageBindlessIndex(index, image, subresource);
    }

    void Device::SetSamplerBindlessIndex(u32 index, const SamplerDesc &sampler) noexcept {
        // TODO: check if desc is valid

        ISetSamplerBindlessIndex(index, sampler);
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