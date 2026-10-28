#include <DnmRHI/Platform/Vulkan/ComputePipeline.hpp>
#include <DnmRHI/Platform/Vulkan/DescriptorManager.hpp>
#include <DnmRHI/Platform/Vulkan/Shader.hpp>

namespace DnmRHI::Vk {
    ComputePipeline::ComputePipeline(Vk::Device &device, const ComputePipelineDesc &desc) : DnmRHI::ComputePipeline(device, desc)  {
        const auto &vk_shader = static_cast<const Vk::Shader &>(*desc.compute_shader);
        const auto shader_stage_create_info 
            = vk::PipelineShaderStageCreateInfo{}
                .setStage(vk::ShaderStageFlagBits::eCompute)
                .setPName(GetVkDevice.GetVulkanSettings().generate_spirv_code_via_glsl ? "main" : vk_shader.GetEntryPointInfo().name.c_str())
                .setModule(vk_shader.GetModule())
                ;

        const auto create_flags_2
            = vk::PipelineCreateFlags2CreateInfo{}
                .setFlags(GetVkDeviceFeatures.descriptor_heap ? 
                    vk::PipelineCreateFlagBits2::eDescriptorBufferEXT 
                    : vk::PipelineCreateFlagBits2::eDescriptorHeapEXT)
                ;

        m_pipeline = GetVkDevice.GetDevice().createComputePipeline(
            {},
            vk::ComputePipelineCreateInfo{}
                .setLayout(GetVkDevice.GetDescriptorManager()->GetPipelineLayout())
                .setStage(shader_stage_create_info)
                .setPNext(&create_flags_2)
        ).value;

        if (!desc.name.empty()) SET_DEBUG_NAME_VK_OBJECT(GetVkDevice.GetDevice(), m_pipeline, desc.name.data());
    }
}
