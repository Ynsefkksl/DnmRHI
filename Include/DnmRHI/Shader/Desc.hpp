#pragma once

#include <DnmRHI/Types.hpp>
#include <span>
#include <string_view>

namespace DnmRHI {
    struct ShaderDesc {
        std::span<const u8> byte_code;
        std::span<const std::string_view> defines;
        std::string_view entry_point;
        std::string_view module_name;

        constexpr auto &SetByteCode(std::span<const u8> byte_code) noexcept { this->byte_code = byte_code; return *this; }
        constexpr auto &SetModuleName(std::string_view module_name) noexcept { this->module_name = module_name; return *this; }
        constexpr auto &SetEntryPoint(std::string_view entry_point) noexcept { this->entry_point = entry_point; return *this; }
        constexpr auto &SetDefines(std::span<const std::string_view> defines) noexcept { this->defines = defines; return *this; }
    };
}
