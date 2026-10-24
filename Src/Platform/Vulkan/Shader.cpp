#include <DnmRHI/Platform/Vulkan/Shader.hpp>

namespace DnmRHI::Vk {
    Shader::Shader(Vk::Device &device, const ShaderEntryPoint &entry_point) noexcept : DnmRHI::Shader(device, entry_point) {
        m_module = GetVkDevice.GetDevice().createShaderModule(
            vk::ShaderModuleCreateInfo{}
                .setPCode(reinterpret_cast<const u32 *>(m_entry_point.byte_codes.data()))
                .setCodeSize(entry_point.byte_codes.size())
        );  
    }
}