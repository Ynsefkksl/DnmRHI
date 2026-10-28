#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Platform/Vulkan/ComputePipeline.hpp>
#include <DnmRHI/Platform/Vulkan/GraphicsPipeline.hpp>
#include <DnmRHI/Platform/Vulkan/DescriptorManager.hpp>
#include <DnmRHI/Platform/Vulkan/Semaphore.hpp>
#include <DnmRHI/Platform/Vulkan/CommandBuffer.hpp>
#include <DnmRHI/Platform/Vulkan/Swapchain.hpp>
#include <DnmRHI/Platform/Vulkan/Shader.hpp>
#include <DnmRHI/Platform/Vulkan/vulkan_profiles.h>

#include <print>
#include <set>
#include <vector>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#define ADD_P_NEXT(struct_name) do {struct_name.pNext = p_next; \
			                    p_next = &struct_name;} while(0)
#define FEATURE_MUST_BE_SUPPORTED(feature) if (b8(feature) != true) { is_suitable = false; /* Warn("GPU is unsupported, must be support " #feature); */ }
#define PROPERTY_MUST_BE_GTE(property, value) if (property < value) { is_suitable = false; /* Warn("GPU is unsupported, " #property " greater than " #value ", your value: {}", value); */ }
#define PROPERTY_MUST_BE_SUPPORTED(property) if (b8(property) != true) { is_suitable = false; /* Warn("GPU is unsupported, must be support " #property " property"); */ }

// TODO:
namespace DnmRHI {
    #define MAKE_BLOCK_CAPABILITIES(name) VpBlockProperties{c_Vp_Profile, c_Vp_Profile.specVersion, name}

    static constexpr auto c_Vp_Profile =
        VpProfileProperties{"VP_DNMRHI_VulkanRequirements", VK_API_VERSION_1_4}; 

    static constexpr VpBlockProperties c_Vp_Profile_Capabilities[] = {
        MAKE_BLOCK_CAPABILITIES("Baseline"),
        MAKE_BLOCK_CAPABILITIES("DescriptorHeap"),
        MAKE_BLOCK_CAPABILITIES("PageableDeviceLocalMemory"),
        MAKE_BLOCK_CAPABILITIES("UnifiedImageLayouts"),
        MAKE_BLOCK_CAPABILITIES("ShaderStencilExport"),
        MAKE_BLOCK_CAPABILITIES("MeshAndTaskShaderDerivatives"),
        MAKE_BLOCK_CAPABILITIES("Maintenance9"),
        MAKE_BLOCK_CAPABILITIES("InputOutput16"),
        MAKE_BLOCK_CAPABILITIES("SwapchainMaintenance1"),
        MAKE_BLOCK_CAPABILITIES("RayTracingInvocationReorder"),
        MAKE_BLOCK_CAPABILITIES("OpacityMicromap"),
        MAKE_BLOCK_CAPABILITIES("DebugUtils"),
    };

    static const char* c_Required_Blocks[] = {
        "Baseline"
    };

    static const char* c_Optional_Blocks[] = { "DescriptorHeap", "PageableDeviceLocalMemory", "UnifiedImageLayouts",
        "ShaderStecilExport", "MeshAndTaskShaderDerivatives", "Maintenance9",
        "SwapchainMaintenance1", "DebugUtils""RayTracingInvocationReorder", "OpacityMicromap" };

    #undef MAKE_BLOCK_CAPABILITIES

    static std::vector<std::string> BlockDeviceExts(VpFunctions vp_funcs, const VpProfileProperties& p, const char* block) {
        uint32_t n = 0;
        vpGetProfileDeviceExtensionProperties(vp_funcs, &p, block, &n, nullptr);
        std::vector<VkExtensionProperties> v(n);
        vpGetProfileDeviceExtensionProperties(vp_funcs, &p, block, &n, v.data());
        std::vector<std::string> out;
        for (auto& e : v) out.emplace_back(e.extensionName);
        return out;
    }

    static vk::Device CreateDeviceManual(VpFunctions vp_funcs, vk::Instance instance, vk::PhysicalDevice physical_device, DeviceSettings device_settings, 
        VulkanSettings vk_settings, std::span<const vk::DeviceQueueCreateInfo> queue_infos, Vk::DeviceFeatures &out_features) {
        vk::StructureChain<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan12Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceVulkan14Features,
            vk::PhysicalDeviceConditionalRenderingFeaturesEXT,
            vk::PhysicalDeviceDescriptorBufferFeaturesEXT,
            vk::PhysicalDeviceMeshShaderFeaturesEXT,
            vk::PhysicalDeviceRayQueryFeaturesKHR,
            vk::PhysicalDeviceFragmentShadingRateFeaturesKHR,
            vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR,
            vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
            vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
            vk::PhysicalDeviceComputeShaderDerivativesFeaturesKHR,
            vk::PhysicalDeviceDescriptorHeapFeaturesEXT,
            vk::PhysicalDeviceShaderUntypedPointersFeaturesKHR,
            vk::PhysicalDevicePageableDeviceLocalMemoryFeaturesEXT,
            vk::PhysicalDeviceMemoryPriorityFeaturesEXT,
            vk::PhysicalDeviceUnifiedImageLayoutsFeaturesKHR,
            vk::PhysicalDeviceMaintenance9FeaturesKHR,
            vk::PhysicalDeviceSwapchainMaintenance1FeaturesKHR,
            vk::PhysicalDeviceRayTracingInvocationReorderFeaturesEXT,
            vk::PhysicalDeviceOpacityMicromapFeaturesEXT,
            vk::PhysicalDeviceMutableDescriptorTypeFeaturesEXT
        > chain;
        auto& f2 = chain.get<vk::PhysicalDeviceFeatures2>();

        auto available = physical_device.enumerateDeviceExtensionProperties();
        auto hasExt = [&](const std::string& name) {
            return std::any_of(available.begin(), available.end(), [&](auto& a) {
                return name == a.extensionName; });
        };

        std::vector<const char *> optional_blocks;

        {
            for (const char *b : c_Optional_Blocks) {
                const std::string block_name(b);

                if (block_name == "DescriptorHeap") {
                    out_features.descriptor_heap = !vk_settings.no_descriptor_heap;
                    if (vk_settings.no_descriptor_heap) continue;
                }
                else if (block_name == "PageableDeviceLocalMemory") {
                    out_features.pageable_device_local_memory = !vk_settings.no_pageable_device_local_memory;
                    if (vk_settings.no_pageable_device_local_memory) continue;
                }
                else if (block_name == "UnifiedImageLayouts") {
                    out_features.unified_image_layouts = !vk_settings.no_unified_image_layouts;
                    if (vk_settings.no_unified_image_layouts) continue;
                }
                else if (block_name == "Maintenance9") {
                    out_features.maintenance9 = !vk_settings.no_maintenance9;
                    if (vk_settings.no_maintenance9) continue;
                }
                else if (block_name == "InputOutput16") {
                    out_features.input_output_16 = !vk_settings.no_input_output_16;
                    if (vk_settings.no_input_output_16) continue;
                }
                else if (block_name == "SwapchainMaintenance1") {
                    out_features.swapchain_maintenance1 = !vk_settings.no_swapchain_maintenance1;
                    if (vk_settings.no_swapchain_maintenance1) continue;
                }
                else if (block_name == "ShaderStencilExport") {
                    if (!device_settings.shader_stencil_export) continue;
                }
                else if (block_name == "RayTracingInvocationReorder") {
                    if (!device_settings.ray_tracing_invocation_reorder) continue;
                }
                else if (block_name == "OpacityMicromap") {
                    if (!device_settings.opacity_micromap) continue;
                }

                optional_blocks.push_back(b);
            }
        }

        std::set<std::string> ext_set;

        for (const char* b : c_Required_Blocks) {
            vpGetProfileFeatures(VpFunctions{}, &c_Vp_Profile, b, &f2);
            for (auto& e : BlockDeviceExts(vp_funcs, c_Vp_Profile, b)) ext_set.insert(e);
        }

        for (const char* b : optional_blocks) {
            auto exts = BlockDeviceExts(vp_funcs, c_Vp_Profile, b);
            if (!std::all_of(exts.begin(), exts.end(), hasExt)) continue;
            vpGetProfileFeatures(VpFunctions{}, &c_Vp_Profile, b, &f2);
            for (auto& e : exts) ext_set.insert(e);
        }

        auto sup = physical_device.getFeatures2<vk::PhysicalDeviceFeatures2,
                                    vk::PhysicalDeviceVulkan11Features>();

        chain.get<vk::PhysicalDeviceVulkan11Features>().storageInputOutput16 =
            !vk_settings.no_input_output_16 && sup.get<vk::PhysicalDeviceVulkan11Features>().storageInputOutput16;

        std::vector<const char*> extension_names;
        for (auto& e : ext_set) extension_names.push_back(e.c_str());

        const auto device_create_info
            = vk::DeviceCreateInfo{}
                .setPNext(&f2)
                .setQueueCreateInfos(queue_infos)
                .setPEnabledExtensionNames(extension_names)
                ;

        return physical_device.createDevice(device_create_info);
    }

    [[nodiscard]] Device *CreateVulkanDevice(const DeviceInfo &device_info, DeviceSettings device_settings, const VulkanSettings &vk_settings) {
        return new Vk::Device(device_info, device_settings, vk_settings);
    }

    [[nodiscard]] static std::vector<VpBlockProperties> GetSupportedBlocks(std::span<const VpBlockProperties> prop, bool is_supported) {
        if (is_supported) 
            return std::vector(prop.begin(), prop.end());

        std::vector<VpBlockProperties> out;

        for (const auto &block_props : c_Vp_Profile_Capabilities) {
            b8 is_unsupported{false};
            for (const auto &unsupported_block : prop)
                if (std::string(block_props.blockName) == unsupported_block.blockName) 
                    is_unsupported = true;

            if (!is_unsupported)
                out.emplace_back(block_props);
        }

        return out;
    }

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
            SET_VK_FUNC(vkDestroyInstance);
        }
        DEFINE_VK_FUNC(vkEnumerateInstanceExtensionProperties);
        DEFINE_VK_FUNC(vkEnumeratePhysicalDevices);
        DEFINE_VK_FUNC(vkGetPhysicalDeviceMemoryProperties);
        DEFINE_VK_FUNC(vkGetPhysicalDeviceProperties2);
        DEFINE_VK_FUNC(vkDestroyInstance);

        #undef DEFINE_VK_FUNC
        #undef SET_VK_FUNC
    };

    [[nodiscard]] static DeviceInfo GetDeviceInfo(vk::PhysicalDevice device, std::span<const VpBlockProperties> block_properties, QueryDispatcher &query_dispatcher) {
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
                = blocks.contains("MeshAndTaskShaderDerivatives");

            device_info_out.features.shader_stencil_export 
                = blocks.contains("ShaderStecilExport");

            device_info_out.features.ray_tracing_invocation_reorder 
                = blocks.contains("RayTracingInvocationReorder");

            device_info_out.features.opacity_micromap
                = blocks.contains("OpacityMicromap");
        }

        return device_info_out;
    } 

    std::vector<DeviceInfo> QuerySupportedVulkanDevices() {
        vk::detail::DynamicLoader dyn_loader;
        QueryDispatcher query_dispatcher(dyn_loader);

        const auto vp_functions = CreateVpFunctions(dyn_loader);

        VkBool32 supported{};

        u32 block_count{};
        vpGetInstanceProfileVariantsSupport(vp_functions, nullptr, &c_Vp_Profile, &supported, &block_count, nullptr);
        std::vector<VpBlockProperties> block_properties(block_count);
        vpGetInstanceProfileVariantsSupport(vp_functions, nullptr, &c_Vp_Profile, &supported, &block_count, block_properties.data());

        const auto supported_blocks = GetSupportedBlocks(block_properties, supported);
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
        vp_instance_create_info.enabledProfileBlockCount = supported_blocks.size();
        vp_instance_create_info.pEnabledProfileBlocks = supported_blocks.data();

        vk::Instance instance;
        auto result = vpCreateInstance(vp_functions, &vp_instance_create_info, nullptr, reinterpret_cast<VkInstance *>(&instance));
        if (result != VK_SUCCESS) return {};

        std::vector<DeviceInfo> device_infos;
        const auto devices = instance.enumeratePhysicalDevices(query_dispatcher);

        for (const auto device : devices) {
            u32 device_block_count{};
            vpGetPhysicalDeviceProfileVariantsSupport(vp_functions, instance, device, &c_Vp_Profile, &supported, &device_block_count, nullptr);
            std::vector<VpBlockProperties> device_block_properties(device_block_count);
            vpGetPhysicalDeviceProfileVariantsSupport(vp_functions, instance, device, &c_Vp_Profile, &supported, &device_block_count, device_block_properties.data());

            const auto supported_device_blocks = GetSupportedBlocks(device_block_properties, supported);

            {
                b8 is_supported{false};
                for (const auto &block : device_block_properties)
                    if (std::string(block.blockName) == "Baseline") is_supported = true;

                if (is_supported) continue;
            }

            device_infos.emplace_back(
                GetDeviceInfo(device, supported_device_blocks, query_dispatcher)
            );
        }

        instance.destroy(nullptr, query_dispatcher);
        return device_infos;
    }
}

namespace DnmRHI::Vk {
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

    static vk::Bool32 DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                vk::DebugUtilsMessageTypeFlagsEXT message_types,
                                const vk::DebugUtilsMessengerCallbackDataEXT *callback_data,
                                void *user_data) {
        constexpr auto unnecessary_messages = std::array{306759332, 2132353751, 2132353751, -2030147807, 601872502, 615892639, 1734198062};
        for (const i32 i : unnecessary_messages)
            if (callback_data->messageIdNumber == i) return vk::True;

        if (message_severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
            std::println("Vulkan Error; message id {}, message: {}", callback_data->messageIdNumber, callback_data->pMessage);
        }
        else if (message_severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
            std::println("Vulkan Warn; message id {}, message: {}", callback_data->messageIdNumber, callback_data->pMessage);
        }
        else {
            std::println("Vulkan Info; message id {}, message: {}", callback_data->messageIdNumber, callback_data->pMessage);
        }
        return vk::True;
    }

    Device::Device(const DeviceInfo &info, DeviceSettings device_settings, const VulkanSettings &settings) noexcept 
    : DnmRHI::Device(info, device_settings), m_vk_settings(settings) {}

    Device::~Device() noexcept {
        if (m_device) GetDevice().waitIdle();

        if (m_descriptor_manager) delete m_descriptor_manager;
        if (m_vma_allocator) vmaDestroyAllocator(m_vma_allocator);
        if (m_debug_messenger) m_instance.destroy(m_debug_messenger);
        if (m_device) m_device.destroy();

        if (m_instance) m_instance.destroy();
    }

    void Device::IInit() noexcept {
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));
        const auto vp_functions = CreateVpFunctions(m_dl);

        CreateInstance(vp_functions);
        CreateDevice(vp_functions);
        CreateVmaAllocator();
        CreateDescriptorManager();
    }

    void Device::CreateInstance(const VpFunctions &vp_functions) noexcept {
        std::vector<const char *> extensions;
        std::vector<const char *> layers;

        VkBool32 supported{};

        u32 block_count{};
        vpGetInstanceProfileVariantsSupport(vp_functions, nullptr, &c_Vp_Profile, &supported, &block_count, nullptr);
        std::vector<VpBlockProperties> block_properties(block_count);
        vpGetInstanceProfileVariantsSupport(vp_functions, nullptr, &c_Vp_Profile, &supported, &block_count, block_properties.data());
        auto supported_blocks = GetSupportedBlocks(block_properties, supported);

        extensions.insert_range(extensions.end(), GetSurfaceExtensions(vk::enumerateInstanceExtensionProperties(nullptr)));

        for (const auto *ext : extensions) {
            const std::string name(ext);
            if (name == "VK_KHR_win32_surface")
                m_vk_device_features.win32_surface = true;
            else if (name == "VK_KHR_wayland_surface")
                m_vk_device_features.wayland_surface = true;
            else if (name == "VK_KHR_xcb_surface")
                m_vk_device_features.xcb_surface = true;
            else if (name == "VK_KHR_xlib_surface")
                m_vk_device_features.xlib_surface = true;
        }

        const auto debug_utils_it = std::ranges::find_if(supported_blocks, 
                [] (const auto &block) {
                    return std::string(block.blockName) == "DebugUtils";
                });

        if (GetSettings().validation_layer) {
            if (debug_utils_it == supported_blocks.end()) return;

            b8 validation_layer_supported;
            for (const auto &prop : vk::enumerateInstanceLayerProperties())
                if (std::string(prop.layerName) == "VK_LAYER_KHRONOS_validation") validation_layer_supported = true;

            if (!validation_layer_supported) return;

            layers.emplace_back("VK_LAYER_KHRONOS_validation");
        }
        else {
            supported_blocks.erase(debug_utils_it);
        }

        const auto app_info
            = vk::ApplicationInfo{}
                .setApplicationVersion(VK_MAKE_VERSION(0, 1, 0))
                .setEngineVersion(VK_MAKE_VERSION(0, 1, 0))
                .setApiVersion(VK_API_VERSION_1_4)
                .setPEngineName("DnmEngine")
                .setPApplicationName("SomeGame")
                ;

        auto enabled_features = std::vector{
            // TODO: my gpu crached when best practices enabled(, and validation layer)
            // vk::ValidationFeatureEnableEXT::eBestPractices,
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
                                //  | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                                //  | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
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
                .setPEnabledExtensionNames(extensions)
                .setPEnabledLayerNames(layers)
                .setPApplicationInfo(&app_info)
                .setPNext(&debug_create_info)
                ;

        VpInstanceCreateInfo vp_instance_create_info{};
        vp_instance_create_info.pCreateInfo = reinterpret_cast<const VkInstanceCreateInfo *>(&instance_create_info);
        vp_instance_create_info.enabledProfileBlockCount = supported_blocks.size();
        vp_instance_create_info.pEnabledProfileBlocks = supported_blocks.data();

        auto result = vpCreateInstance(vp_functions, &vp_instance_create_info, nullptr, reinterpret_cast<VkInstance *>(&m_instance));
        if (result != VK_SUCCESS) return;

        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

        if (GetSettings().validation_layer) m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(debug_create_info);    
    }

    void Device::CreateDevice(const VpFunctions &vp_functions) noexcept {
        for (const auto device : m_instance.enumeratePhysicalDevices()) {
            vk::PhysicalDeviceVulkan11Properties vk11{};
            vk::PhysicalDeviceProperties2 properties{};
            properties.pNext = &vk11;
            device.getProperties2(&properties);

            if (vk11.deviceUUID == m_device_info.device_uuid) {
                m_physical_device = device;
                break;
            }
        }

        if (m_physical_device == VK_NULL_HANDLE) return;
 
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

            //? 
            m_vk_device_properties.pipeline_cache_uuid = m_physical_device.getProperties().pipelineCacheUUID;
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

        m_device = CreateDeviceManual(vp_functions, m_instance, m_physical_device, m_settings, m_vk_settings, queues, m_vk_device_features);

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
        create_info.vulkanApiVersion = VK_API_VERSION_1_4;
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
            m_vk_device_features.descriptor_heap ? 
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
    
    void Device::ISetUniformBufferBindlessIndex(u32 index, DnmRHI::Buffer &buffer, Subresource subresource) noexcept {
        m_descriptor_manager->SetUniformBufferBindlessIndex(index, static_cast<Vk::Buffer &>(buffer), subresource);
    }

    void Device::ISetSampledImageBindlessIndex(u32 index, DnmRHI::Image &image, Subresource subresource) noexcept {
        m_descriptor_manager->SetSampledImageBindlessIndex(index, static_cast<Vk::Image &>(image), subresource);
    }

    void Device::ISetStorageImageBindlessIndex(u32 index, DnmRHI::Image &image, Subresource subresource) noexcept {
        m_descriptor_manager->SetStorageImageBindlessIndex(index, static_cast<Vk::Image &>(image), subresource);
    }

    void Device::ISetSamplerBindlessIndex(u32 index, const SamplerDesc &sampler) noexcept {
        m_descriptor_manager->SetSamplerBindlessIndex(index, sampler);
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