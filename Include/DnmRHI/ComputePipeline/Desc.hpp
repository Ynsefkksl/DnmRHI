#pragma once

#include <DnmRHI/Types.hpp>
#include <span>
#include <string>

namespace DnmRHI {
    struct ComputePipelineDesc { 
        std::string name;
        std::span<const u8> shader_byte_code;
        
        constexpr auto &SetName(std::string_view name) noexcept { this->name = name; return *this; }
        constexpr auto &SetShaderByteCode(std::span<const u8> shader_byte_code) noexcept { this->shader_byte_code = shader_byte_code; return *this; }
    };
}