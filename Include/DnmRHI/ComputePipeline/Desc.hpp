#pragma once

#include <DnmRHI/Types.hpp>
#include <string>

namespace DnmRHI {
    struct ComputePipelineDesc { 
        std::string name;
        Shader *compute_shader;
        
        constexpr auto &SetName(std::string_view name) noexcept { this->name = name; return *this; }
        constexpr auto &SetComputeShader(Shader *shader) noexcept { this->compute_shader = shader; return *this; }
    };
}