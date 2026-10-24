#pragma once

#include <DnmRHI/Types.hpp>
#include <DnmRHI/Shader/Desc.hpp>

namespace DnmRHI {
    struct ShaderEntryPoint; 
    
    class Shader : public Object {
    protected:
        explicit Shader(Device &device, const ShaderEntryPoint &entry_point) noexcept : m_entry_point(entry_point), Object(device, ObjectType::eShader) {}
    public:
        [[nodiscard]] const auto &GetEntryPointInfo() const noexcept { return m_entry_point; }
    protected:
        const ShaderEntryPoint &m_entry_point;
    };
}