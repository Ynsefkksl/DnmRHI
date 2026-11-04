#include <DnmRHI/Platform/Vulkan/ComputePipeline.hpp>
#include <DnmRHI/Platform/Vulkan/DescriptorManager.hpp>
#include <DnmRHI/Platform/Vulkan/Shader.hpp>

namespace DnmRHI::Vk {
    ComputePipeline::ComputePipeline(Vk::Device &device, const ComputePipelineDesc &desc) : DnmRHI::ComputePipeline(device, desc)  {
        const auto shader_module
            = vk::ShaderModuleCreateInfo{}
                .setPCode(reinterpret_cast<const u32 *>(desc.compute_shader->GetEntryPointInfo().byte_codes.data()))
                .setCodeSize(desc.compute_shader->GetEntryPointInfo().byte_codes.size())
                ;

        const auto shader_stage_create_info 
            = vk::PipelineShaderStageCreateInfo{}
                .setStage(vk::ShaderStageFlagBits::eCompute)
                .setPName(GetVkDevice.GetVulkanSettings().generate_spirv_code_via_glsl ? "main" : desc.compute_shader->GetEntryPointInfo().name.c_str())
                .setPNext(&shader_module)
                ;

        const auto create_flags_2
            = vk::PipelineCreateFlags2CreateInfo{}
                .setFlags(GetVkDeviceFeatures.descriptor_heap ? 
                    vk::PipelineCreateFlagBits2::eDescriptorHeapEXT
                    : vk::PipelineCreateFlagBits2::eDescriptorBufferEXT)
                ;

        m_pipeline = GetVkDevice.GetDevice().createComputePipeline(
            {},
            vk::ComputePipelineCreateInfo{}
                .setLayout(GetVkDevice.GetDescriptorManager()->GetPipelineLayout())
                .setStage(shader_stage_create_info)
                .setPNext(&create_flags_2),
            GetVkDevice.GetAllocationCallbacks()
        ).value;

        if (!desc.name.empty()) SET_DEBUG_NAME_VK_OBJECT(GetVkDevice.GetDevice(), m_pipeline, desc.name.data());
    }
}
