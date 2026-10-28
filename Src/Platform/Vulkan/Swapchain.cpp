#include <DnmRHI/Platform/Vulkan/Swapchain.hpp>
#include <DnmRHI/Utility/Counter.hpp>
#include <DnmRHI/Platform/Vulkan/Image.hpp>

namespace DnmRHI::Vk {
    namespace Utility {
        [[nodiscard]] constexpr vk::Format SwapchainFormatToBasicFormat(vk::Format format) noexcept {
            switch (format) {
                case vk::Format::eB8G8R8A8Unorm: return vk::Format::eR8G8B8A8Unorm;
                case vk::Format::eB8G8R8A8Srgb: return vk::Format::eR8G8B8A8Srgb;

                case vk::Format::eR8G8B8A8Srgb:
                case vk::Format::eR8G8B8A8Unorm: return format;
            default: std::unreachable();
            }
        }

        [[nodiscard]] constexpr DnmRHI::Format VkFormatToRHIImageFormat(vk::Format format) noexcept {
            switch (format) {
                case vk::Format::eB8G8R8A8Srgb: return DnmRHI::Format::eR8G8B8A8_Srgb;
                case vk::Format::eB8G8R8A8Unorm: return DnmRHI::Format::eR8G8B8A8_Unorm;

                case vk::Format::eR8G8B8A8Srgb: return DnmRHI::Format::eR8G8B8A8_Srgb;
                case vk::Format::eR8G8B8A8Unorm: return DnmRHI::Format::eR8G8B8A8_Unorm;
            default: std::unreachable();
            }
        }

        [[nodiscard]] static vk::SurfaceKHR CreateVkSurface(vk::Instance instance, const WindowInfo &window) noexcept {
            if (window.platform == WindowPlatform::eHeadless)
                return instance.createHeadlessSurfaceEXT(
                    vk::HeadlessSurfaceCreateInfoEXT{}
                );

            // TODO: Xlib
            switch (window.platform) {
#ifdef DnmRHI_Platform_Win32
                case DnmRHI::WindowPlatform::eWin32: return instance.createWin32SurfaceKHR(
                    vk::Win32SurfaceCreateInfoKHR{}
                        .setHwnd(reinterpret_cast<HWND>(window.win32_handle.hwnd))
                        .setHinstance(reinterpret_cast<HINSTANCE>(window.win32_handle.hinstance))
                );
#else
                case DnmRHI::WindowPlatform::eXcb: return instance.createXcbSurfaceKHR(
                    vk::XcbSurfaceCreateInfoKHR{}
                        .setWindow(static_cast<u32>(window.xcb_handle.window))
                        .setConnection(reinterpret_cast<xcb_connection_t *>(window.xcb_handle.connection))
                );
                case DnmRHI::WindowPlatform::eWayland: return instance.createWaylandSurfaceKHR(
                    vk::WaylandSurfaceCreateInfoKHR{}
                        .setSurface(reinterpret_cast<wl_surface *>(window.wayland_handle.surface))
                        .setDisplay(reinterpret_cast<wl_display *>(window.wayland_handle.display))
                );
#endif
            default: assert(DnmRHI::WindowPlatform::eXLib != window.platform && "i forget xcb, probably your problem it is pls write"); std::unreachable();
            }
        }
    }

    Swapchain::Swapchain(Vk::Device &device, const SwapchainDesc &desc) noexcept : DnmRHI::Swapchain(device, desc) {
        m_surface = Utility::CreateVkSurface(GetVkDevice.GetInstance(), desc.window);
        IReCreate(desc.settings);
    }

    Swapchain::~Swapchain() noexcept {
        GetVkDevice.GetDevice().waitIdle();
        
        for (auto *image : m_images)
            delete image;

        for (auto i : m_image_acquire_semaphores)
            GetVkDevice.GetDevice().destroy(i);

        for (auto i : m_image_available_semaphores)
            GetVkDevice.GetDevice().destroy(i);

        if (m_swapchain) GetVkDevice.GetDevice().destroy(m_swapchain);
        if (m_surface) GetVkDevice.GetInstance().destroy(m_surface);
    }

    void Swapchain::IReCreate(const SwapchainSettings &settings) noexcept {
        m_desc.settings = settings;

		const auto get_desired = [] (auto &&desired_list, auto &&search_list, auto not_find_value) noexcept -> auto {
				for (const auto &desired_value : desired_list)
					for (const auto &value : search_list)
						if (desired_value == value)
							return value;
				return not_find_value;
			};

		const auto get_desired_func = [](auto &&desired_list, auto &&search_list, auto not_find_value, auto &&search_list_func) -> auto {
                for (const auto &desired_value : desired_list)
                    for (const auto &value : search_list) {
                        const auto value_other = search_list_func(value);
                        if (desired_value == value_other)
                            return value_other;
                    }
                return not_find_value;
            };

        vk::PresentModeKHR present_mode;

        {
            const auto requested = To_Vk(settings.vsync);
            const auto supported_present_modes = GetVkDevice.GetPhysicalDevice().getSurfacePresentModesKHR(m_surface);

            // supported_present_modes.
            if (requested == vk::PresentModeKHR::eFifo) {
                present_mode = requested;
            }
            else if (requested == vk::PresentModeKHR::eMailbox) {
                const auto desired_present_modes_vsync = {
                    vk::PresentModeKHR::eMailbox,
                    vk::PresentModeKHR::eFifoLatestReady,
                    vk::PresentModeKHR::eFifoRelaxed,
                    vk::PresentModeKHR::eFifo,
                };

                present_mode = get_desired(
                    desired_present_modes_vsync, 
                    supported_present_modes, 
                    vk::PresentModeKHR::eFifo);
            }
            else if (requested == vk::PresentModeKHR::eFifoLatestReady) {
                const auto desired_present_modes_vsync = {
                    vk::PresentModeKHR::eFifoLatestReady,
                    vk::PresentModeKHR::eMailbox,
                    vk::PresentModeKHR::eFifoRelaxed,
                    vk::PresentModeKHR::eFifo,
                };

                present_mode = get_desired(
                    desired_present_modes_vsync, 
                    supported_present_modes, 
                    vk::PresentModeKHR::eFifo);
            }
            else if (requested == vk::PresentModeKHR::eFifoRelaxed) {
                const auto desired_present_modes_vsync = {
                    vk::PresentModeKHR::eFifoRelaxed,
                    vk::PresentModeKHR::eFifoLatestReady,
                    vk::PresentModeKHR::eMailbox,
                    vk::PresentModeKHR::eFifo,
                };

                present_mode = get_desired(
                    desired_present_modes_vsync, 
                    supported_present_modes, 
                    vk::PresentModeKHR::eFifo);
            }
            else if (requested == vk::PresentModeKHR::eImmediate) {
                const auto desired_present_modes_vsync = {
                    vk::PresentModeKHR::eImmediate,
                    vk::PresentModeKHR::eMailbox,
                    vk::PresentModeKHR::eFifoLatestReady,
                    vk::PresentModeKHR::eFifoRelaxed,
                    vk::PresentModeKHR::eFifo,
                };

                present_mode = get_desired(
                    desired_present_modes_vsync, 
                    supported_present_modes, 
                    vk::PresentModeKHR::eFifo);
            }
        }

        vk::SurfaceTransformFlagBitsKHR current_transform;

        {
            const auto surface_capabilities = GetVkDevice.GetPhysicalDevice().getSurfaceCapabilitiesKHR(m_surface);

            m_image_count = settings.triple_buffering || (present_mode == vk::PresentModeKHR::eMailbox) ? 3u : 2u;

            m_image_count = std::clamp(m_image_count, 
                surface_capabilities.minImageCount, 
                surface_capabilities.maxImageCount ? surface_capabilities.maxImageCount : UINT32_MAX);

            current_transform = surface_capabilities.currentTransform;

            m_width = surface_capabilities.currentExtent.width;
            m_height = surface_capabilities.currentExtent.height;
        }

        vk::ColorSpaceKHR color_space;

        {
            const auto surface_formats = GetVkDevice.GetPhysicalDevice().getSurfaceFormatsKHR(m_surface);
            
            const auto desired_surface_format_linear = {
                vk::SurfaceFormatKHR{vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear},
                vk::SurfaceFormatKHR{vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear},
            };

            const auto desired_surface_format_srgb = {
                vk::SurfaceFormatKHR{vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
                vk::SurfaceFormatKHR{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
            };

            const auto surface_format = get_desired(
                surface_formats, 
                settings.color_space == ColorSpace::eLinear ? desired_surface_format_linear : desired_surface_format_srgb, 
                vk::SurfaceFormatKHR{vk::Format::eUndefined}
                );

            color_space = surface_format.colorSpace;
            m_format = surface_format.format;
        }

        const auto create_info
            = vk::SwapchainCreateInfoKHR{}
                .setMinImageCount(m_image_count)
                .setImageFormat(m_format)
                .setImageColorSpace(color_space)
                .setImageExtent({m_width, m_height})
                .setImageArrayLayers(1)
                .setImageUsage(To_Vk(settings.image_flags, true))
                .setImageSharingMode(vk::SharingMode::eExclusive)
                .setPreTransform(current_transform)
                .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                .setPresentMode(present_mode)
                .setClipped(true)
                .setSurface(m_surface)
                .setOldSwapchain(m_swapchain)
                ;

        m_swapchain = GetVkDevice.GetDevice().createSwapchainKHR(create_info);
        if (!m_desc.name.empty()) SET_DEBUG_NAME_VK_OBJECT(GetVkDevice.GetDevice(), m_swapchain, m_desc.name.c_str());

        m_swapchain_images = GetVkDevice.GetDevice().getSwapchainImagesKHR(m_swapchain);

        m_image_count = m_swapchain_images.size();

        m_images.reserve(m_image_count);

        for (const auto i : Counter(m_image_count)) {
            const auto semaphore = m_image_available_semaphores.emplace_back(
                GetVkDevice.GetDevice().createSemaphore({})
            );

            if (!m_desc.name.empty()) SET_DEBUG_NAME_VK_OBJECT(GetVkDevice.GetDevice(), semaphore, std::format("{} swapchain, {}. image available semaphore", m_desc.name, i).c_str());

            m_images.emplace_back(new Vk::Image(
                GetVkDevice,
                m_swapchain_images[i],
                settings.image_flags,
                m_width, m_height,
                m_format,
                this,
                i,
                m_desc.name.empty() ? "" : std::format("{} {}. image", m_desc.name, i)
            ));
        }

        for (const auto i : Counter(c_Frames_In_Flight_Count)) {
            const auto semaphore = m_image_acquire_semaphores.emplace_back(
                GetVkDevice.GetDevice().createSemaphore({})
            );

            if (!m_desc.name.empty()) SET_DEBUG_NAME_VK_OBJECT(GetVkDevice.GetDevice(), semaphore, std::format("{} swapchain, {}. image acquire semaphore", m_desc.name, i).c_str());
        }

        // translate swapchain image layouts to present src, my image layout manager need it
        {
            const vk::Device device = GetVkDevice.GetDevice();
            const auto queue        = GetVkDevice.GetGeneralQueue();
            const auto image_count  = m_swapchain_images.size();

            const auto acquire_semaphore = device.createSemaphore({});

            std::vector<vk::Semaphore> present_semaphores;
            present_semaphores.reserve(image_count);
            for (size_t i = 0; i < image_count; ++i) {
                present_semaphores.push_back(device.createSemaphore({}));
            }

            const auto cmd_pool = device.createCommandPool(
                vk::CommandPoolCreateInfo{}
                    .setQueueFamilyIndex(GetVkDevice.GetGeneralQueueFamilyIndex()));

            const auto cmd_buffer = device.allocateCommandBuffers(
                vk::CommandBufferAllocateInfo{}
                    .setCommandBufferCount(1)
                    .setCommandPool(cmd_pool)
                    .setLevel(vk::CommandBufferLevel::ePrimary))[0];

            std::vector<bool> transitioned(image_count, false);
            size_t remaining = image_count;

            while (remaining > 0) {
                device.waitIdle();

                const auto acquire_result = device.acquireNextImageKHR(
                    m_swapchain, UINT64_MAX, acquire_semaphore, nullptr);

                if (acquire_result.result != vk::Result::eSuccess &&
                    acquire_result.result != vk::Result::eSuboptimalKHR) {
                    break;
                }

                const uint32_t image_index = acquire_result.value;
                const auto image = m_swapchain_images[image_index];

                device.resetCommandPool(cmd_pool);
                cmd_buffer.begin(
                    vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

                const auto barrier
                    = vk::ImageMemoryBarrier{}
                        .setImage(image)
                        .setSrcAccessMask({})
                        .setDstAccessMask({})
                        .setOldLayout(vk::ImageLayout::eUndefined)
                        .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
                        .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                        .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                        .setSubresourceRange(
                            vk::ImageSubresourceRange{}
                                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                .setBaseArrayLayer(0)
                                .setBaseMipLevel(0)
                                .setLayerCount(1)
                                .setLevelCount(1));

                cmd_buffer.pipelineBarrier(
                    vk::PipelineStageFlagBits::eAllCommands,
                    vk::PipelineStageFlagBits::eBottomOfPipe,
                    {}, {}, {}, barrier);
                cmd_buffer.end();

                constexpr vk::PipelineStageFlags wait_stage{vk::PipelineStageFlagBits::eAllCommands};
                const auto submit_info
                    = vk::SubmitInfo{}
                        .setCommandBuffers(cmd_buffer)
                        .setWaitDstStageMask(wait_stage)
                        .setWaitSemaphores(acquire_semaphore)
                        .setSignalSemaphores(present_semaphores[image_index]);
                queue.submit(submit_info);

                const auto present_info
                    = vk::PresentInfoKHR{}
                        .setWaitSemaphores(present_semaphores[image_index])
                        .setSwapchains(m_swapchain)
                        .setImageIndices(image_index);
                std::ignore = queue.presentKHR(present_info);

                if (!transitioned[image_index]) {
                    transitioned[image_index] = true;
                    --remaining;
                }
            }

            device.waitIdle();
            device.destroy(cmd_pool);
            device.destroy(acquire_semaphore);
            for (const auto semaphore : present_semaphores) {
                device.destroy(semaphore);
            }
        }
    }

    u32 Swapchain::IAcquireNextImage() noexcept {
        const auto vk_image_index = 
            GetVkDevice.GetDevice().acquireNextImageKHR(m_swapchain, UINT64_MAX, 
                m_image_acquire_semaphores[m_frame_index], nullptr).value;

        m_vk_image_index = vk_image_index;
        return m_vk_image_index;
    }
}