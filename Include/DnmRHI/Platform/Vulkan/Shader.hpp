#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Shader/Shader.hpp>

namespace DnmRHI::Vk {
    class Shader final : public DnmRHI::Shader {
    public:
        explicit Shader(Vk::Device &device, const ShaderEntryPoint &entry_point) noexcept : DnmRHI::Shader(device, entry_point) {}
        virtual ~Shader() override {}
    };
}