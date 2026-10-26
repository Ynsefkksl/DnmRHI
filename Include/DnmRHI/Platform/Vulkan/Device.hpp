#pragma once

#include <DnmRHI/Common/QueueType.hpp>
#include <DnmRHI/Image/SampleCount.hpp>
#include <DnmRHI/Device/Device.hpp>
#include <DnmRHI/Device/VulkanSettings.hpp>
#include <DnmRHI/Platform/Vulkan/ToVk.hpp>

#define SET_DEBUG_NAME_VK_OBJECT(device, object, debug_name) if (VULKAN_HPP_DEFAULT_DISPATCHER.vkSetDebugUtilsObjectNameEXT && debug_name) device.setDebugUtilsObjectNameEXT( \
    vk::DebugUtilsObjectNameInfoEXT{object.objectType, reinterpret_cast<const uint64_t&>(object), debug_name});

#define TO_C_PTR(vk_obj) reinterpret_cast<decltype(vk_obj)::NativeType *>(&vk_obj)
#define TO_C_CONST_PTR(vk_obj) reinterpret_cast<const decltype(vk_obj)::NativeType *>(&vk_obj)

#define GetVkDevice (static_cast<::DnmRHI::Vk::Device &>(GetDevice()))
#define GetVkDeviceProperties GetVkDevice.GetVulkanDeviceProperties()
#define GetVkDeviceFeatures GetVkDevice.GetVulkanDeviceFeatures()

typedef struct VpFunctions_T *VpFunctions;

namespace DnmRHI::Vk {
    class DescriptorManager;
    class Image;
    class Buffer;
    class Shader;
    class ComputePipeline;
    class GraphicsPipeline;
    class RayTracingPipeline;
    class CommandBuffer;
    class Semaphore;

    struct DeviceFeatures {
        b8 win32_surface: 1;
        b8 xcb_surface: 1;
        b8 xlib_surface: 1;
        b8 wayland_surface: 1;
        b8 surface_maintenance1: 1;

        b8 descirptor_heap: 1;
        b8 maintenance9: 1;
        b8 pageable_device_local_memory: 1;
        b8 unified_image_layouts: 1;
        b8 input_output_16: 1;
        b8 swapchain_maintenance1: 1;
    };

    struct DeviceProperties {
        std::array<u8, VK_UUID_SIZE> pipeline_cache_uuid;
        std::array<vk::SampleCountFlagBits, 4> sample_map{};

        u32 sampled_image_descriptor_size;
        u32 storage_image_descriptor_size;
        u32 sampler_descriptor_size;
        u32 ubo_image_descriptor_size;

        u32 image_descriptor_size_for_heap;
        u32 buffer_descriptor_size_for_heap;
        u32 resource_descriptor_size_for_heap;
        u32 sampler_descriptor_size_for_heap;
        u32 resource_heap_min_reserve;
        u32 sampler_heap_min_reserve;
        u32 sampler_heap_alignment;
        u32 resource_heap_alignment;
    };

    class Device final : public DnmRHI::Device {
    public:
        explicit Device(const DeviceInfo &info, DeviceSettings device_settings, const VulkanSettings &settings) noexcept;
        ~Device() noexcept;

        virtual void IInit() noexcept override;

        [[nodiscard]] virtual DnmRHI::Buffer *ICreateBuffer(const BufferDesc &desc) noexcept override;
        [[nodiscard]] virtual DnmRHI::Image *ICreateImage(const ImageDesc &desc) noexcept override;
        [[nodiscard]] virtual DnmRHI::CommandBuffer *ICreateCommandBuffer(const CommandBufferDesc &desc) noexcept override;
        [[nodiscard]] virtual DnmRHI::GraphicsPipeline *ICreateGraphicsPipeline(const GraphicsPipelineDesc &desc) noexcept override;
        [[nodiscard]] virtual DnmRHI::ComputePipeline *ICreateComputePipeline(const ComputePipelineDesc &desc) noexcept override;
        [[nodiscard]] virtual DnmRHI::Semaphore *ICreateSemaphore(const SemaphoreDesc &desc) noexcept override;
        [[nodiscard]] virtual DnmRHI::Swapchain *ICreateSwapchain(const SwapchainDesc &desc) noexcept override;
        [[nodiscard]] virtual DnmRHI::Shader *ICreateShader(const ShaderEntryPoint &entry_point) noexcept override;

        [[nodiscard]] virtual u32 IGetBindlessStorage(DnmRHI::Image &image, ImageSubresource subresource) noexcept override;
        [[nodiscard]] virtual u32 IGetBindlessSampled(DnmRHI::Image &image, ImageSubresource subresource) noexcept override;
        [[nodiscard]] virtual u32 IGetBindlessSampler(const SamplerDesc &desc) noexcept override;

        virtual void ISubmitCommandBuffer(const CommandBufferSubmitDesc &desc) noexcept override;
        virtual void IWaitSemaphore(const WaitSemaphoreDesc &desc) noexcept override;
        virtual void IPresent(const PresentDesc &desc) noexcept override;
        [[nodiscard]] virtual slang::ISession *CreateCompilerSession(slang::IGlobalSession *global_session) noexcept override;

        [[nodiscard]] auto GetInstance() const noexcept { return m_instance; }
        [[nodiscard]] auto GetPhysicalDevice() const noexcept { return m_physical_device; }
        [[nodiscard]] auto GetDevice() const noexcept { return m_device; }
        [[nodiscard]] auto *GetAllocator() const noexcept { return m_vma_allocator; }

        [[nodiscard]] auto GetGeneralQueue() const noexcept { return m_general_queue; }
        [[nodiscard]] auto GetComputeQueue() const noexcept { return m_compute_queue; }
        [[nodiscard]] auto GetCopyQueue() const noexcept { return m_copy_queue; }

        [[nodiscard]] auto GetGeneralQueueFamilyIndex() const noexcept { return m_general_queue_family_index; }
        [[nodiscard]] auto GetComputeQueueFamilyIndex() const noexcept { return m_compute_queue_family_index; }
        [[nodiscard]] auto GetCopyQueueFamilyIndex() const noexcept { return m_copy_queue_family_index; }

        [[nodiscard]] auto GetQueue(QueueType type) const noexcept {
            switch (type) {
                case QueueType::eGeneral: return m_general_queue;
                case QueueType::eCompute: return m_compute_queue;
                case QueueType::eCopy: return m_copy_queue;
                default: std::unreachable();
            }
        }

        [[nodiscard]] auto GetQueueFamilyIndex(QueueType type) const noexcept {
            switch (type) {
                case QueueType::eGeneral: return m_general_queue_family_index;
                case QueueType::eCompute: return m_compute_queue_family_index;
                case QueueType::eCopy: return m_copy_queue_family_index;
                default: std::unreachable();
            }
        }

        [[nodiscard]] auto *GetDescriptorManager() const noexcept { return m_descriptor_manager; }

        [[nodiscard]] auto GetVulkanSettings() const noexcept { return m_vk_settings; }
        [[nodiscard]] auto GetVulkanDeviceProperties() const noexcept { return m_vk_device_properties; }
        [[nodiscard]] auto GetVulkanDeviceFeatures() const noexcept { return m_vk_device_features; }

        [[nodiscard]] auto GetSamples(DnmRHI::SampleCount sample) const noexcept {
            return m_vk_device_properties.sample_map[static_cast<u8>(sample)];
        }
    private:
        void CreateInstance(const VpFunctions &vp_functions) noexcept;
        void CreateDevice(const VpFunctions &vp_functions) noexcept;
        void CreateVmaAllocator() noexcept;
        void CreateDescriptorManager() noexcept;

        // i back maunel check
        static b8 CheckGPUSupportAndGetFeatures(vk::PhysicalDevice device, DeviceInfo &device_info, 
                b8 just_check, Vk::DeviceFeatures vk_features, std::vector<const char *> &extensions, void *&features);

        vk::detail::DynamicLoader m_dl;
        vk::Instance m_instance;
        vk::DebugUtilsMessengerEXT m_debug_messenger;
        vk::PhysicalDevice m_physical_device;
        vk::Device m_device;

        vk::Queue m_general_queue;
        vk::Queue m_compute_queue;
        vk::Queue m_copy_queue;

        u32 m_general_queue_family_index;
        u32 m_compute_queue_family_index;
        u32 m_copy_queue_family_index;

        VmaAllocator m_vma_allocator;
        DescriptorManager *m_descriptor_manager;

        VulkanSettings m_vk_settings;
        DeviceFeatures m_vk_device_features;
        DeviceProperties m_vk_device_properties;

        friend DnmRHI::Device *CreateVulkanDevice(const DeviceInfo &device_info, DeviceSettings device_settings, const VulkanSettings &vk_settings);
    };
}