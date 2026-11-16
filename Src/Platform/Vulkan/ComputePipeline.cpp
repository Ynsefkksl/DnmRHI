#include <DnmRHI/Platform/Vulkan/ComputePipeline.hpp>
#include <DnmRHI/Platform/Vulkan/DescriptorManager.hpp>
#include <DnmRHI/Shader/ByteCodeHeader.hpp>

namespace DnmRHI::Vk {
    ComputePipeline::ComputePipeline(Vk::Device &device, const ComputePipelineDesc &desc) : DnmRHI::ComputePipeline(device, desc)  {
        const auto *shader_header = reinterpret_cast<const DnmRHI::ShaderByteCodeHeader *>(desc.shader_byte_code.data());
        const auto shader_byte_code = shader_header->GetShaderByteCode(desc.shader_byte_code);

        const auto shader_module
            = vk::ShaderModuleCreateInfo{}
                .setPCode(reinterpret_cast<const u32 *>(shader_byte_code.data()))
                .setCodeSize(shader_byte_code.size())
                ;

        const auto shader_stage_create_info 
            = vk::PipelineShaderStageCreateInfo{}
                .setStage(vk::ShaderStageFlagBits::eCompute)
                .setPName(shader_header->GetShaderEntryPointName().data())
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
