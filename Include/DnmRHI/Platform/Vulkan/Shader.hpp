#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/Shader/Shader.hpp>

namespace DnmRHI::Vk {
    class Shader final : public DnmRHI::Shader {
    public:
        explicit Shader(Vk::Device &device, const ShaderEntryPoint &entry_point) noexcept;
        virtual ~Shader() override { 
            GetVkDevice.GetDevice().waitIdle();
            GetVkDevice.GetDevice().destroy(m_module);
        }

        [[nodiscard]] auto GetModule() const noexcept { return m_module; }
    private:
        vk::ShaderModule m_module;
    };
}