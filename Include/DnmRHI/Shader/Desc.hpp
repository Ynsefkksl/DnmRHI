#pragma once

#include <string_view>

namespace DnmRHI {
    struct ShaderDesc {
        std::string_view file;
        std::string_view entry_point;

        constexpr auto &SetFile(std::string_view file) noexcept { this->file = file; return *this; }
        constexpr auto &SetEntryPoint(std::string_view entry_point) noexcept { this->entry_point = entry_point; return *this; }
    };
}
