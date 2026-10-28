#pragma once

#include <DnmRHI/Types.hpp>
#include <DnmRHI/Image/Flags.hpp>
#include <string>

namespace DnmRHI {
    enum class WindowPlatform : u8 {
        eHeadless,
        eWin32,
        eWayland,
        eXcb,
        eXLib,
    };

    enum class ColorSpace : u8 {
        eLinear,
        eSrgb,
        //TODO: hdr things
    };

    enum class VSync : u8 {
        eImmediate,
        eFifo,
        eMailbox, 
        eFifoRelexed,
        eFifoLastestReady
    };
    
    // TODO: write consturcter and helper funcs
    struct WindowInfo {
        WindowPlatform platform;
        union {
            struct Win32 {
                void *hinstance;
                void *hwnd;
            } win32_handle;
            struct XLib {
                void *dpy;
                u64 window;
            } xlib_handle;
            struct Xcb {
                void *connection;
                u32 window;
            } xcb_handle;
            struct Wayland {
                void *display;
                void *surface;
            } wayland_handle;
            struct Headless {} headless;
        };

        constexpr auto &SetWindowPlatform(WindowPlatform platform) noexcept { this->platform = platform; return *this; }
        constexpr auto &SetHeadless() noexcept { 
            platform = WindowPlatform::eHeadless;
            this->headless = {}; return *this;
        }
        constexpr auto &SetWin32(void *hinstance, void *hwnd) noexcept {
            platform = WindowPlatform::eWin32;
            this->win32_handle = {hinstance, hwnd}; return *this;
        }
        constexpr auto &SetXLib(void *dpy, u64 window) noexcept { 
            platform = WindowPlatform::eXLib;
            this->xlib_handle = {dpy, window}; return *this; 
        }
        constexpr auto &SetXcb(void *connection, u32 window) noexcept { 
            platform = WindowPlatform::eXcb;
            this->xcb_handle = {connection, window}; return *this; 
        }
        constexpr auto &SetWayland(void *display, void *surface) noexcept { 
            platform = WindowPlatform::eWayland;
            this->wayland_handle = {display, surface}; return *this; 
        }
    };

    struct SwapchainSettings {
        VSync vsync;
        ColorSpace color_space;
        ImageFlags image_flags;
        b8 triple_buffering; // false is double buffering, some drivers requires triple buffers in this situation false value override as triple buffers
    
        constexpr auto &SetVSync(VSync vsync) noexcept { this->vsync = vsync; return *this; }
        constexpr auto &SetColorSpace(ColorSpace color_space) noexcept { this->color_space = color_space; return *this; }
        constexpr auto &SetImageFlags(ImageFlags image_flags) noexcept { this->image_flags = image_flags; return *this; }
        constexpr auto &SetTripleBuffer(b8 triple_buffering) noexcept { this->triple_buffering = triple_buffering; return *this; }
    };

    struct SwapchainDesc {
        std::string name{};
        WindowInfo window;
        SwapchainSettings settings;

        constexpr auto &SetName(std::string_view name) noexcept { this->name = name; return *this; }
        constexpr auto &SetWindow(const WindowInfo &window) noexcept { this->window = window; return *this; }
        constexpr auto &SetSettings(SwapchainSettings settings) noexcept { this->settings = settings; return *this; }
    };
}