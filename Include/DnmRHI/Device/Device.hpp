#pragma once

#include <DnmRHI/Common/Subresource.hpp>
#include <DnmRHI/Shader/EntryPoint.hpp>
#include <span>
#include <array>

// TODO: temp
namespace slang {
    class ISession;
    class IGlobalSession;
}

namespace DnmRHI {
    struct PresentDesc {
        Swapchain *swapchain;

        constexpr auto &SetSwapchain(Swapchain &swapchain) noexcept { this->swapchain = &swapchain; return *this; }
    };

    struct SemaphoreValueDesc {
        Semaphore *semaphore;
        u64 value;

        constexpr auto &SetSemaphore(Semaphore &semaphore) noexcept { this->semaphore = &semaphore; return *this; }
        constexpr auto &SetValue(u64 value) noexcept { this->value = value; return *this; }
    };

    struct CommandBufferSubmitDesc {
        std::span<CommandBuffer *const> command_buffers;
        std::span<const SemaphoreValueDesc> wait_semaphores;
        std::span<const SemaphoreValueDesc> signal_semaphores;

        constexpr auto &SetCommandBuffer(std::span<CommandBuffer *const> command_buffer) noexcept { this->command_buffers = command_buffer; return *this; }
        constexpr auto &SetWaitSemaphore(std::span<const SemaphoreValueDesc> wait_semaphore) noexcept { this->wait_semaphores = wait_semaphore; return *this; }
        constexpr auto &SetSignalSemaphore(std::span<const SemaphoreValueDesc> signal_semaphore) noexcept { this->signal_semaphores = signal_semaphore; return *this; }
        constexpr auto &SetCommandBuffer(std::initializer_list<CommandBuffer *const> command_buffer) noexcept { this->command_buffers = command_buffer; return *this; }
        constexpr auto &SetWaitSemaphore(std::initializer_list<const SemaphoreValueDesc> wait_semaphore) noexcept { this->wait_semaphores = wait_semaphore; return *this; }
        constexpr auto &SetSignalSemaphore(std::initializer_list<const SemaphoreValueDesc> signal_semaphore) noexcept { this->signal_semaphores = signal_semaphore; return *this; }
    };

    struct WaitSemaphoreDesc {
        std::span<const SemaphoreValueDesc> semaphores; 
        u64 timeout;
        bool any;

        constexpr auto &SetSemaphores(std::span<const SemaphoreValueDesc> semaphores) noexcept { this->semaphores = semaphores; return *this; }
        constexpr auto &SetSemaphores(std::initializer_list<const SemaphoreValueDesc> semaphores) noexcept { this->semaphores = semaphores; return *this; }
        constexpr auto &SetTimeout(u64 timeout) noexcept { this->timeout = timeout; return *this; }
        constexpr auto &SetAny(bool any) noexcept { this->any = any; return *this; }
    };

    struct DeviceFeatures {
        b8 shader_stencil_export: 1;
        b8 mesh_and_task_shader_derivatives: 1;
        b8 ray_tracing_invocation_reorder: 1;
        b8 opacity_micromap: 1;

        b8 has_rebar: 1;
        DeviceType device_type;
    };

    struct DeviceProperties {
        std::string name;
        // TODO: i can use vendor extensions and libs for more detailed device information(cu, sgpr count etc.) 
        GpuVendor vendor;
        u16 subgroup_size;
        u16 min_subgroup_size;
        u16 max_subgroup_size;
    };

    struct DeviceSettings {
        b8 shader_debug: 1{};
        b8 desc_validation: 1{};
        b8 validation_layer: 1{};
        // this can be broke nsight, rgp etc.
        b8 gpu_assisted_validaiton: 1{};
        b8 perfer_fp32_instead_of_fp16:1{};
    };

    struct DeviceInfo {
        DeviceFeatures features;
        DeviceProperties properties;
        std::array<u8, 16> device_uuid;
        std::array<u8, 16> driver_uuid;
    };

    std::vector<DeviceInfo> QuerySupportedD3D12Devices();
    std::vector<DeviceInfo> QuerySupportedVulkanDevices();

    class Device {
    protected:
        explicit Device(DeviceSettings settings) : m_settings(settings) {}
    public:
        virtual ~Device();

        void Init() noexcept;

        [[nodiscard]] u32 GetBindlessStorage(DnmRHI::Image &image, ImageSubresource subresource) noexcept;
        [[nodiscard]] u32 GetBindlessSampled(DnmRHI::Image &image, ImageSubresource subresource) noexcept;
        [[nodiscard]] u32 GetBindlessSampler(const SamplerDesc &desc) noexcept;

        [[nodiscard]] Buffer *CreateBuffer(const BufferDesc &desc) noexcept;
        [[nodiscard]] Image *CreateImage(const ImageDesc &desc) noexcept;
        [[nodiscard]] CommandBuffer *CreateCommandBuffer(const CommandBufferDesc &desc) noexcept;
        [[nodiscard]] GraphicsPipeline *CreateGraphicsPipeline(const GraphicsPipelineDesc &desc) noexcept;
        [[nodiscard]] ComputePipeline *CreateComputePipeline(const ComputePipelineDesc &desc) noexcept;
        [[nodiscard]] Semaphore *CreateSemaphore(const SemaphoreDesc &desc) noexcept;
        [[nodiscard]] Swapchain *CreateSwapchain(const SwapchainDesc &desc) noexcept;
        [[nodiscard]] Shader *CreateShader(const ShaderDesc &desc) noexcept;

        void SubmitCommandBuffer(const CommandBufferSubmitDesc &desc) noexcept;
        void WaitSemaphore(const WaitSemaphoreDesc &desc) noexcept;
        void Present(const PresentDesc &desc) noexcept;

        [[nodiscard]] const auto &GetFeatures() const noexcept { return m_device_features; }
        [[nodiscard]] const auto &GetProperties() const noexcept { return m_device_features; }
        [[nodiscard]] auto GetSettings() const noexcept { return m_settings; }

        [[nodiscard]] auto *GetShaderCompiler() const noexcept { return m_shader_compiler; }

        // helpers
        [[nodiscard]] auto IsNvidia() const noexcept { return m_device_properties.vendor == GpuVendor::eNvidia; }
        [[nodiscard]] auto IsAmd() const noexcept { return m_device_properties.vendor == GpuVendor::eAmd; }
        [[nodiscard]] auto IsIntel() const noexcept { return m_device_properties.vendor == GpuVendor::eIntel; }
        [[nodiscard]] auto IsQualcomm() const noexcept { return m_device_properties.vendor == GpuVendor::eQualcomm; }
    protected:
        virtual void IInit() noexcept = 0;

        [[nodiscard]] virtual DnmRHI::Buffer *ICreateBuffer(const BufferDesc &desc) noexcept = 0;
        [[nodiscard]] virtual DnmRHI::Image *ICreateImage(const ImageDesc &desc) noexcept = 0;
        [[nodiscard]] virtual DnmRHI::CommandBuffer *ICreateCommandBuffer(const CommandBufferDesc &desc) noexcept = 0;
        [[nodiscard]] virtual DnmRHI::GraphicsPipeline *ICreateGraphicsPipeline(const GraphicsPipelineDesc &desc) noexcept = 0;
        [[nodiscard]] virtual DnmRHI::ComputePipeline *ICreateComputePipeline(const ComputePipelineDesc &desc) noexcept = 0;
        [[nodiscard]] virtual DnmRHI::Semaphore *ICreateSemaphore(const SemaphoreDesc &desc) noexcept = 0;
        [[nodiscard]] virtual DnmRHI::Swapchain *ICreateSwapchain(const SwapchainDesc &desc) noexcept = 0;
        [[nodiscard]] virtual DnmRHI::Shader *ICreateShader(const ShaderEntryPoint &entry_point) noexcept = 0;

        [[nodiscard]] virtual u32 IGetBindlessStorage(DnmRHI::Image &image, ImageSubresource subresource) noexcept = 0;
        [[nodiscard]] virtual u32 IGetBindlessSampled(DnmRHI::Image &image, ImageSubresource subresource) noexcept = 0;
        [[nodiscard]] virtual u32 IGetBindlessSampler(const SamplerDesc &desc) noexcept = 0;

        virtual void ISubmitCommandBuffer(const CommandBufferSubmitDesc &desc) noexcept = 0;
        virtual void IWaitSemaphore(const WaitSemaphoreDesc &desc) noexcept = 0;
        virtual void IPresent(const PresentDesc &desc) noexcept = 0;
        [[nodiscard]] virtual slang::ISession *CreateCompilerSession(slang::IGlobalSession *global_session) noexcept = 0;

        DeviceFeatures m_device_features{};
        DeviceProperties m_device_properties{};
        DeviceSettings m_settings{};
        ShaderCompiler *m_shader_compiler{};

        friend ShaderCompiler;
    };
}