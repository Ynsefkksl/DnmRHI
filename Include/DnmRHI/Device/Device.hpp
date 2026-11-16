#pragma once

#include <DnmRHI/Common/Subresource.hpp>
#include <DnmRHI/Common/ShaderStages.hpp>
#include <functional>
#include <span>
#include <array>
#include <string_view>
#include <string>

// TODO: temp
namespace slang {
    struct ISession;
    struct IGlobalSession;
}

namespace DnmRHI {
    enum class MessageSeverity {
        eInfo,
        eWarning,
        eError
    };

    enum class MessageCatagory {
        eGraphicsBackend,
        eShaderCompiler,
        eRHI,
        eUser
    };

    enum class MessageID : u64{};

    using DebugCallbackFn = std::function<void(MessageSeverity severty, MessageCatagory catagory, MessageID message_id, std::string_view message)>;

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
        constexpr auto &SetCommandBuffer(std::initializer_list<CommandBuffer *const> command_buffer) noexcept { this->command_buffers = command_buffer; return *this; }
        constexpr auto &SetWaitSemaphore(std::span<const SemaphoreValueDesc> wait_semaphore) noexcept { this->wait_semaphores = wait_semaphore; return *this; }
        constexpr auto &SetWaitSemaphore(std::initializer_list<const SemaphoreValueDesc> wait_semaphore) noexcept { this->wait_semaphores = wait_semaphore; return *this; }
        constexpr auto &SetSignalSemaphore(std::span<const SemaphoreValueDesc> signal_semaphore) noexcept { this->signal_semaphores = signal_semaphore; return *this; }
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
        // TODO: define up limit
        u32 resource_count{200'000};
        u32 sampler_count{2'000};

        b8 shader_debug: 1{};
        DebugCallbackFn debug_callback{};
        b8 rhi_validation: 1{};
        b8 api_validation_layer: 1{};
        b8 gpu_assisted_validaiton: 1{};
        
        constexpr auto &SetResourceCount(u32 resource_count) noexcept { this->resource_count = resource_count; return *this; }
        constexpr auto &SetSamplerCount(u32 sampler_count) noexcept { this->sampler_count = sampler_count; return *this; }

        constexpr auto &SetDebugCallback(DebugCallbackFn callback) noexcept { this->debug_callback = std::move(callback); return *this; }
        constexpr auto &SetRHIValidation(b8 rhi_validation) noexcept { this->rhi_validation = rhi_validation; return *this; }
        constexpr auto &SetApiValidationLayer(b8 api_validation_layer) noexcept { this->api_validation_layer = api_validation_layer; return *this; }
        constexpr auto &SetGpuAssistedValidation(b8 gpu_assisted_validaiton) noexcept { this->gpu_assisted_validaiton = gpu_assisted_validaiton; return *this; }
    };

    struct DeviceInfo {
        GraphicsApi api;
        DeviceFeatures features;
        DeviceProperties properties;
        std::array<u8, 16> device_uuid;
        std::array<u8, 16> driver_uuid;
    };

    [[nodiscard]] std::vector<DeviceInfo> QuerySupportedD3D12Devices() noexcept;
    [[nodiscard]] std::vector<DeviceInfo> QuerySupportedVulkanDevices() noexcept;

    // device_info must be same with QuerySupported*Devices output
    [[nodiscard]] ResultOr<Device *> CreateD3D12Device(const DeviceInfo &device_info, DeviceSettings device_settings, const D3D12Settings &d3d12_settings) noexcept;
    [[nodiscard]] ResultOr<Device *> CreateVulkanDevice(const DeviceInfo &device_info, DeviceSettings device_settings, const VulkanSettings &vk_settings) noexcept;
    [[nodiscard]] ResultOr<Device *> CreateDevice(const DeviceInfo &device_info, DeviceSettings device_settings) noexcept;

    class Device {
    protected:
        explicit Device(const DeviceInfo &info, DeviceSettings settings) : m_device_info(info), m_settings(settings) {
            if (m_settings.gpu_assisted_validaiton)
                m_settings.api_validation_layer = true;

            if (m_settings.api_validation_layer)
                m_settings.rhi_validation = true;
        }
    public:
        virtual ~Device();

        Result Init() noexcept;
        Result WaitIdle() noexcept;

        [[nodiscard]] ResultOr<Buffer *> CreateBuffer(const BufferDesc &desc) noexcept;
        [[nodiscard]] ResultOr<Image *> CreateImage(const ImageDesc &desc) noexcept;
        [[nodiscard]] ResultOr<CommandBuffer *> CreateCommandBuffer(const CommandBufferDesc &desc) noexcept;
        [[nodiscard]] ResultOr<GraphicsPipeline *> CreateGraphicsPipeline(const GraphicsPipelineDesc &desc) noexcept;
        [[nodiscard]] ResultOr<ComputePipeline *> CreateComputePipeline(const ComputePipelineDesc &desc) noexcept;
        [[nodiscard]] ResultOr<Semaphore *> CreateSemaphore(const SemaphoreDesc &desc) noexcept;
        [[nodiscard]] ResultOr<Swapchain *> CreateSwapchain(const SwapchainDesc &desc) noexcept;

        [[nodiscard]] ResultOr<std::vector<u8>> CompileShader(const ShaderDesc &desc) noexcept;

        Result SetUniformBufferBindlessIndex(u32 index, DnmRHI::Buffer &buffer, Subresource subresource) noexcept;
        Result SetSampledImageBindlessIndex(u32 index, DnmRHI::Image &image, Subresource subresource) noexcept;
        Result SetStorageImageBindlessIndex(u32 index, DnmRHI::Image &image, Subresource subresource) noexcept;
        Result SetSamplerBindlessIndex(u32 index, const SamplerDesc &sampler) noexcept;

        Result SubmitCommandBuffer(const CommandBufferSubmitDesc &desc) noexcept;
        Result WaitSemaphore(const WaitSemaphoreDesc &desc) noexcept;
        Result Present(const PresentDesc &desc) noexcept;

        u32 ShaderHeaderMagic() const noexcept { return IShaderHeaderMagic(); }

        [[nodiscard]] const auto &GetFeatures() const noexcept { return m_device_info.features; }
        [[nodiscard]] const auto &GetProperties() const noexcept { return m_device_info.properties; }
        [[nodiscard]] const auto &GetDeviceInfo() const noexcept { return m_device_info; }
        [[nodiscard]] auto GetSettings() const noexcept { return m_settings; }

        void Message(MessageSeverity severty, MessageCatagory catagory, MessageID message_id, std::string_view message) {
            if (m_settings.debug_callback) m_settings.debug_callback(severty, catagory, message_id, message);
        }
    protected:
        virtual Result IInit() noexcept = 0;
        virtual Result IWaitIdle() noexcept = 0;

        [[nodiscard]] virtual ResultOr<DnmRHI::Buffer *> ICreateBuffer(const BufferDesc &desc) noexcept = 0;
        [[nodiscard]] virtual ResultOr<DnmRHI::Image *> ICreateImage(const ImageDesc &desc) noexcept = 0;
        [[nodiscard]] virtual ResultOr<DnmRHI::CommandBuffer *> ICreateCommandBuffer(const CommandBufferDesc &desc) noexcept = 0;
        [[nodiscard]] virtual ResultOr<DnmRHI::GraphicsPipeline *> ICreateGraphicsPipeline(const GraphicsPipelineDesc &desc) noexcept = 0;
        [[nodiscard]] virtual ResultOr<DnmRHI::ComputePipeline *> ICreateComputePipeline(const ComputePipelineDesc &desc) noexcept = 0;
        [[nodiscard]] virtual ResultOr<DnmRHI::Semaphore *> ICreateSemaphore(const SemaphoreDesc &desc) noexcept = 0;
        [[nodiscard]] virtual ResultOr<DnmRHI::Swapchain *> ICreateSwapchain(const SwapchainDesc &desc) noexcept = 0;

        virtual Result ISetUniformBufferBindlessIndex(u32 index, DnmRHI::Buffer &buffer, Subresource subresource) noexcept = 0;
        virtual Result ISetSampledImageBindlessIndex(u32 index, DnmRHI::Image &image, Subresource subresource) noexcept = 0;
        virtual Result ISetStorageImageBindlessIndex(u32 index, DnmRHI::Image &image, Subresource subresource) noexcept = 0;
        virtual Result ISetSamplerBindlessIndex(u32 index, const SamplerDesc &sampler) noexcept = 0;

        virtual Result ISubmitCommandBuffer(const CommandBufferSubmitDesc &desc) noexcept = 0;
        virtual Result IWaitSemaphore(const WaitSemaphoreDesc &desc) noexcept = 0;
        virtual Result IPresent(const PresentDesc &desc) noexcept = 0;

        virtual u32 IShaderHeaderMagic() const noexcept = 0;

        DeviceInfo m_device_info{};
        DeviceSettings m_settings{};
        ShaderCompiler *m_shader_compiler{};

        slang::ISession *m_compiler_session;
        slang::IGlobalSession *m_compiler_global_session;

        friend ShaderCompiler;
    };
}