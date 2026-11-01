
/*
 * Copyright (C) 2021-2026 Valve Corporation
 * Copyright (C) 2021-2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file is ***GENERATED***.  Do Not Edit.
 * See scripts/gen_profiles_solution.py for modifications.
 */

#ifndef VULKAN_PROFILES_H_
#define VULKAN_PROFILES_H_ 1

#define VPAPI_ATTR

#ifdef __cplusplus
    extern "C" {
#endif

#include <vulkan/vulkan.h>

#if defined(VK_VERSION_1_4) && \
    defined(VK_EXT_calibrated_timestamps) && \
    defined(VK_EXT_conditional_rendering) && \
    defined(VK_EXT_conservative_rasterization) && \
    defined(VK_EXT_debug_utils) && \
    defined(VK_EXT_descriptor_buffer) && \
    defined(VK_EXT_descriptor_heap) && \
    defined(VK_EXT_memory_budget) && \
    defined(VK_EXT_memory_priority) && \
    defined(VK_EXT_mesh_shader) && \
    defined(VK_EXT_mutable_descriptor_type) && \
    defined(VK_EXT_opacity_micromap) && \
    defined(VK_EXT_pageable_device_local_memory) && \
    defined(VK_EXT_ray_tracing_invocation_reorder) && \
    defined(VK_EXT_shader_stencil_export) && \
    defined(VK_KHR_acceleration_structure) && \
    defined(VK_KHR_compute_shader_derivatives) && \
    defined(VK_KHR_compute_shader_derivatives) && \
    defined(VK_KHR_deferred_host_operations) && \
    defined(VK_KHR_fragment_shading_rate) && \
    defined(VK_KHR_get_surface_capabilities2) && \
    defined(VK_KHR_maintenance9) && \
    defined(VK_KHR_ray_query) && \
    defined(VK_KHR_ray_tracing_maintenance1) && \
    defined(VK_KHR_ray_tracing_pipeline) && \
    defined(VK_KHR_shader_untyped_pointers) && \
    defined(VK_KHR_surface) && \
    defined(VK_KHR_surface_maintenance1) && \
    defined(VK_KHR_swapchain) && \
    defined(VK_KHR_swapchain_maintenance1) && \
    defined(VK_KHR_unified_image_layouts)
#define VP_DNMRHI_VulkanRequirements 1
#define VP_DNMRHI_VULKANREQUIREMENTS_NAME "VP_DNMRHI_VulkanRequirements"
#define VP_DNMRHI_VULKANREQUIREMENTS_SPEC_VERSION 0
#define VP_DNMRHI_VULKANREQUIREMENTS_MIN_API_VERSION VK_MAKE_VERSION(1, 4, 0)
#endif

#define VP_HEADER_VERSION_COMPLETE VK_MAKE_API_VERSION(0, 2, 0, VK_HEADER_VERSION)

#define VP_MAX_PROFILE_NAME_SIZE 256U

typedef struct VpProfileProperties {
    char        profileName[VP_MAX_PROFILE_NAME_SIZE];
    uint32_t    specVersion;
} VpProfileProperties;

typedef struct VpBlockProperties {
    VpProfileProperties profiles;
    uint32_t apiVersion;
    char blockName[VP_MAX_PROFILE_NAME_SIZE];
} VpBlockProperties;

typedef struct VpVideoProfileProperties {
    char name[VP_MAX_PROFILE_NAME_SIZE];
} VpVideoProfileProperties;

typedef enum VpInstanceCreateFlagBits {
    VP_INSTANCE_CREATE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VpInstanceCreateFlagBits;
typedef VkFlags VpInstanceCreateFlags;

typedef struct VpInstanceCreateInfo {
    const VkInstanceCreateInfo* pCreateInfo;
    VpInstanceCreateFlags       flags;
    uint32_t                    enabledFullProfileCount;
    const VpProfileProperties*  pEnabledFullProfiles;
    uint32_t                    enabledProfileBlockCount;
    const VpBlockProperties*    pEnabledProfileBlocks;
} VpInstanceCreateInfo;

typedef enum VpDeviceCreateFlagBits {
    VP_DEVICE_CREATE_DISABLE_ROBUST_BUFFER_ACCESS_BIT = 0x0000001,
    VP_DEVICE_CREATE_DISABLE_ROBUST_IMAGE_ACCESS_BIT = 0x0000002,
    VP_DEVICE_CREATE_DISABLE_ROBUST_ACCESS =
        VP_DEVICE_CREATE_DISABLE_ROBUST_BUFFER_ACCESS_BIT | VP_DEVICE_CREATE_DISABLE_ROBUST_IMAGE_ACCESS_BIT,

    VP_DEVICE_CREATE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VpDeviceCreateFlagBits;
typedef VkFlags VpDeviceCreateFlags;

typedef struct VpDeviceCreateInfo {
    const VkDeviceCreateInfo*   pCreateInfo;
    VpDeviceCreateFlags         flags;
    uint32_t                    enabledFullProfileCount;
    const VpProfileProperties*  pEnabledFullProfiles;
    uint32_t                    enabledProfileBlockCount;
    const VpBlockProperties*    pEnabledProfileBlocks;
} VpDeviceCreateInfo;

VK_DEFINE_HANDLE(VpFunctions)

typedef VpFunctions VpCapabilities;

typedef enum VpFunctionsCreateFlagBits {
    VP_FUNCTIONS_CREATE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VpFunctionsCreateFlagBits;

typedef VkFlags VpFunctionsCreateFlags;

typedef enum VpInstanceFunctionsLoadFlagBits {
    VP_INSTANCE_FUNCTIONS_LOAD_KHR_GET_PHYSICAL_DEVICE_PROPERTIES2_BIT = (1 << 0),
    VP_INSTANCE_FUNCTIONS_LOAD_MISSING_ONLY_BIT = (1 << 1),
    VP_INSTANCE_FUNCTIONS_LOAD_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VpInstanceFunctionsLoadFlagBits;

typedef VkFlags VpInstanceFunctionsLoadFlags;

struct VpFunctions_T {
    static VpFunctions_T& Get() {
        static VpFunctions_T instance;
        return instance;
    }

    VpFunctions_T() {
#ifndef VK_NO_PROTOTYPES
        ImportVulkanFunctions_Static();
#endif//VK_NO_PROTOTYPES
    }

    VkResult validate(bool full_init = false) const {
        // Validate global vulkan function initialization
        // vkEnumerateInstanceVersion is omitted from validation on purpose.
        // It is not available in Vulkan 1.0, and nullptr is a valid state indicating Vulkan 1.0.

        if (this->EnumerateInstanceExtensionProperties == nullptr ||
            this->CreateInstance == nullptr ||
            this->GetInstanceProcAddr == nullptr) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }

        if (full_init) {
            if (this->GetPhysicalDeviceFeatures2 == nullptr ||
                this->GetPhysicalDeviceProperties2 == nullptr ||
                this->GetPhysicalDeviceFormatProperties2 == nullptr ||
                this->GetPhysicalDeviceQueueFamilyProperties2 == nullptr) {
                return VK_ERROR_INITIALIZATION_FAILED;
            }
        }

        return VK_SUCCESS;
    }

    PFN_vkGetInstanceProcAddr GetInstanceProcAddr = nullptr;
    PFN_vkEnumerateInstanceVersion EnumerateInstanceVersion;
    PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties = nullptr;
    PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties = nullptr;
    PFN_vkGetPhysicalDeviceFeatures2 GetPhysicalDeviceFeatures2 = nullptr;
    PFN_vkGetPhysicalDeviceProperties2 GetPhysicalDeviceProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceFormatProperties2 GetPhysicalDeviceFormatProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties2 GetPhysicalDeviceQueueFamilyProperties2 = nullptr;
    PFN_vkCreateInstance CreateInstance = nullptr;
    PFN_vkCreateDevice CreateDevice = nullptr;

private:
#ifndef VK_NO_PROTOTYPES
    void ImportVulkanFunctions_Static() {
        #define VP_SET_STATIC(memberName, staticFuncName) this->memberName = (PFN_vk##memberName)staticFuncName

        VP_SET_STATIC(GetInstanceProcAddr, vkGetInstanceProcAddr);

        VP_SET_STATIC(EnumerateInstanceVersion, vkEnumerateInstanceVersion);
        VP_SET_STATIC(EnumerateInstanceExtensionProperties, vkEnumerateInstanceExtensionProperties);
        VP_SET_STATIC(EnumerateDeviceExtensionProperties, vkEnumerateDeviceExtensionProperties);

        VP_SET_STATIC(GetPhysicalDeviceFeatures2, vkGetPhysicalDeviceFeatures2);
        VP_SET_STATIC(GetPhysicalDeviceProperties2, vkGetPhysicalDeviceProperties2);
        VP_SET_STATIC(GetPhysicalDeviceFormatProperties2, vkGetPhysicalDeviceFormatProperties2);
        VP_SET_STATIC(GetPhysicalDeviceQueueFamilyProperties2, vkGetPhysicalDeviceQueueFamilyProperties2);

        VP_SET_STATIC(CreateInstance, vkCreateInstance);
        VP_SET_STATIC(CreateDevice, vkCreateDevice);

        #undef VP_SET_STATIC
    }
#endif//VK_NO_PROTOTYPES
};

/// Description of a Allocator to be created.
typedef struct VpFunctionsCreateInfo
{
    /// Flags for created allocator. Use #VpInstanceCreateFlagBits enum.
    VpFunctionsCreateFlags          flags;

    PFN_vkGetInstanceProcAddr GetInstanceProcAddr = nullptr;
    PFN_vkEnumerateInstanceVersion EnumerateInstanceVersion;
    PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties = nullptr;
    PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties = nullptr;
    PFN_vkCreateInstance CreateInstance = nullptr;
    PFN_vkCreateDevice CreateDevice = nullptr;
    PFN_vkGetPhysicalDeviceFeatures2 GetPhysicalDeviceFeatures2 = nullptr;
    PFN_vkGetPhysicalDeviceProperties2 GetPhysicalDeviceProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceFormatProperties2 GetPhysicalDeviceFormatProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties2 GetPhysicalDeviceQueueFamilyProperties2 = nullptr;
} VpFunctionsCreateInfo;

#ifdef VP_USE_OBJECT

VPAPI_ATTR VkResult vpCreateFunctions(
    const VpFunctionsCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VpFunctions*                                pFunctions
);

/// Destroys allocator object.
VPAPI_ATTR void vpDestroyFunctions(
    VpFunctions                                 functions,
    const VkAllocationCallbacks*                pAllocator);

#endif//VP_USE_OBJECT

/// Helper function to initialize a VpFunctions instance with global Vulkan functions loaded with GetInstanceProcAddr
VPAPI_ATTR VkResult vpInitializeGlobalFunctions(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    PFN_vkGetInstanceProcAddr                   GetInstanceProcAddr);

/// Initializes capabilities with instance functions
VPAPI_ATTR VkResult vpInitializeInstanceFunctions(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    VkInstance                                  instance,
    VpInstanceFunctionsLoadFlags                flags);

// Query the list of available profiles in the library
VPAPI_ATTR VkResult vpGetProfiles(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    uint32_t*                                   pPropertyCount,
    VpProfileProperties*                        pProperties);

// List the required profiles of a profile
VPAPI_ATTR VkResult vpGetProfileRequiredProfiles(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    uint32_t*                                   pPropertyCount,
    VpProfileProperties*                        pProperties);

// Query the profile required Vulkan API version
VPAPI_ATTR uint32_t vpGetProfileAPIVersion(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile);

// List the recommended fallback profiles of a profile
VPAPI_ATTR VkResult vpGetProfileFallbacks(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    uint32_t*                                   pPropertyCount,
    VpProfileProperties*                        pProperties);

// Query whether the profile has multiple variants. Profiles with multiple variants can only use vpGetInstanceProfileSupport and vpGetPhysicalDeviceProfileSupport capabilities of the library. Other function will return a VK_ERROR_UNKNOWN error
VPAPI_ATTR VkResult vpHasMultipleVariantsProfile(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    VkBool32*                                   pHasMultipleVariants);

// Check whether a profile is supported at the instance level
VPAPI_ATTR VkResult vpGetInstanceProfileSupport(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const char*                                 pLayerName,
    const VpProfileProperties*                  pProfile,
    VkBool32*                                   pSupported);

// Check whether a variant of a profile is supported at the instance level and report this list of blocks used to validate the profiles
VPAPI_ATTR VkResult vpGetInstanceProfileVariantsSupport(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const char*                                 pLayerName,
    const VpProfileProperties*                  pProfile,
    VkBool32*                                   pSupported,
    uint32_t*                                   pPropertyCount,
    VpBlockProperties*                          pProperties);

// Create a VkInstance with the profile instance extensions enabled
VPAPI_ATTR VkResult vpCreateInstance(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance);

// Check whether a profile is supported by the physical device
VPAPI_ATTR VkResult vpGetPhysicalDeviceProfileSupport(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    VkInstance                                  instance,
    VkPhysicalDevice                            physicalDevice,
    const VpProfileProperties*                  pProfile,
    VkBool32*                                   pSupported);

// Check whether a variant of a profile is supported by the physical device and report this list of blocks used to validate the profiles
VPAPI_ATTR VkResult vpGetPhysicalDeviceProfileVariantsSupport(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    VkInstance                                  instance,
    VkPhysicalDevice                            physicalDevice,
    const VpProfileProperties*                  pProfile,
    VkBool32*                                   pSupported,
    uint32_t*                                   pPropertyCount,
    VpBlockProperties*                          pProperties);

// Create a VkDevice with the profile features and device extensions enabled
VPAPI_ATTR VkResult vpCreateDevice(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    VkPhysicalDevice                            physicalDevice,
    const VpDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice);

// Query the list of instance extensions of a profile
VPAPI_ATTR VkResult vpGetProfileInstanceExtensionProperties(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties);

// Query the list of device extensions of a profile
VPAPI_ATTR VkResult vpGetProfileDeviceExtensionProperties(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t*                                   pPropertyCount,
    VkExtensionProperties*                      pProperties);

// Fill the feature structures with the requirements of a profile
VPAPI_ATTR VkResult vpGetProfileFeatures(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    void*                                       pNext);

// Query the list of feature structure types specified by the profile
VPAPI_ATTR VkResult vpGetProfileFeatureStructureTypes(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t*                                   pStructureTypeCount,
    VkStructureType*                            pStructureTypes);

// Fill the property structures with the requirements of a profile
VPAPI_ATTR VkResult vpGetProfileProperties(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    void*                                       pNext);

// Query the list of property structure types specified by the profile
VPAPI_ATTR VkResult vpGetProfilePropertyStructureTypes(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t*                                   pStructureTypeCount,
    VkStructureType*                            pStructureTypes);

// Fill the queue family property structures with the requirements of a profile
VPAPI_ATTR VkResult vpGetProfileQueueFamilyProperties(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t*                                   pPropertyCount,
    VkQueueFamilyProperties2KHR*                pProperties);

// Query the list of queue family property structure types specified by the profile
VPAPI_ATTR VkResult vpGetProfileQueueFamilyStructureTypes(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t*                                   pStructureTypeCount,
    VkStructureType*                            pStructureTypes);

// Query the list of formats with specified requirements by a profile
VPAPI_ATTR VkResult vpGetProfileFormats(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t*                                   pFormatCount,
    VkFormat*                                   pFormats);

// Query the requirements of a format for a profile
VPAPI_ATTR VkResult vpGetProfileFormatProperties(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    VkFormat                                    format,
    void*                                       pNext);

// Query the list of format structure types specified by the profile
VPAPI_ATTR VkResult vpGetProfileFormatStructureTypes(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t*                                   pStructureTypeCount,
    VkStructureType*                            pStructureTypes);

#ifdef VK_KHR_video_queue
// Query the list of video profiles specified by the profile
VPAPI_ATTR VkResult vpGetProfileVideoProfiles(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t*                                   pVideoProfileCount,
    VpVideoProfileProperties*                   pVideoProfiles);

// Query the video profile info structures for a video profile defined by a profile
VPAPI_ATTR VkResult vpGetProfileVideoProfileInfo(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t                                    videoProfileIndex,
    VkVideoProfileInfoKHR*                      pVideoProfileInfo);

// Query the list of video profile info structure types specified by the profile for a video profile
VPAPI_ATTR VkResult vpGetProfileVideoProfileInfoStructureTypes(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t                                    videoProfileIndex,
    uint32_t*                                   pStructureTypeCount,
    VkStructureType*                            pStructureTypes);

// Query the video capabilities requirements for a video profile defined by a profile
VPAPI_ATTR VkResult vpGetProfileVideoCapabilities(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t                                    videoProfileIndex,
    void*                                       pNext);

// Query the list of video capability structure types specified by the profile for a video profile
VPAPI_ATTR VkResult vpGetProfileVideoCapabilityStructureTypes(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t                                    videoProfileIndex,
    uint32_t*                                   pStructureTypeCount,
    VkStructureType*                            pStructureTypes);

// Query the video format property requirements for a video profile defined by a profile
VPAPI_ATTR VkResult vpGetProfileVideoFormatProperties(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t                                    videoProfileIndex,
    uint32_t*                                   pPropertyCount,
    VkVideoFormatPropertiesKHR*                 pProperties);

// Query the list of video format property structure types specified by the profile for a video profile
VPAPI_ATTR VkResult vpGetProfileVideoFormatStructureTypes(
#ifdef VP_USE_OBJECT
    VpFunctions                                 functions,
#endif//VP_USE_OBJECT
    const VpProfileProperties*                  pProfile,
    const char*                                 pBlockName,
    uint32_t                                    videoProfileIndex,
    uint32_t*                                   pStructureTypeCount,
    VkStructureType*                            pStructureTypes);
#endif  // VK_KHR_video_queue

#ifdef __cplusplus
}
#endif

#endif // VULKAN_PROFILES_H_
