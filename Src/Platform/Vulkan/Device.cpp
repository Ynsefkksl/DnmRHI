#include <DnmRHI/Platform/Vulkan/ComputePipeline.hpp>
#include <DnmRHI/Platform/Vulkan/GraphicsPipeline.hpp>
#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Platform/Vulkan/DescriptorManager.hpp>
#include <DnmRHI/Platform/Vulkan/Semaphore.hpp>
#include <DnmRHI/Platform/Vulkan/CommandBuffer.hpp>
#include <DnmRHI/Platform/Vulkan/Swapchain.hpp>
#include <DnmRHI/Platform/Vulkan/Shader.hpp>
#include <DnmRHI/Platform/Vulkan/vulkan_profiles.h>

#include <set>
#include <unordered_set>
#include <vector>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#define ADD_P_NEXT(struct_name) do {struct_name.pNext = p_next; \
			                    p_next = &struct_name;} while(0)
#define FEATURE_MUST_BE_SUPPORTED(feature) if (b8(feature) != true) { is_suitable = false; /* Warn("GPU is unsupported, must be support " #feature); */ }
#define PROPERTY_MUST_BE_GTE(property, value) if (property < value) { is_suitable = false; /* Warn("GPU is unsupported, " #property " greater than " #value ", your value: {}", value); */ }
#define PROPERTY_MUST_BE_SUPPORTED(property) if (b8(property) != true) { is_suitable = false; /* Warn("GPU is unsupported, must be support " #property " property"); */ }

// TODO:
namespace DnmRHI {
    [[nodiscard]] static VpFunctions CreateVpFunctions(vk::detail::DynamicLoader &dyn_loader) {
        VpFunctionsCreateInfo create_info{};
        create_info.GetInstanceProcAddr                     = dyn_loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        create_info.EnumerateInstanceVersion                = dyn_loader.getProcAddress<PFN_vkEnumerateInstanceVersion>("vkEnumerateInstanceVersion");
        create_info.EnumerateInstanceExtensionProperties    = dyn_loader.getProcAddress<PFN_vkEnumerateInstanceExtensionProperties>("vkEnumerateInstanceExtensionProperties");
        create_info.EnumerateDeviceExtensionProperties      = dyn_loader.getProcAddress<PFN_vkEnumerateDeviceExtensionProperties>("vkEnumerateDeviceExtensionProperties");
        create_info.CreateInstance                          = dyn_loader.getProcAddress<PFN_vkCreateInstance>("vkCreateInstance");
        create_info.CreateDevice                            = dyn_loader.getProcAddress<PFN_vkCreateDevice>("vkCreateDevice");
        create_info.GetPhysicalDeviceFeatures2              = dyn_loader.getProcAddress<PFN_vkGetPhysicalDeviceFeatures2>("vkGetPhysicalDeviceFeatures2");
        create_info.GetPhysicalDeviceProperties2            = dyn_loader.getProcAddress<PFN_vkGetPhysicalDeviceProperties2>("vkGetPhysicalDeviceProperties2");
        create_info.GetPhysicalDeviceFormatProperties2      = dyn_loader.getProcAddress<PFN_vkGetPhysicalDeviceFormatProperties2>("vkGetPhysicalDeviceFormatProperties2");
        create_info.GetPhysicalDeviceQueueFamilyProperties2 = dyn_loader.getProcAddress<PFN_vkGetPhysicalDeviceQueueFamilyProperties2>("vkGetPhysicalDeviceQueueFamilyProperties2");

        VpFunctions functions = VK_NULL_HANDLE;
        auto result = vpCreateFunctions(&create_info, nullptr, &functions);
        if (result != VK_SUCCESS) return {};
        
        return functions;
    }

    [[nodiscard]] static std::vector<const char *> GetSurfaceExtensions(std::span<const vk::ExtensionProperties> instance_extensions) {
        std::vector<const char *> out;

        for (const auto &extension : instance_extensions) {
            const auto extension_name = std::string(extension.extensionName);

            if (extension_name == "VK_KHR_win32_surface")
                out.emplace_back("VK_KHR_win32_surface");
            else if (extension_name == "VK_KHR_wayland_surface")
                out.emplace_back("VK_KHR_wayland_surface");
            else if (extension_name == "VK_KHR_xcb_surface")
                out.emplace_back("VK_KHR_xcb_surface");
            else if (extension_name == "VK_KHR_xlib_surface")
                out.emplace_back("VK_KHR_xlib_surface");
        }

        return out;
    }

    struct QueryDispatcher {
        constexpr u32 getVkHeaderVersion() const noexcept { return VK_HEADER_VERSION; }

        #define DEFINE_VK_FUNC(name) PFN_##name name
        #define SET_VK_FUNC(name) name = dyn_loader.getProcAddress<PFN_##name>(#name)

        QueryDispatcher(vk::detail::DynamicLoader &dyn_loader) {
            SET_VK_FUNC(vkEnumerateInstanceExtensionProperties);
            SET_VK_FUNC(vkEnumeratePhysicalDevices);
            SET_VK_FUNC(vkGetPhysicalDeviceMemoryProperties);
            SET_VK_FUNC(vkGetPhysicalDeviceProperties2);
        }
        DEFINE_VK_FUNC(vkEnumerateInstanceExtensionProperties);
        DEFINE_VK_FUNC(vkEnumeratePhysicalDevices);
        DEFINE_VK_FUNC(vkGetPhysicalDeviceMemoryProperties);
        DEFINE_VK_FUNC(vkGetPhysicalDeviceProperties2);

        #undef DEFINE_VK_FUNC
        #undef SET_VK_FUNC
    };

    [[nodiscard]] static DeviceInfo GetDeviceInfo(vk::PhysicalDevice device, b8 supported, std::span<const VpBlockProperties> block_properties, QueryDispatcher &query_dispatcher) {
        DeviceInfo device_info_out{};

        {
            vk::PhysicalDeviceVulkan13Properties vk13{};
            vk::PhysicalDeviceVulkan12Properties vk12{};
            vk12.pNext = &vk13;
            vk::PhysicalDeviceVulkan11Properties vk11{};
            vk11.pNext = &vk12;
            vk::PhysicalDeviceProperties2 properties{};
            properties.pNext = &vk11;
            device.getProperties2(&properties, query_dispatcher);
    
            switch (properties.properties.deviceType) {
                case vk::PhysicalDeviceType::eOther: device_info_out.features.device_type = DeviceType::eUnknown; break;
                case vk::PhysicalDeviceType::eIntegratedGpu: device_info_out.features.device_type = DeviceType::eIntegred; break;
                case vk::PhysicalDeviceType::eDiscreteGpu: device_info_out.features.device_type = DeviceType::eDiscrete; break;
                case vk::PhysicalDeviceType::eVirtualGpu: device_info_out.features.device_type = DeviceType::eVirtual; break;
                case vk::PhysicalDeviceType::eCpu: device_info_out.features.device_type = DeviceType::eCpu; break;
            }
            
            if (device_info_out.features.device_type == DeviceType::eDiscrete) {
                const auto memory_properties = device.getMemoryProperties(query_dispatcher);
                for (const auto mem_type : memory_properties.memoryTypes)
                    if (mem_type.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible)
                        device_info_out.features.has_rebar |= (memory_properties.memoryHeaps[mem_type.heapIndex].size > 1 << 30); // 1GB
            }

            device_info_out.device_uuid = vk11.deviceUUID;
            device_info_out.driver_uuid = vk11.driverUUID;
            device_info_out.properties.name = properties.properties.deviceName.data();

            switch (properties.properties.vendorID) {
                case 0x10DE: device_info_out.properties.vendor = GpuVendor::eNvidia; break;
                case 0x1002: device_info_out.properties.vendor = GpuVendor::eAmd; break;
                case 0x8086: device_info_out.properties.vendor = GpuVendor::eIntel; break;
                case 0x5143: device_info_out.properties.vendor = GpuVendor::eQualcomm; break;
                default: device_info_out.properties.vendor = GpuVendor::eUnknown; break;
            }

            device_info_out.properties.subgroup_size = vk11.subgroupSize;
            device_info_out.properties.min_subgroup_size = vk13.minSubgroupSize;
            device_info_out.properties.max_subgroup_size = vk13.maxSubgroupSize;
        }

        {
            std::set<std::string> blocks;
            for (const auto &prop : block_properties)
                blocks.emplace(std::string(prop.blockName));

            device_info_out.features.mesh_and_task_shader_derivatives 
                = supported == blocks.contains("MeshAndTaskShaderDerivatives");

            device_info_out.features.shader_stencil_export 
                = supported == blocks.contains("ShaderStecilExport");

            device_info_out.features.ray_tracing_invocation_reorder 
                = supported == blocks.contains("RayTracingInvocationReorder");

            device_info_out.features.opacity_micromap
                = supported == blocks.contains("OpacityMicromap");
        }

        return device_info_out;
    } 

    std::vector<DeviceInfo> QuerySupportedVulkanDevices() {
        vk::detail::DynamicLoader dyn_loader;
        QueryDispatcher query_dispatcher(dyn_loader);

        const auto properties =
            VpProfileProperties{"VP_DNMRHI_VulkanRequirements"}; 

        const auto vp_functions = CreateVpFunctions(dyn_loader);

        VkBool32 supported{};

        u32 block_count{};
        vpGetInstanceProfileVariantsSupport(vp_functions, nullptr, &properties, &supported, &block_count, nullptr);
        std::vector<VpBlockProperties> block_properties(block_count);
        vpGetInstanceProfileVariantsSupport(vp_functions, nullptr, &properties, &supported, &block_count, block_properties.data());

        // if instance has no surface ext return null
        {
            const auto extensions = vk::enumerateInstanceExtensionProperties(nullptr, query_dispatcher);

            auto surface_exts = GetSurfaceExtensions(extensions);

            if (surface_exts.empty()) return {};
        }

        const auto app_info
            = vk::ApplicationInfo{}
                .setApplicationVersion(VK_MAKE_VERSION(0, 1, 0))
                .setEngineVersion(VK_MAKE_VERSION(0, 1, 0))
                .setApiVersion(VK_API_VERSION_1_4)
                .setPEngineName("DnmEngine")
                .setPApplicationName("query_device_support")
                ;

        const auto instance_create_info
            = vk::InstanceCreateInfo{}
                .setPApplicationInfo(&app_info);

        VpInstanceCreateInfo vp_instance_create_info{};
        vp_instance_create_info.pCreateInfo = reinterpret_cast<const VkInstanceCreateInfo *>(&instance_create_info);
        vp_instance_create_info.enabledFullProfileCount = 1;
        vp_instance_create_info.pEnabledFullProfiles = &properties;
        vp_instance_create_info.enabledProfileBlockCount = block_properties.size();
        vp_instance_create_info.pEnabledProfileBlocks = block_properties.data();

        vk::Instance instance;
        auto result = vpCreateInstance(vp_functions, &vp_instance_create_info, nullptr, reinterpret_cast<VkInstance *>(&instance));
        if (result != VK_SUCCESS) return {};

        {
            std::vector<DeviceInfo> device_infos;
            const auto devices = instance.enumeratePhysicalDevices(query_dispatcher);

            for (const auto device : devices) {
                u32 device_block_count{};
                vpGetPhysicalDeviceProfileVariantsSupport(vp_functions, instance, device, &properties, &supported, &device_block_count, nullptr);
                std::vector<VpBlockProperties> device_block_properties(device_block_count);
                vpGetPhysicalDeviceProfileVariantsSupport(vp_functions, instance, device, &properties, &supported, &device_block_count, device_block_properties.data());

                device_infos.emplace_back(
                    GetDeviceInfo(device, supported == VK_TRUE, device_block_properties, query_dispatcher)
                );
            }

            return device_infos;
        }
    }
}

namespace DnmRHI::Vk {
    namespace Utility {
        template <typename T>
        static b8 FeaturesIsSupported(const auto &driver_report_features, void *&p_next, std::initializer_list<vk::Bool32 T:: *> members) noexcept {
            if constexpr (std::is_same_v<T, vk::PhysicalDeviceFeatures>) {
                auto new_ = vk::PhysicalDeviceFeatures2{}.setPNext(p_next);
                for (auto member : members) {
                    if (new_.features.*member && !(driver_report_features.template get<vk::PhysicalDeviceFeatures2>().features.*member))
                        return false;
    
                    new_.features.*member = vk::True;
                }
    
                p_next = malloc(sizeof(vk::PhysicalDeviceFeatures2));
                memcpy(p_next, &new_, sizeof(vk::PhysicalDeviceFeatures2));
            }
            else {
                auto new_ = T{}.setPNext(p_next);
                for (auto member : members) {
                    if (new_.*member && !(driver_report_features.template get<T>().*member))
                        return false;
    
                    new_.*member = vk::True;
                }
    
                p_next = malloc(sizeof(T));
                memcpy(p_next, &new_, sizeof(T));
            }
            return true;
        }

        template <typename T>
        static T *FindInPNext(void *p_next) {
            auto *chain = reinterpret_cast<vk::BaseOutStructure *>(p_next);
            while (chain->pNext != nullptr) {
                if (chain->sType == T::structureType) return reinterpret_cast<T *>(chain);
                chain = chain->pNext;
            }
            return nullptr;
        }

        constexpr Sync GetSyncFromUsageFlags(ImageFlags flags) {
            Sync sync{};
            sync.access |= vk::AccessFlagBits2::eTransferRead;
            sync.stage |= vk::PipelineStageFlagBits2::eTransfer;

            if (flags.Has(DnmRHI::ImageFlagBit::eStorage)) {
                sync.access |= vk::AccessFlagBits2::eShaderStorageWrite | vk::AccessFlagBits2::eShaderStorageRead;
                sync.stage |= vk::PipelineStageFlagBits2::eAllGraphics;
            }
            if (flags.Has(DnmRHI::ImageFlagBit::eAttachment)) {
                sync.access |= vk::AccessFlagBits2::eColorAttachmentWrite | vk::AccessFlagBits2::eColorAttachmentRead;
                sync.stage |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            }
            if (flags.Has(DnmRHI::ImageFlagBit::eCopyDst)) {
                sync.access |= vk::AccessFlagBits2::eTransferWrite;
                sync.stage |= vk::PipelineStageFlagBits2::eTransfer;
            }
            if (flags.Has(DnmRHI::ImageFlagBit::eSampled)) {
                sync.access |= vk::AccessFlagBits2::eShaderSampledRead;
                sync.stage |= vk::PipelineStageFlagBits2::eAllGraphics;
            }

            return sync;
        }
    }

    static vk::Bool32 DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                vk::DebugUtilsMessageTypeFlagsEXT message_types,
                                const vk::DebugUtilsMessengerCallbackDataEXT *callback_data,
                                void *user_data) {
        constexpr auto unnecessary_messages = std::array{306759332, 2132353751, 2132353751, -2030147807, 601872502, 615892639, 1734198062};
        for (const i32 i : unnecessary_messages)
            if (callback_data->messageIdNumber == i) return vk::True;

        if (message_severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
            // // Error("Vulkan // Error; message id {}, message: {}", callback_data->messageIdNumber, callback_data->pMessage);
        }
        else if (message_severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
            // Warn("Vulkan Warn; message id {}, message: {}", callback_data->messageIdNumber, callback_data->pMessage);
        }
        else {
            // Info("Vulkan Info; message id {}, message: {}", callback_data->messageIdNumber, callback_data->pMessage);
        }
        return vk::True;
    }

    class RequiredExtensions {
    public:
        RequiredExtensions(std::initializer_list<const char *> exts) : m_required_exts(exts) {}
        RequiredExtensions(std::vector<const char *> exts) : m_required_exts(exts) {}

        void AddExtension(const char *ext) noexcept { m_required_exts.emplace_back(ext); }
        void AddExtension(std::vector<const char *> exts) noexcept { m_required_exts.insert_range(m_required_exts.end(), exts); }

        b8 CheckExtensions(std::span<const vk::ExtensionProperties> supported_extensions, std::vector<std::string> &unsupported_exts) const noexcept {
            unsupported_exts.clear();
            for (const auto *required_ext : m_required_exts) {
                b8 is_supported{};
                for (const auto &supported_ext : supported_extensions) {
                    if (std::strcmp(required_ext, supported_ext.extensionName.data()) == 0) {
                        is_supported = true;
                        break;
                    }
                }

                if (!is_supported)
                    unsupported_exts.emplace_back(required_ext);
            }
            return unsupported_exts.empty();
        }

        b8 CheckLayers(std::span<const vk::LayerProperties> supported_extensions, std::vector<std::string> &unsupported_exts) const noexcept {
            unsupported_exts.clear();
            for (const auto *required_ext : m_required_exts) {
                b8 is_supported{};
                for (const auto &supported_ext : supported_extensions) {
                    if (std::strcmp(required_ext, supported_ext.layerName.data()) == 0) {
                        is_supported = true;
                        break;
                    }
                }

                if (!is_supported)
                    unsupported_exts.emplace_back(required_ext);
            }
            return unsupported_exts.empty();
        }

        std::vector<const char *> GetExtensions() const noexcept { return m_required_exts; }
    private:
        std::vector<const char *> m_required_exts;
    };

    class OptionalExtensions {
    public:
        OptionalExtensions(std::initializer_list<const char *> exts) : m_optional_exts(exts) {}

        void AddExtension(const char *ext) noexcept { m_optional_exts.emplace_back(ext); }
        void AddExtension(std::vector<const char *> exts) noexcept { m_optional_exts.insert_range(m_optional_exts.end(), exts); }

        b8 CheckExtensions(std::span<const vk::ExtensionProperties> supported_extensions, std::unordered_set<std::string> &supported_exts) noexcept {
            supported_exts.clear();

            for (const auto *optional_ext : m_optional_exts) {
                b8 is_supported{};
                for (const auto &supported_ext : supported_extensions) {
                    if (std::strcmp(optional_ext, supported_ext.extensionName.data()) == 0) {
                        is_supported = true;
                        break;
                    }
                }

                if (is_supported) {
                    supported_exts.emplace(optional_ext);
                    m_supported_exts.emplace_back(optional_ext);
                }
            }
            return supported_exts.empty();
        }

        std::vector<const char *> GetExtensions() const noexcept {
            return m_supported_exts;
        }
    private:
        std::vector<const char *> m_optional_exts;
        std::vector<const char *> m_supported_exts;
    };

    Device::Device(DeviceSettings device_settings, const VulkanSettings &settings) noexcept 
    : DnmRHI::Device(device_settings), m_vk_settings(settings) {}

    Device::~Device() noexcept {
        GetDevice().waitIdle();

        if (m_descriptor_manager) delete m_descriptor_manager;
        if (m_vma_allocator) vmaDestroyAllocator(m_vma_allocator);
        if (m_debug_messenger) m_instance.destroy(m_debug_messenger);
        if (m_device) m_device.destroy();

        if (m_instance) m_instance.destroy();
    }

    void Device::IInit() noexcept {
        CreateInstance();
        CreateDevice();
        CreateVmaAllocator();
        CreateDescriptorManager();
    }

    b8 Device::GetInstanceFeatures(std::vector<const char *> &supported_ext, std::vector<const char *> &layers, InstanceFeatures &instance_features) {
        const auto supported_extensions = vk::enumerateInstanceExtensionProperties();

        supported_ext.clear();
        layers.clear();
        b8 is_suitable{true};

        if (GetSettings().validation_layer) {
            const auto supported_layers = vk::enumerateInstanceLayerProperties();

            // TODO: better name
            RequiredExtensions required_layers({
                "VK_LAYER_KHRONOS_validation",
                "VK_LAYER_KHRONOS_synchronization2"
            });

            if (std::vector<std::string> unsupported_layers;
                !required_layers.CheckLayers(supported_layers, unsupported_layers)) {
                    // // Error("Vulkan Instance Creation Failed, unsupported layers: {}", unsupported_layers);
                    is_suitable = false;
                    return false;
                }

            layers.insert_range(layers.end(), required_layers.GetExtensions());
        }

        {
            RequiredExtensions required_ext({
                VK_KHR_SURFACE_EXTENSION_NAME
            });
            if (GetSettings().validation_layer) required_ext.AddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    
            if (std::vector<std::string> unsupported_exts;
                !required_ext.CheckExtensions(supported_extensions, unsupported_exts)) {
                    // Error("Vulkan Instance Creation Failed, unsupported extensions: {}", unsupported_exts);
                    is_suitable = false;
                    return false;
                }

            supported_ext.insert_range(supported_ext.end(), required_ext.GetExtensions());
        }

        {
            OptionalExtensions optional_ext({
                "VK_KHR_wayland_surface",
                "VK_KHR_xcb_surface",
                "VK_KHR_win32_surface",
                "VK_EXT_surface_maintenance1",
                "VK_KHR_get_surface_capabilities2"
            });

            std::unordered_set<std::string> supported_optional_ext;
            optional_ext.CheckExtensions(supported_extensions, supported_optional_ext);

            instance_features.wayland_surface = supported_optional_ext.contains("VK_KHR_wayland_surface");
            instance_features.win32_surface = supported_optional_ext.contains("VK_KHR_win32_surface");
            instance_features.xcb_surface = supported_optional_ext.contains("VK_KHR_xcb_surface");
            instance_features.surface_maintenance1 = supported_optional_ext.contains("VK_KHR_get_surface_capabilities2") && supported_optional_ext.contains("VK_EXT_surface_maintenance1");

            if (!(instance_features.wayland_surface 
            || instance_features.win32_surface 
            || instance_features.xcb_surface)) {
                // Error("Vulkan Instance Creation Failed, instance must be support one of the surface extensions: {}", std::array{"VK_KHR_wayland_surface", "VK_KHR_win32_surface", "VK_KHR_xcb_surface"});
                is_suitable = false;
                return false;
            }

            supported_ext.insert_range(supported_ext.end(), optional_ext.GetExtensions());
        }
        return true;
    }

    static b8 CheckAndGetDeviceFeatures(vk::PhysicalDevice device, DeviceInfo &device_info_out, std::vector<const char *> &supported_ext_out, void *&feature_chain) {
        std::vector<const char *> supported_ext{};
        b8 is_suitable{true};

        {
            vk::PhysicalDeviceVulkan13Properties vk13{};
            vk::PhysicalDeviceVulkan12Properties vk12{};
            vk12.pNext = &vk13;
            vk::PhysicalDeviceVulkan11Properties vk11{};
            vk11.pNext = &vk12;
            vk::PhysicalDeviceProperties2 properties{};
            properties.pNext = &vk11;
            device.getProperties2(&properties);
    
            switch (properties.properties.deviceType) {
                case vk::PhysicalDeviceType::eOther: device_info_out.features.device_type = DeviceType::eUnknown; break;
                case vk::PhysicalDeviceType::eIntegratedGpu: device_info_out.features.device_type = DeviceType::eIntegred; break;
                case vk::PhysicalDeviceType::eDiscreteGpu: device_info_out.features.device_type = DeviceType::eDiscrete; break;
                case vk::PhysicalDeviceType::eVirtualGpu: device_info_out.features.device_type = DeviceType::eVirtual; break;
                case vk::PhysicalDeviceType::eCpu: device_info_out.features.device_type = DeviceType::eCpu; break;
            }
            
            if (device_info_out.features.device_type == DeviceType::eDiscrete) {
                const auto memory_properties = device.getMemoryProperties();
                for (const auto mem_type : memory_properties.memoryTypes)
                    if (mem_type.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible)
                        device_info_out.features.has_rebar |= (memory_properties.memoryHeaps[mem_type.heapIndex].size > 1 << 30); // 1GB
            }

            device_info_out.device_uuid = vk11.deviceUUID;
            device_info_out.driver_uuid = vk11.driverUUID;
            device_info_out.properties.name = properties.properties.deviceName.data();

            switch (properties.properties.vendorID) {
                case 0x10DE: device_info_out.properties.vendor = GpuVendor::eNvidia; break;
                case 0x1002: device_info_out.properties.vendor = GpuVendor::eAmd; break;
                case 0x8086: device_info_out.properties.vendor = GpuVendor::eIntel; break;
                case 0x5143: device_info_out.properties.vendor = GpuVendor::eQualcomm; break;
                default: device_info_out.properties.vendor = GpuVendor::eUnknown; break;
            }

            device_info_out.properties.subgroup_size = vk11.subgroupSize;
            device_info_out.properties.min_subgroup_size = vk13.minSubgroupSize;
            device_info_out.properties.max_subgroup_size = vk13.maxSubgroupSize;
        }

        {
            const auto supported_extensions = device.enumerateDeviceExtensionProperties();

            {
                RequiredExtensions required_ext({
                    VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME,
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                    VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
                    VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME,
                    VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
                    VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME,
                    VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
                    VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME,
                    VK_EXT_MESH_SHADER_EXTENSION_NAME,
                    VK_KHR_RAY_QUERY_EXTENSION_NAME,
                    VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME,
                    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                });
        
                if (std::vector<std::string> unsupported_exts;
                    !required_ext.CheckExtensions(supported_extensions, unsupported_exts)) {
                        is_suitable = false;
                        // Warn("Device is unsupported, unsupported extensions: {}", unsupported_exts);
                    }
    
                if(feature_chain)
                    supported_ext.insert_range(supported_ext.end(), required_ext.GetExtensions());
            }

            {
                OptionalExtensions optional_ext({
                    VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME,
                    VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME,
                    VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME,
                    VK_KHR_MAINTENANCE_9_EXTENSION_NAME,
                    VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME,
                    VK_EXT_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME,
                    VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME,
                    VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME,
                    VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME
                });

                std::unordered_set<std::string> supported_optional_ext;
                optional_ext.CheckExtensions(supported_extensions, supported_optional_ext);

                if (feature_chain) supported_ext.insert_range(supported_ext.end(), optional_ext.GetExtensions());

                // device_info_out.features.descirptor_heap = supported_optional_ext.contains(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME) 
                //                                 && supported_optional_ext.contains(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME);
                // device_info_out.features.shader_stencil_export = supported_optional_ext.contains(VK_EXT_SHADER_STENCIL_EXPORT_EXTENSION_NAME);
                // device_info_out.features.maintenance9 = supported_optional_ext.contains(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
                // device_info_out.features.pageable_device_local_memory = supported_optional_ext.contains(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME)
                //                                 && supported_optional_ext.contains(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
                // device_info_out.features.unified_image_layouts = supported_optional_ext.contains(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME);
                // device_info_out.features.ray_tracing_invocation_reorder = supported_optional_ext.contains(VK_EXT_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME);
            }
        }

        {
            auto features = device.getFeatures2<
                vk::PhysicalDeviceFeatures2,
                vk::PhysicalDeviceVulkan11Features,
                vk::PhysicalDeviceVulkan12Features,
                vk::PhysicalDeviceVulkan13Features,
                vk::PhysicalDeviceVulkan14Features,
                vk::PhysicalDeviceConditionalRenderingFeaturesEXT,
                vk::PhysicalDeviceDescriptorBufferFeaturesEXT,
                vk::PhysicalDeviceShaderUntypedPointersFeaturesKHR,
                vk::PhysicalDeviceDescriptorHeapFeaturesEXT,
                vk::PhysicalDeviceUnifiedImageLayoutsFeaturesKHR,
                vk::PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT,
                vk::PhysicalDeviceMaintenance9FeaturesKHR,
                vk::PhysicalDeviceMeshShaderFeaturesEXT,
                vk::PhysicalDeviceRayQueryFeaturesKHR,
                vk::PhysicalDeviceFragmentShadingRateFeaturesKHR,
                vk::PhysicalDeviceRayTracingInvocationReorderFeaturesEXT,
                vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR,
                vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
                vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
                vk::PhysicalDeviceComputeShaderDerivativesFeaturesKHR,
                vk::PhysicalDeviceOpacityMicromapFeaturesEXT,
                vk::PhysicalDeviceMemoryPriorityFeaturesEXT,
                vk::PhysicalDeviceRobustness2FeaturesEXT
                >();

            const auto find_in_pnext = [] <typename T> (void *p_next) -> T * {
                auto *chain = reinterpret_cast<vk::BaseOutStructure *>(p_next);
                while (chain->pNext != nullptr) {
                    if (chain->sType == T::StructureType) return reinterpret_cast<T *>(chain);
                    chain = chain->pNext;
                }
                return nullptr;
            };

            const auto features_is_supported = [&]<typename S>(std::initializer_list<vk::Bool32 S:: *> members) -> b8 {
                return Utility::FeaturesIsSupported(features, feature_chain, members);
            };

            // device_features.maintenance9
            //     = device_features.maintenance9
            //         && features_is_supported(
            //             { &vk::PhysicalDeviceMaintenance9FeaturesKHR::maintenance9 });

            // device_features.opacity_micromap
            //     = device_features.opacity_micromap
            //         && features_is_supported(
            //             { &vk::PhysicalDeviceOpacityMicromapFeaturesEXT::micromap });

            // device_features.pageable_device_local_memory
            //     = device_features.pageable_device_local_memory
            //             && features_is_supported(
            //                 { &vk::PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT::pageableDeviceLocalMemory })
            //             && features_is_supported(
            //                 { &vk::PhysicalDeviceMemoryPriorityFeaturesEXT::memoryPriority });

            // device_features.unified_image_layouts
            //     = device_features.unified_image_layouts
            //         && features_is_supported(
            //             { &vk::PhysicalDeviceUnifiedImageLayoutsFeaturesKHR::unifiedImageLayouts });

            // device_features.descirptor_heap
            //     = device_features.descirptor_heap
            //         && features_is_supported(
            //             { &vk::PhysicalDeviceDescriptorHeapFeaturesEXT::descriptorHeap })
            //         && features_is_supported(
            //             { &vk::PhysicalDeviceShaderUntypedPointersFeaturesKHR::shaderUntypedPointers });

            // device_features.ray_tracing_invocation_reorder
            //     = device_features.ray_tracing_invocation_reorder
            //         && features_is_supported(
            //             { &vk::PhysicalDeviceRayTracingInvocationReorderFeaturesEXT::rayTracingInvocationReorder });

            // if (!features_is_supported(
            //     {
            //         &vk::PhysicalDeviceRobustness2FeaturesEXT::nullDescriptor,
            //     })) {
            //     is_suitable = false;
            //     // Error("GPU is unsupported, must be support VK_EXT_robustness2 extension");
            // }

            // if (!features_is_supported(
            //     {
            //         &vk::PhysicalDeviceComputeShaderDerivativesFeaturesKHR::computeDerivativeGroupLinear,
            //         &vk::PhysicalDeviceComputeShaderDerivativesFeaturesKHR::computeDerivativeGroupQuads,
            //     })) {
            //     is_suitable = false;
            //     // Error("GPU is unsupported, must be support VK_EXT_compute_shader_derivatives extension");
            // }

            // if (!features_is_supported(
            //     { &vk::PhysicalDeviceDescriptorBufferFeaturesEXT::descriptorBuffer })) {
            //     is_suitable = false;
            //     // Error("GPU is unsupported, must be support VK_EXT_descriptor_buffer extension");
            // }

            // if (!features_is_supported(
            //     {
            //         &vk::PhysicalDeviceFragmentShadingRateFeaturesKHR::attachmentFragmentShadingRate,
            //         &vk::PhysicalDeviceFragmentShadingRateFeaturesKHR::pipelineFragmentShadingRate,
            //         &vk::PhysicalDeviceFragmentShadingRateFeaturesKHR::primitiveFragmentShadingRate,
            //     })) {
            //     is_suitable = false;
            //     // Error("GPU is unsupported, must be support VK_KHR_fragment_shading_rate extension");
            // }

            if (!features_is_supported(
                {
                    &vk::PhysicalDeviceMeshShaderFeaturesEXT::meshShader,
                    &vk::PhysicalDeviceMeshShaderFeaturesEXT::taskShader,
                    &vk::PhysicalDeviceMeshShaderFeaturesEXT::primitiveFragmentShadingRateMeshShader,
                })) {
                is_suitable = false;
                // Error("GPU is unsupported, must be support VK_EXT_mesh_shader extension");
            }

            if (!features_is_supported(
                { &vk::PhysicalDeviceRayQueryFeaturesKHR::rayQuery })) {
                is_suitable = false;
                // Error("GPU is unsupported, must be support VK_KHR_ray_query extension");
            }

            if (!features_is_supported(
                {
                    &vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR::rayTracingMaintenance1,
                    &vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR::rayTracingPipelineTraceRaysIndirect2,
                })) {
                is_suitable = false;
                // Error("GPU is unsupported, must be support VK_KHR_ray_tracing_maintenance1 extension");
            }

            if (!features_is_supported(
                {
                    &vk::PhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipeline,
                    &vk::PhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipelineTraceRaysIndirect,
                })) {
                is_suitable = false;
                // Error("GPU is unsupported, must be support VK_KHR_ray_tracing_pipeline extension");
            }

            if (!features_is_supported(
                {
                    &vk::PhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructure,
                    &vk::PhysicalDeviceAccelerationStructureFeaturesKHR::descriptorBindingAccelerationStructureUpdateAfterBind,
                })) {
                is_suitable = false;
                // Error("GPU is unsupported, must be support VK_KHR_acceleration_structure extension");
            }

            if (!features_is_supported(
                { &vk::PhysicalDeviceConditionalRenderingFeaturesEXT::conditionalRendering })) {
                is_suitable = false;
                // Error("GPU is unsupported, must be support VK_EXT_conditional_rendering extension");
            }

            if (!features_is_supported(
                { &vk::PhysicalDeviceVulkan14Features::maintenance5 })) {
                is_suitable = false;
                // Error("GPU is unsupported, must be support required Vulkan 1.4 features");
            }

            if (!features_is_supported(
                {
                    &vk::PhysicalDeviceVulkan13Features::computeFullSubgroups,
                    &vk::PhysicalDeviceVulkan13Features::dynamicRendering,
                    &vk::PhysicalDeviceVulkan13Features::inlineUniformBlock,
                    &vk::PhysicalDeviceVulkan13Features::maintenance4,
                    &vk::PhysicalDeviceVulkan13Features::pipelineCreationCacheControl,
                    &vk::PhysicalDeviceVulkan13Features::privateData,
                    &vk::PhysicalDeviceVulkan13Features::robustImageAccess,
                    &vk::PhysicalDeviceVulkan13Features::shaderDemoteToHelperInvocation,
                    &vk::PhysicalDeviceVulkan13Features::shaderIntegerDotProduct,
                    &vk::PhysicalDeviceVulkan13Features::shaderTerminateInvocation,
                    &vk::PhysicalDeviceVulkan13Features::shaderZeroInitializeWorkgroupMemory,
                    &vk::PhysicalDeviceVulkan13Features::subgroupSizeControl,
                    &vk::PhysicalDeviceVulkan13Features::synchronization2,
                })) {
                is_suitable = false;
                // Error("GPU is unsupported, must be support required Vulkan 1.3 features");
            }

            if (!features_is_supported(
                {
                    &vk::PhysicalDeviceVulkan12Features::bufferDeviceAddress,
                    &vk::PhysicalDeviceVulkan12Features::descriptorBindingPartiallyBound,
                    &vk::PhysicalDeviceVulkan12Features::descriptorBindingSampledImageUpdateAfterBind,
                    &vk::PhysicalDeviceVulkan12Features::descriptorBindingUniformBufferUpdateAfterBind,
                    &vk::PhysicalDeviceVulkan12Features::descriptorBindingStorageImageUpdateAfterBind,
                    &vk::PhysicalDeviceVulkan12Features::descriptorBindingUpdateUnusedWhilePending,
                    &vk::PhysicalDeviceVulkan12Features::descriptorBindingVariableDescriptorCount,
                    &vk::PhysicalDeviceVulkan12Features::descriptorIndexing,
                    &vk::PhysicalDeviceVulkan12Features::drawIndirectCount,
                    &vk::PhysicalDeviceVulkan12Features::hostQueryReset,
                    &vk::PhysicalDeviceVulkan12Features::imagelessFramebuffer,
                    &vk::PhysicalDeviceVulkan12Features::runtimeDescriptorArray,
                    &vk::PhysicalDeviceVulkan12Features::scalarBlockLayout,
                    &vk::PhysicalDeviceVulkan12Features::samplerFilterMinmax,
                    &vk::PhysicalDeviceVulkan12Features::samplerMirrorClampToEdge,
                    &vk::PhysicalDeviceVulkan12Features::separateDepthStencilLayouts,
                    &vk::PhysicalDeviceVulkan12Features::shaderInt8,
                    &vk::PhysicalDeviceVulkan12Features::shaderFloat16,
                    &vk::PhysicalDeviceVulkan12Features::shaderOutputLayer,
                    &vk::PhysicalDeviceVulkan12Features::shaderOutputViewportIndex,
                    &vk::PhysicalDeviceVulkan12Features::shaderSampledImageArrayNonUniformIndexing,
                    &vk::PhysicalDeviceVulkan12Features::shaderStorageBufferArrayNonUniformIndexing,
                    &vk::PhysicalDeviceVulkan12Features::shaderStorageImageArrayNonUniformIndexing,
                    &vk::PhysicalDeviceVulkan12Features::shaderUniformBufferArrayNonUniformIndexing,
                    &vk::PhysicalDeviceVulkan12Features::shaderSubgroupExtendedTypes,
                    &vk::PhysicalDeviceVulkan12Features::storageBuffer8BitAccess,
                    &vk::PhysicalDeviceVulkan12Features::subgroupBroadcastDynamicId,
                    &vk::PhysicalDeviceVulkan12Features::timelineSemaphore,
                    &vk::PhysicalDeviceVulkan12Features::uniformBufferStandardLayout,
                    &vk::PhysicalDeviceVulkan12Features::vulkanMemoryModel,
                    &vk::PhysicalDeviceVulkan12Features::vulkanMemoryModelDeviceScope,
                })) {
                is_suitable = false;
                // Error("GPU is unsupported, must be support required Vulkan 1.2 features");
            }

            if (!features_is_supported(
                {
                    &vk::PhysicalDeviceVulkan11Features::multiview,
                    &vk::PhysicalDeviceVulkan11Features::shaderDrawParameters,
                    &vk::PhysicalDeviceVulkan11Features::variablePointers,
                    &vk::PhysicalDeviceVulkan11Features::variablePointersStorageBuffer,
                    &vk::PhysicalDeviceVulkan11Features::storageBuffer16BitAccess,
                })) {
                is_suitable = false;
                // Error("GPU is unsupported, must be support required Vulkan 1.1 features");
            }

            if (!features_is_supported(
                {
                    &vk::PhysicalDeviceFeatures::vertexPipelineStoresAndAtomics,
                    &vk::PhysicalDeviceFeatures::fragmentStoresAndAtomics,
                    &vk::PhysicalDeviceFeatures::multiDrawIndirect,
                    &vk::PhysicalDeviceFeatures::imageCubeArray,
                    &vk::PhysicalDeviceFeatures::depthClamp,
                    &vk::PhysicalDeviceFeatures::depthBiasClamp,
                    &vk::PhysicalDeviceFeatures::shaderCullDistance,
                    &vk::PhysicalDeviceFeatures::shaderClipDistance,
                    &vk::PhysicalDeviceFeatures::independentBlend,
                    &vk::PhysicalDeviceFeatures::inheritedQueries,
                    &vk::PhysicalDeviceFeatures::samplerAnisotropy,
                    &vk::PhysicalDeviceFeatures::sampleRateShading,
                    &vk::PhysicalDeviceFeatures::textureCompressionBC,
                    &vk::PhysicalDeviceFeatures::shaderInt16,
                    &vk::PhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing,
                    &vk::PhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing,
                    &vk::PhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing,
                    &vk::PhysicalDeviceFeatures::drawIndirectFirstInstance,
                    &vk::PhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat,
                    &vk::PhysicalDeviceFeatures::multiViewport,
                    &vk::PhysicalDeviceFeatures::pipelineStatisticsQuery,
                    &vk::PhysicalDeviceFeatures::fullDrawIndexUint32,
                    &vk::PhysicalDeviceFeatures::shaderInt64,
                })) {
                is_suitable = false;
                // Error("GPU is unsupported, must be support required Vulkan 1.0 features");
            }

            Utility::FindInPNext<vk::PhysicalDeviceVulkan11Features>(feature_chain)
                ->setStorageInputOutput16(features.get<vk::PhysicalDeviceVulkan11Features>().storageInputOutput16);
        }

        // TODO: check properties

        {
            void *p_next = nullptr;

            // vk::PhysicalDeviceDescriptorHeapPropertiesEXT descriptor_heap_properties{};
            // if (device_features.descirptor_heap)
            // ADD_P_NEXT(descriptor_heap_properties);
        
            vk::PhysicalDeviceComputeShaderDerivativesPropertiesKHR compute_shader_derivatives_properties{};
            ADD_P_NEXT(compute_shader_derivatives_properties);

            vk::PhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties{};
            ADD_P_NEXT(descriptor_buffer_properties);

            vk::PhysicalDeviceVulkan12Properties properties12{};
            ADD_P_NEXT(properties12);

            vk::PhysicalDeviceVulkan11Properties properties11{};
            ADD_P_NEXT(properties11);

            vk::PhysicalDeviceProperties2 properties_chain{};
            ADD_P_NEXT(properties_chain);

            auto &properties10 = properties_chain.properties;
            device.getProperties2(&properties_chain);

            // device_features.mesh_and_task_shader_derivatives = compute_shader_derivatives_properties.meshAndTaskShaderDerivatives;

            PROPERTY_MUST_BE_GTE(properties10.limits.maxColorAttachments, 8);
            PROPERTY_MUST_BE_GTE(properties10.limits.maxPerStageDescriptorUniformBuffers, 15);

            if (!(properties11.subgroupSupportedOperations & 
                (
                vk::SubgroupFeatureFlagBits::eBasic
                | vk::SubgroupFeatureFlagBits::eVote
                | vk::SubgroupFeatureFlagBits::eArithmetic
                | vk::SubgroupFeatureFlagBits::eBallot
                | vk::SubgroupFeatureFlagBits::eShuffle
                | vk::SubgroupFeatureFlagBits::eShuffleRelative
                | vk::SubgroupFeatureFlagBits::eClustered
                | vk::SubgroupFeatureFlagBits::eQuad
                ))) {
                    is_suitable = false;
                    // Warn("GPU is unsupported, must be support {}, your gpu {}", vk::to_string(vk::SubgroupFeatureFlagBits::eBasic
                    //     | vk::SubgroupFeatureFlagBits::eVote
                    //     | vk::SubgroupFeatureFlagBits::eArithmetic
                    //     | vk::SubgroupFeatureFlagBits::eBallot
                    //     | vk::SubgroupFeatureFlagBits::eShuffle
                    //     | vk::SubgroupFeatureFlagBits::eShuffleRelative
                    //     | vk::SubgroupFeatureFlagBits::eClustered
                    //     | vk::SubgroupFeatureFlagBits::eQuad), vk::to_string(properties11.subgroupSupportedOperations));
                }

            if (!(properties11.subgroupSupportedStages & (vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eFragment))) {
                is_suitable = false;
                // Warn("GPU is unsupported, must be support {}, your gpu {}", 
                //     vk::to_string(vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eFragment), 
                //     vk::to_string(properties11.subgroupSupportedStages));
            }

            PROPERTY_MUST_BE_GTE(properties12.maxPerStageDescriptorUpdateAfterBindStorageImages, 20'000);
            PROPERTY_MUST_BE_GTE(properties12.maxPerStageDescriptorUpdateAfterBindSampledImages, 200'000);
            PROPERTY_MUST_BE_GTE(properties12.maxPerStageDescriptorUpdateAfterBindSamplers, 2'000);

            // m_vk_device_properties.ubo_image_descriptor_size = descriptor_buffer_properties.uniformBufferDescriptorSize;
            // m_vk_device_properties.sampled_image_descriptor_size = descriptor_buffer_properties.sampledImageDescriptorSize;
            // m_vk_device_properties.storage_image_descriptor_size = descriptor_buffer_properties.storageImageDescriptorSize;
            // m_vk_device_properties.sampler_descriptor_size = descriptor_buffer_properties.samplerDescriptorSize;

            // const auto align = [] (u32 val, u32 align) {
            //     return (val + align - 1) & ~(align - 1);
            // };

            // if (m_vk_device_features.descirptor_heap) {
            //     m_vk_device_properties.resource_descriptor_size_for_heap
            //         = align(std::max(descriptor_heap_properties.bufferDescriptorSize, descriptor_heap_properties.imageDescriptorSize), std::max(descriptor_heap_properties.bufferDescriptorAlignment, descriptor_heap_properties.imageDescriptorAlignment));
            //     m_vk_device_properties.sampler_descriptor_size_for_heap = descriptor_heap_properties.samplerDescriptorSize;
            //     m_vk_device_properties.resource_heap_min_reserve = descriptor_heap_properties.minResourceHeapReservedRange;
            //     m_vk_device_properties.sampler_heap_min_reserve = descriptor_heap_properties.minSamplerHeapReservedRange;
            //     m_vk_device_properties.resource_heap_alignment = descriptor_heap_properties.resourceHeapAlignment;
            //     m_vk_device_properties.sampler_heap_alignment = descriptor_heap_properties.samplerHeapAlignment;
            // }
        }

        {
            // if (device_features.pageable_device_local_memory) Info("GPU supports pageable_device_local_memory");
            // if (device_features.unified_image_layouts) Info("GPU supports unified_image_layouts");
            // if (device_features.descirptor_heap) Info("GPU supports descirptor_heap");
            // if (device_features.shader_stencil_export) Info("GPU supports shader_stencil_export");
            // if (device_features.maintenance9) Info("GPU supports maintenance9");
            // if (device_features.input_output_16) Info("GPU supports input_output_16");
            // if (device_features.swapchain_maintenance1) Info("GPU supports swapchain_maintenance1");
            // if (device_features.mesh_and_task_shader_derivatives) Info("GPU supports mesh_and_task_shader_derivatives");
            // if (device_features.ray_tracing_invocation_reorder) Info("GPU supports ray_tracing_invocation_reorder");
            // if (device_features.opacity_micromap) Info("GPU supports opacity_micromap");
        }

        {
            vk::PhysicalDeviceVulkan12Properties vk12; 
            vk::PhysicalDeviceProperties2 properties;
            properties.pNext = &vk12;
            device.getProperties2(&properties);

//            Info("{}, is {}", properties.properties.deviceName.data(), is_suitable ? "supported" : "not supported");
        }

        if (is_suitable)
            supported_ext_out = supported_ext;

        return is_suitable;
    }

    void Device::CreateInstance() noexcept {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));

        std::vector<const char *> layers;
        std::vector<const char *> extensions;

        GetInstanceFeatures(extensions, layers, m_vk_instance_features);

        const auto app_info
            = vk::ApplicationInfo{}
                .setApplicationVersion(VK_MAKE_VERSION(0, 1, 0))
                .setEngineVersion(VK_MAKE_VERSION(0, 1, 0))
                .setApiVersion(VK_API_VERSION_1_4)
                .setPEngineName("DnmEngine")
                .setPApplicationName("some_game")
                ;

        auto enabled_features = std::vector{
            // TODO: my gpu crached when best practices enabled(, and validation layer)
            vk::ValidationFeatureEnableEXT::eBestPractices,
            vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
            vk::ValidationFeatureEnableEXT::eDebugPrintf,
        };

        if (m_settings.gpu_assisted_validaiton) {
            enabled_features.push_back(vk::ValidationFeatureEnableEXT::eGpuAssisted);
            enabled_features.push_back(vk::ValidationFeatureEnableEXT::eGpuAssistedReserveBindingSlot);
        }

        const auto validataion_features 
            = vk::ValidationFeaturesEXT{}.setEnabledValidationFeatures(enabled_features);

        const auto debug_create_info
            = vk::DebugUtilsMessengerCreateInfoEXT{}
                .setMessageSeverity(
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                // | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                                // | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                            )
                .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                                            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                                            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                                        )
                .setPfnUserCallback((vk::PFN_DebugUtilsMessengerCallbackEXT)DebugCallback)
                .setPNext(m_settings.validation_layer ? &validataion_features : nullptr)
                ;

        const auto instance_create_info 
            = vk::InstanceCreateInfo{}
                .setPApplicationInfo(&app_info)
                .setPEnabledLayerNames(layers)
                .setPEnabledExtensionNames(extensions)
                .setPNext(m_settings.validation_layer ? &debug_create_info : nullptr);
                ;

        try {
            m_instance = vk::createInstance(instance_create_info);
        } 
        catch (const std::exception &e) {
            // Critical("Failed Vulkan Instance Creation: {}", e.what());
            return;
        }

        // Info("Vulkan Instance Created");
        // Info("Enabled Vulkan Instance Extensions: {}", extensions);
        // Info("Enabled Vulkan Instance Layers: {}", layers);

        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);
        // Info("Vulkan Instance Functions Loaded");

        if (GetSettings().validation_layer) m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(debug_create_info);
        // if (GetSettings().validation_layer) Info("Vulkan Debug Messenger Created");
    }

    void Device::CreateDevice() noexcept {
        std::vector<const char *> extensions;

        const auto delete_feature_chain = [](void *&feature_chain) {
            if (!feature_chain) return;

            auto *chain = reinterpret_cast<vk::BaseOutStructure *>(feature_chain);
            std::vector<void *> ptrs;
            while (chain->pNext != nullptr) {
                ptrs.emplace_back(chain->pNext);
                chain = chain->pNext;
            }

            for (auto *ptr : ptrs | std::views::reverse)
                std::free(ptr);

            feature_chain = nullptr;
        };

        void *feature_chain{};

        {
            const auto physical_devices = m_instance.enumeratePhysicalDevices();
            // m_device_features.is_apu = true;

            for (const auto physical_device : physical_devices) {
                void *current_feature_chain{};
                const auto is_supported = CheckAndGetDeviceFeatures(physical_device, extensions, m_vk_device_features, current_feature_chain);

                if (!is_supported) {
                    delete_feature_chain(current_feature_chain);
                    extensions.clear();
                    continue;
                }

                m_physical_device = physical_device;
                
                if (m_physical_device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                    // m_device_features.is_apu = false;
                    delete_feature_chain(feature_chain);
                    feature_chain = current_feature_chain;
                    break;
                }
                delete_feature_chain(feature_chain);
                feature_chain = current_feature_chain;
            }
        }

        // if (m_physical_device == VK_NULL_HANDLE) Critical("No supported any gpu");

        // Info("Selected Vulkan Physical Device: {}", m_physical_device.getProperties().deviceName.data());
        // Info("Enabled Vulkan Device Extensions: {}", extensions);

        void *p_next{};

        {
            const vk::SampleCountFlags supported_samples = m_physical_device.getProperties().limits.framebufferColorSampleCounts;

            m_vk_device_properties.sample_map[int(SampleCount::e1)] = vk::SampleCountFlagBits::e1;
            m_vk_device_properties.sample_map[int(SampleCount::e2)] = vk::SampleCountFlagBits::e1;
            m_vk_device_properties.sample_map[int(SampleCount::e4)] = vk::SampleCountFlagBits::e1;
            m_vk_device_properties.sample_map[int(SampleCount::e8)] = vk::SampleCountFlagBits::e1;

            if (vk::SampleCountFlagBits::e2 & supported_samples)
                m_vk_device_properties.sample_map[int(SampleCount::e2)] = vk::SampleCountFlagBits::e2;
            else if (vk::SampleCountFlagBits::e4 & supported_samples)
                m_vk_device_properties.sample_map[int(SampleCount::e2)] = vk::SampleCountFlagBits::e4;

            if (vk::SampleCountFlagBits::e4 & supported_samples)
                m_vk_device_properties.sample_map[int(SampleCount::e4)] = vk::SampleCountFlagBits::e4;
            else if (vk::SampleCountFlagBits::e8 & supported_samples)
                m_vk_device_properties.sample_map[int(SampleCount::e4)] = vk::SampleCountFlagBits::e8;
            else if (vk::SampleCountFlagBits::e2 & supported_samples)
                m_vk_device_properties.sample_map[int(SampleCount::e4)] = vk::SampleCountFlagBits::e2;

            if (vk::SampleCountFlagBits::e8 & supported_samples)
                m_vk_device_properties.sample_map[int(SampleCount::e8)] = vk::SampleCountFlagBits::e8;
            else if (vk::SampleCountFlagBits::e4 & supported_samples)
                m_vk_device_properties.sample_map[int(SampleCount::e8)] = vk::SampleCountFlagBits::e4;
            else if (vk::SampleCountFlagBits::e2 & supported_samples)
                m_vk_device_properties.sample_map[int(SampleCount::e8)] = vk::SampleCountFlagBits::e2;

            m_vk_device_properties.pipeline_cache_uuid = m_physical_device.getProperties().pipelineCacheUUID;

            m_vk_device_features.descirptor_heap = m_vk_device_features.descirptor_heap && !GetVulkanSettings().no_descriptor_heap;
            m_vk_device_features.shader_stencil_export = m_vk_device_features.shader_stencil_export && !GetVulkanSettings().no_stencil_export;
            m_vk_device_features.maintenance9 = m_vk_device_features.maintenance9 && !GetVulkanSettings().no_maintenance9;
            m_vk_device_features.pageable_device_local_memory = m_vk_device_features.pageable_device_local_memory && !GetVulkanSettings().no_pageable_device_local_memory;
            m_vk_device_features.unified_image_layouts = m_vk_device_features.unified_image_layouts && !GetVulkanSettings().no_unified_image_layouts;
            m_device_features.shader_stencil_export = m_vk_device_features.shader_stencil_export;
            m_device_features.has_rebar = m_vk_device_features.has_rebar;
            // m_device_features.is_apu = m_vk_device_features.is_apu
            m_device_features.mesh_and_task_shader_derivatives = m_vk_device_features.mesh_and_task_shader_derivatives;
            m_device_features.opacity_micromap = m_vk_device_features.opacity_micromap;
            m_device_features.ray_tracing_invocation_reorder = m_vk_device_features.ray_tracing_invocation_reorder;
        }

        const float queue_priorities = 1.f; 

        auto general_queue_create_info
            = vk::DeviceQueueCreateInfo{}
                .setPQueuePriorities(&queue_priorities)
                .setQueueCount(1)
                .setQueueFamilyIndex(0) // vk::DeviceQueueCreateFlagBits::eInternallySynchronizedKHR
                ;

        auto compute_queue_create_info
                = vk::DeviceQueueCreateInfo{}
                .setPQueuePriorities(&queue_priorities)
                .setQueueCount(1)
                .setQueueFamilyIndex(0) // vk::DeviceQueueCreateFlagBits::eInternallySynchronizedKHR
                ;

        auto copy_queue_create_info
                = vk::DeviceQueueCreateInfo{}
                .setPQueuePriorities(&queue_priorities)
                .setQueueCount(1)
                .setQueueFamilyIndex(0) // vk::DeviceQueueCreateFlagBits::eInternallySynchronizedKHR
                ;

        {
            m_general_queue_family_index = -1;
            m_compute_queue_family_index = -1;
            m_compute_queue_family_index = -1;

            u32 general_queue_count{};
            u32 compute_queue_count{};

            u8 family_index{};
            for (const auto queue_family : m_physical_device.getQueueFamilyProperties2()) {
                const auto count = queue_family.queueFamilyProperties.queueCount;
                const auto flags = queue_family.queueFamilyProperties.queueFlags;

                if (flags & vk::QueueFlagBits::eGraphics) {
                    m_general_queue_family_index = family_index++;
                    general_queue_count = count;
                }
                else if (flags & vk::QueueFlagBits::eCompute) {
                    m_compute_queue_family_index = family_index++;
                    compute_queue_count = count;
                }
                else if (flags & vk::QueueFlagBits::eTransfer) {
                    m_copy_queue_family_index = family_index++;
                }
                else {
                    ++family_index;
                }
            }

            general_queue_create_info
                .setQueueFamilyIndex(m_general_queue_family_index)
                .setQueueCount(1)
                ;

            compute_queue_create_info
                .setQueueFamilyIndex(m_compute_queue_family_index)
                .setQueueCount(1)
                ;

            copy_queue_create_info
                .setQueueFamilyIndex(m_copy_queue_family_index)
                .setQueueCount(1)
                ;

            if (m_compute_queue_family_index == u8(-1))
                compute_queue_create_info.setQueueCount(0);

            if (m_copy_queue_family_index == u8(-1))
                copy_queue_create_info.setQueueCount(0);
        }

        std::vector<vk::DeviceQueueCreateInfo> queues;
        queues.push_back(general_queue_create_info);
        if (compute_queue_create_info.queueCount) queues.push_back(compute_queue_create_info);
        if (copy_queue_create_info.queueCount) queues.push_back(copy_queue_create_info);

        const auto device_create_info
            = vk::DeviceCreateInfo{}
                .setQueueCreateInfos(queues)
                .setPEnabledExtensionNames(extensions)
                .setPNext(feature_chain)
                ;

        try {
            m_device = m_physical_device.createDevice(device_create_info);
        } catch (const std::exception &e) {
            // Critical("Failed Vulkan Device Creation: {}", e.what());
            return;
        }
        // Info("Vulkan Device Created");

        m_general_queue = m_device.getQueue(m_general_queue_family_index, 0);
        m_compute_queue = compute_queue_create_info.queueCount ? m_device.getQueue(m_compute_queue_family_index, 0) : m_general_queue;
        m_copy_queue = copy_queue_create_info.queueCount ? m_device.getQueue(m_copy_queue_family_index, 0) : m_compute_queue;

        SET_DEBUG_NAME_VK_OBJECT(GetDevice(), m_general_queue, "general queue");
        SET_DEBUG_NAME_VK_OBJECT(GetDevice(), m_compute_queue, "compute queue");
        SET_DEBUG_NAME_VK_OBJECT(GetDevice(), m_copy_queue, "copy queue");

        // Info("Vulkan General Queue Family Index: {}", m_general_queue_family_index);
        // Info("Vulkan Compute Queue Family Index: {}", m_compute_queue_family_index);
        // Info("Vulkan Copy Queue Family Index: {}", m_copy_queue_family_index);

        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device);
        // Info("Vulkan Device Func Loaded");

        delete_feature_chain(feature_chain);
    }

    void Device::CreateVmaAllocator() noexcept {
        VmaVulkanFunctions vulkan_functions{};
        vulkan_functions.vkGetInstanceProcAddr = vk::detail::defaultDispatchLoaderDynamic.vkGetInstanceProcAddr;
        vulkan_functions.vkGetDeviceProcAddr = vk::detail::defaultDispatchLoaderDynamic.vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo create_info{};
        create_info.pVulkanFunctions = &vulkan_functions;
        create_info.instance = m_instance;
        create_info.device = m_device;
        create_info.physicalDevice = m_physical_device;
        create_info.vulkanApiVersion = VK_API_VERSION_1_3;
        create_info.flags 
            = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT 
            | VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT 
            | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT
            | VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT
            | VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT
            ;

        vmaCreateAllocator(&create_info, &m_vma_allocator);
    }

    void Device::CreateDescriptorManager() noexcept {
        m_descriptor_manager = 
            m_vk_device_features.descirptor_heap ? 
            static_cast<DescriptorManager *>(new DescriptorManagerHeap(*this)) 
            : static_cast<DescriptorManager *>(new DescriptorManagerBuffer(*this));
    }
    
    DnmRHI::Buffer *Device::ICreateBuffer(const BufferDesc &desc) noexcept {
        return new Buffer(*this, desc);
    }
    DnmRHI::Image *Device::ICreateImage(const ImageDesc &desc) noexcept {
        return new Image(*this, desc);
    }
    DnmRHI::CommandBuffer *Device::ICreateCommandBuffer(const CommandBufferDesc &desc) noexcept {
        return new CommandBuffer(*this, desc);
    }
    DnmRHI::GraphicsPipeline *Device::ICreateGraphicsPipeline(const GraphicsPipelineDesc &desc) noexcept {
        return new GraphicsPipeline(*this, desc);
    }
    DnmRHI::ComputePipeline *Device::ICreateComputePipeline(const ComputePipelineDesc &desc) noexcept {
        return new ComputePipeline(*this, desc);
    }
    DnmRHI::Semaphore *Device::ICreateSemaphore(const SemaphoreDesc &desc) noexcept {
        return new Semaphore(*this, desc);
    }
    DnmRHI::Swapchain *Device::ICreateSwapchain(const SwapchainDesc &desc) noexcept {
        return new Swapchain(*this, desc);
    }
    DnmRHI::Shader *Device::ICreateShader(const ShaderEntryPoint &entry_point) noexcept {
        return new Shader(*this, entry_point);
    }
    
    u32 Device::IGetBindlessStorage(DnmRHI::Image &image, ImageSubresource subresource) noexcept {
        return m_descriptor_manager->GetStorageImageBindlessIndex(reinterpret_cast<Vk::Image &>(image), subresource);
    }
    u32 Device::IGetBindlessSampled(DnmRHI::Image &image, ImageSubresource subresource) noexcept {
        return m_descriptor_manager->GetSampledImageBindlessIndex(reinterpret_cast<Vk::Image &>(image), subresource);
    }
    u32 Device::IGetBindlessSampler(const SamplerDesc &desc) noexcept {
        return m_descriptor_manager->GetSamplerBindlessIndex(desc);
    }

    void Device::ISubmitCommandBuffer(const CommandBufferSubmitDesc &desc) noexcept {
        std::vector<vk::CommandBufferSubmitInfo> vk_command_buffers;
        std::vector<vk::SemaphoreSubmitInfo> vk_wait_semaphores;
        std::vector<vk::SemaphoreSubmitInfo> vk_signal_semaphores;
        vk_command_buffers.reserve(desc.command_buffers.size());
        vk_wait_semaphores.reserve(desc.wait_semaphores.size());
        vk_signal_semaphores.reserve(desc.signal_semaphores.size() + 1);

        const auto queue_type = desc.command_buffers[0]->GetDesc().queue_type;

        for (const auto &desc : desc.signal_semaphores)
            vk_signal_semaphores.emplace_back(
                static_cast<const Vk::Semaphore *>(desc.semaphore)->GetSemaphore(),
                desc.value, 
                vk::PipelineStageFlagBits2::eAllCommands
            );

        for (const auto &desc : desc.wait_semaphores)
            vk_wait_semaphores.emplace_back(
                static_cast<const Vk::Semaphore *>(desc.semaphore)->GetSemaphore(),
                desc.value, 
                vk::PipelineStageFlagBits2::eAllCommands
            );

        for (const auto *command_buffer : desc.command_buffers) {
            const auto *vk_command_buffer = static_cast<const Vk::CommandBuffer *>(command_buffer);
            vk_command_buffers.emplace_back(vk_command_buffer->GetCommandBuffer());

            vk_signal_semaphores.reserve(vk_command_buffer->GetUsedSwapchainImages().size());
            vk_wait_semaphores.reserve(vk_command_buffer->GetUsedSwapchainImages().size());
            for (const auto &swapchain_image_info : vk_command_buffer->GetUsedSwapchainImages()) {
                const auto &swapchain = *static_cast<const Vk::Swapchain *>(swapchain_image_info.swapchain);
                const auto vk_image_index = swapchain.GetVkImageIndex();

                const auto swapchain_available_semaphore = swapchain.GetImageAvailableSemaphore()[swapchain_image_info.image_index];
                const auto image_acquired_semaphore = swapchain.GetImageAcquireSemaphores()[swapchain.GetCurrentFrameIndex()];

                vk_signal_semaphores.emplace_back(swapchain_available_semaphore, 0, vk::PipelineStageFlagBits2::eAllCommands);
                vk_wait_semaphores.emplace_back(image_acquired_semaphore, 0, vk::PipelineStageFlagBits2::eAllCommands);
            }
        }

        const auto submit_info
            = vk::SubmitInfo2{}
                .setCommandBufferInfos(vk_command_buffers)
                .setWaitSemaphoreInfos(vk_wait_semaphores)
                .setSignalSemaphoreInfos(vk_signal_semaphores);

        switch (queue_type) {
            case QueueType::eGeneral:
                m_general_queue.submit2(submit_info); break;
            case QueueType::eCompute:
                m_compute_queue.submit2(submit_info); break;
            case QueueType::eCopy:
                m_copy_queue.submit2(submit_info); break;
            default: std::unreachable();
        }
    }

    void Device::IWaitSemaphore(const WaitSemaphoreDesc &desc) noexcept {
        std::vector<vk::Semaphore> semaphores{}; // (desc.semaphores.begin(), desc.semaphores.end());
        std::vector<u64> values{};
        semaphores.reserve(desc.semaphores.size());
        values.reserve(desc.semaphores.size());

        for (const auto &semaphore : desc.semaphores) {
            semaphores.emplace_back(static_cast<const Semaphore *>(semaphore.semaphore)->GetSemaphore());
            values.emplace_back(semaphore.value);
        }

        [[maybe_unused]] const auto _ = m_device.waitSemaphores(
            vk::SemaphoreWaitInfo{}
                .setSemaphores(semaphores)
                .setValues(values)
                .setFlags(desc.any ? vk::SemaphoreWaitFlagBits::eAny : vk::SemaphoreWaitFlagBits{}),
            desc.timeout);
    }

    void Device::IPresent(const PresentDesc &desc) noexcept {
        auto &swapchain = reinterpret_cast<Vk::Swapchain &>(*desc.swapchain);
        const auto vk_image_index = swapchain.GetVkImageIndex();
        const auto vk_swapchain = swapchain.GetSwapchain();

        const auto cmd_finished_semaphore = swapchain.GetImageAvailableSemaphore()[vk_image_index];

        const auto present_info
            = vk::PresentInfoKHR{}
                .setSwapchains(vk_swapchain)
                .setImageIndices(vk_image_index)
                .setWaitSemaphores(cmd_finished_semaphore)
                ;

        {
            [[maybe_unused]] const auto _ = m_general_queue.presentKHR(present_info);
        }
    }
}