#include <DnmRHI/Platform/Vulkan/ImageView.hpp>
#include <DnmRHI/Platform/Vulkan/Image.hpp>

namespace DnmRHI::Vk {
    ImageView::ImageView(Vk::Device &device, const DnmRHI::ImageViewDesc &desc) noexcept : DnmRHI::ImageView(device, desc)  {
        m_create_info.setImage(static_cast<Vk::Image *>(desc.image)->GetImage())
                    .setViewType(To_Vk(desc.type))
                    .setFormat(To_Vk(desc.image->GetDesc().format))
                    .setSubresourceRange(
                        vk::ImageSubresourceRange{}
                            .setAspectMask(To_Vk(desc.subresource.aspect_flags))
                            .setBaseMipLevel(desc.subresource.base_mipmap)
                            .setLevelCount(desc.subresource.mipmap_count)
                            .setBaseArrayLayer(desc.subresource.base_layer)
                            .setLayerCount(desc.subresource.layer_count)
                    );

        if (!GetVkDevice.GetVulkanDeviceFeatures().descriptor_heap 
            && desc.image->GetDesc().flags.Has(DnmRHI::ImageFlagBit::eAttachment) 
            && desc.image->GetDesc().flags.Has(DnmRHI::ImageFlagBit::eVariableRateShadingAttachment)) {
            m_image_view = GetVkDevice.GetDevice().createImageView(m_create_info, GetVkDevice.GetAllocationCallbacks());
        }
    }
}
