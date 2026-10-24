#include <DnmRHI/Platform/Vulkan/Image.hpp>
#include <DnmRHI/Platform/Vulkan/ToVk.hpp>
#include <set>

namespace DnmRHI::Vk {
    namespace Utility {
        [[nodiscard]] constexpr DnmRHI::Format VkFormatToRHIImageFormat(vk::Format format) noexcept {
            switch (format) {
                case vk::Format::eB8G8R8A8Srgb: return DnmRHI::Format::eB8G8R8A8_Srgb;
                case vk::Format::eB8G8R8A8Unorm: return DnmRHI::Format::eB8G8R8A8_Unorm;

                case vk::Format::eR8G8B8A8Srgb: return DnmRHI::Format::eR8G8B8A8_Srgb;
                case vk::Format::eR8G8B8A8Unorm: return DnmRHI::Format::eR8G8B8A8_Unorm;
            default: std::unreachable();
            }
        }

        // TODO: queue flags are necessery use them
        constexpr auto SelectDefaultLayout(ImageFlags flags, b8 unified_image_layouts) noexcept {
            const auto general_queue = flags.Has(DnmRHI::ImageFlagBit::eGeneralQueue);
            const auto compute_queue = flags.Has(DnmRHI::ImageFlagBit::eComputeQueue);
            const auto copy_queue = flags.Has(DnmRHI::ImageFlagBit::eCopyQueue);

            const auto single_queue = (general_queue + compute_queue + copy_queue) < 2;

            if (flags.Has(ImageFlagBit::eStorage) 
                || unified_image_layouts
                || (!single_queue && (copy_queue && general_queue) || (copy_queue && compute_queue))
            )
                return vk::ImageLayout::eGeneral;
            else if (flags.Has(ImageFlagBit::eSampled))
                return vk::ImageLayout::eReadOnlyOptimal;
            else if (flags.Has(ImageFlagBit::eAttachment))
                return vk::ImageLayout::eAttachmentOptimal;
            else if (flags.Has(ImageFlagBit::eVariableRateShadingAttachment))
                return vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR;
            else if (flags.Has(ImageFlagBit::eCopySrc))
                return vk::ImageLayout::eTransferSrcOptimal;
            else if (flags.Has(ImageFlagBit::eCopyDst))
                return vk::ImageLayout::eTransferDstOptimal;
            return vk::ImageLayout::eGeneral;
        }

        constexpr auto SelectAttachmentLayout(ImageFlags flags, b8 unified_image_layouts) noexcept {
            if (flags.Has(ImageFlagBit::eAttachment)) {
                const auto general_queue = flags.Has(DnmRHI::ImageFlagBit::eGeneralQueue);
                const auto compute_queue = flags.Has(DnmRHI::ImageFlagBit::eComputeQueue);
                const auto copy_queue = flags.Has(DnmRHI::ImageFlagBit::eCopyQueue);

                const auto single_queue = (general_queue + copy_queue) < 2;

                if (flags.Has(ImageFlagBit::eStorage) || unified_image_layouts || (!single_queue && (copy_queue && general_queue) || (copy_queue && compute_queue)))
                    return vk::ImageLayout::eGeneral;
                else
                    return vk::ImageLayout::eAttachmentOptimal;
            }
            return vk::ImageLayout::eUndefined;
        }

        constexpr auto SelectCopySrcLayout(ImageFlags flags, b8 unified_image_layouts) noexcept {
            if (flags.Has(ImageFlagBit::eCopySrc)) {
                const auto general_queue = flags.Has(DnmRHI::ImageFlagBit::eGeneralQueue);
                const auto compute_queue = flags.Has(DnmRHI::ImageFlagBit::eComputeQueue);
                const auto copy_queue = flags.Has(DnmRHI::ImageFlagBit::eCopyQueue);

                const auto single_queue = (general_queue + compute_queue + copy_queue) < 2;

                if (flags.Has(ImageFlagBit::eStorage) || unified_image_layouts || (!single_queue && (copy_queue && general_queue) || (copy_queue && compute_queue)))
                    return vk::ImageLayout::eGeneral;
                return vk::ImageLayout::eTransferSrcOptimal;
            }
            return vk::ImageLayout::eUndefined;
        }

        constexpr auto SelectCopyDstLayout(ImageFlags flags, b8 unified_image_layouts) noexcept {
            if (flags.Has(ImageFlagBit::eCopyDst)) {
                const auto general_queue = flags.Has(DnmRHI::ImageFlagBit::eGeneralQueue);
                const auto compute_queue = flags.Has(DnmRHI::ImageFlagBit::eComputeQueue);
                const auto copy_queue = flags.Has(DnmRHI::ImageFlagBit::eCopyQueue);

                const auto single_queue = (general_queue + compute_queue + copy_queue) < 2;

                if (flags.Has(ImageFlagBit::eStorage) || unified_image_layouts || (!single_queue && (copy_queue && general_queue) || (copy_queue && compute_queue)))
                    return vk::ImageLayout::eGeneral;
                return vk::ImageLayout::eTransferDstOptimal;
            }
            return vk::ImageLayout::eUndefined;
        }

        constexpr auto SelectVariableRateShadingLayout(ImageFlags flags, b8 unified_image_layouts) noexcept {
            if (flags.Has(ImageFlagBit::eVariableRateShadingAttachment)) {
                const auto general_queue = flags.Has(DnmRHI::ImageFlagBit::eGeneralQueue);
                const auto compute_queue = flags.Has(DnmRHI::ImageFlagBit::eComputeQueue);
                const auto copy_queue = flags.Has(DnmRHI::ImageFlagBit::eCopyQueue);

                const auto single_queue = (general_queue + compute_queue + copy_queue) < 2;
                
                if (flags.Has(ImageFlagBit::eStorage) || unified_image_layouts || (!single_queue && (copy_queue && general_queue) || (copy_queue && compute_queue)))
                    return vk::ImageLayout::eGeneral;
                return vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR;
            }
            return vk::ImageLayout::eUndefined;
        }
    }

    Image::Image(Vk::Device &device, const ImageDesc &desc) noexcept
    : DnmRHI::Image(device, desc), 
        c_IsSwapchainImage(false),
        c_DefaultLayout{Utility::SelectDefaultLayout(desc.flags, GetVkDeviceFeatures.unified_image_layouts)}, 
        c_AttachmentImageLayout{Utility::SelectAttachmentLayout(desc.flags, GetVkDeviceFeatures.unified_image_layouts)},
        c_ReadOnlyAttachmentImageLayout{GetVkDeviceFeatures.unified_image_layouts ? vk::ImageLayout::eGeneral : vk::ImageLayout::eReadOnlyOptimal},
        c_CopySrcImageLayout{Utility::SelectCopySrcLayout(desc.flags, GetVkDeviceFeatures.unified_image_layouts)},
        c_CopyDstImageLayout{Utility::SelectCopyDstLayout(desc.flags, GetVkDeviceFeatures.unified_image_layouts)},
        c_VariableRateShadingLayout{Utility::SelectVariableRateShadingLayout(desc.flags, GetVkDeviceFeatures.unified_image_layouts)}
        {
        VmaAllocationCreateInfo alloc_create_info{};
        alloc_create_info.usage = VMA_MEMORY_USAGE_AUTO;

        vk::ImageCreateFlags flags{};
        if ((GetDesc().type == DnmRHI::ImageType::e2D) && (GetDesc().extent.depth % 6) == 0) flags |= vk::ImageCreateFlagBits::eCubeCompatible;
        if (GetDesc().type == DnmRHI::ImageType::e3D) flags |= vk::ImageCreateFlagBits::e2DArrayCompatible;

        std::set<u32> queue_families_set;
        if (GetDesc().flags.Has(DnmRHI::ImageFlagBit::eGeneralQueue)) queue_families_set.emplace(GetVkDevice.GetGeneralQueueFamilyIndex());
        if (GetDesc().flags.Has(DnmRHI::ImageFlagBit::eComputeQueue)) queue_families_set.emplace(GetVkDevice.GetComputeQueueFamilyIndex());
        if (GetDesc().flags.Has(DnmRHI::ImageFlagBit::eCopyQueue)) queue_families_set.emplace(GetVkDevice.GetCopyQueueFamilyIndex());

        std::vector<u32> queue_families(queue_families_set.begin(), queue_families_set.end());

        const auto image_create_info
            = vk::ImageCreateInfo{}
                .setImageType(To_Vk(GetDesc().type))
                .setFormat(To_Vk(GetDesc().format))
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setExtent({GetDesc().extent.width, GetDesc().extent.height, GetDesc().type == DnmRHI::ImageType::e3D ? GetDesc().extent.depth : 1u})
                .setMipLevels(GetDesc().mipmap_count)
                .setArrayLayers(GetDesc().type != DnmRHI::ImageType::e3D ? GetDesc().extent.depth : 1)
                .setSamples(GetVkDevice.GetSamples(GetDesc().sample_count))
                .setTiling(vk::ImageTiling::eOptimal)
                .setUsage(To_Vk(GetDesc().flags, IsColorFormat(GetDesc().format)))
                .setSharingMode((queue_families_set.size() > 1) ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive)
                .setQueueFamilyIndices(queue_families)
                ;

        vmaCreateImage(GetVkDevice.GetAllocator(), TO_C_CONST_PTR(image_create_info), &alloc_create_info, TO_C_PTR(m_image), &m_allocation, nullptr);
        if (!desc.name.empty()) SET_DEBUG_NAME_VK_OBJECT(GetVkDevice.GetDevice(), m_image, GetDesc().name.data());
    }

    vk::ImageViewCreateInfo Image::GetCreateImageViewInfo(DnmRHI::ImageSubresource subresource) const noexcept {
        return vk::ImageViewCreateInfo()
                .setImage(m_image)
                .setViewType(To_Vk(subresource.type))
                .setFormat(To_Vk(GetDesc().format))
                .setSubresourceRange(vk::ImageSubresourceRange()
                    .setAspectMask(To_Vk(subresource.aspect_flags))
                    .setBaseMipLevel(subresource.base_layer)
                    .setLevelCount(subresource.mipmap_count)
                    .setBaseArrayLayer(subresource.layer_count)
                    .setLayerCount(subresource.base_mipmap));
    }

    Image::Image(Vk::Device &device, vk::Image image, ImageFlags flags, u32 width, u32 height, vk::Format format, Vk::Swapchain *swapchain, u8 swapchain_image_index, std::string_view name) noexcept
    :   m_swapchain(swapchain),
        c_SwapchainImageIndex(swapchain_image_index),
        DnmRHI::Image(device, 
            ImageDesc{}
                .SetFlags(flags)
                .SetFormat(Format::eUndefined)
                .SetName(name)
                .SetExtent({static_cast<u16>(width), static_cast<u16>(height), 1})
                .SetMipmapCount(1)
                .SetType(ImageType::e2D)
                .SetFormat(Utility::VkFormatToRHIImageFormat(format))
                .SetSampleCount(SampleCount::e1)
            ), 
        c_IsSwapchainImage(true),
        c_DefaultLayout{Utility::SelectDefaultLayout(flags, GetVkDeviceFeatures.unified_image_layouts)}, 
        c_AttachmentImageLayout{Utility::SelectAttachmentLayout(flags, GetVkDeviceFeatures.unified_image_layouts)},
        c_ReadOnlyAttachmentImageLayout{GetVkDeviceFeatures.unified_image_layouts ? vk::ImageLayout::eGeneral : vk::ImageLayout::eReadOnlyOptimal},
        c_CopySrcImageLayout{Utility::SelectCopySrcLayout(flags, GetVkDeviceFeatures.unified_image_layouts)},
        c_CopyDstImageLayout{Utility::SelectCopyDstLayout(flags, GetVkDeviceFeatures.unified_image_layouts)},
        c_VariableRateShadingLayout{Utility::SelectVariableRateShadingLayout(flags, GetVkDeviceFeatures.unified_image_layouts)},
        m_image(image),
        m_allocation(nullptr),
        c_SwapchainImageView(GetVkDevice.GetDevice().createImageView(
            vk::ImageViewCreateInfo{}
                .setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(format)
                .setSubresourceRange(
                    vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseArrayLayer(0)
                    .setBaseMipLevel(0)
                    .setLayerCount(1)
                    .setLevelCount(1)
            )
        )) {}
}

