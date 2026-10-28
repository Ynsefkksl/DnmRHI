#include <DnmRHI/Platform/Vulkan/GraphicsPipeline.hpp>
#include <DnmRHI/Platform/Vulkan/DescriptorManager.hpp>
#include <DnmRHI/Platform/Vulkan/Shader.hpp>
#include <DnmRHI/Platform/Vulkan/ToVk.hpp>

namespace DnmRHI::Vk {
    GraphicsPipeline::GraphicsPipeline(Vk::Device &device, const GraphicsPipelineDesc &desc) : DnmRHI::GraphicsPipeline(device, desc) {
        std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
        shader_stages.reserve(desc.shaders.size());

        for (const auto *shader : desc.shaders) {
            const auto &vk_shader = static_cast<const Vk::Shader &>(*shader);
            shader_stages.emplace_back(
                vk::PipelineShaderStageCreateFlags{},
                To_Vk(vk_shader.GetEntryPointInfo().flag_bit), 
                vk_shader.GetModule(), 
                GetVkDevice.GetVulkanSettings().generate_spirv_code_via_glsl ? "main" : vk_shader.GetEntryPointInfo().name.c_str(), 
                nullptr, 
                nullptr);
        }

        std::vector<vk::PipelineColorBlendAttachmentState> attachments;
        for (const auto &attachment : desc.color_output_state.color_attachment_formats)
            attachments.push_back(
                vk::PipelineColorBlendAttachmentState{}
                    .setBlendEnable(attachment.blend_enable)
                    .setDstColorBlendFactor(To_Vk(attachment.dst_color_blend_factor))
                    .setSrcColorBlendFactor(To_Vk(attachment.src_color_blend_factor))
                    .setDstAlphaBlendFactor(To_Vk(attachment.dst_alpha_blend_factor))
                    .setSrcAlphaBlendFactor(To_Vk(attachment.src_alpha_blend_factor))
                    .setColorBlendOp(To_Vk(attachment.color_blend_op))
                    .setAlphaBlendOp(To_Vk(attachment.alpha_blend_op))
                    .setColorWriteMask(To_Vk(attachment.color_write_mask))
            );

        const auto color_blend_state_create_info
            = vk::PipelineColorBlendStateCreateInfo{}
                .setAttachments(attachments)
                ;

        const auto depth_stencil_state_create_info
            = vk::PipelineDepthStencilStateCreateInfo{}
                .setDepthTestEnable(desc.depth_stencil_state.depth_test_enable)
                .setDepthWriteEnable(desc.depth_stencil_state.depth_write_enable)
                .setDepthCompareOp(To_Vk(desc.depth_stencil_state.depth_test_compare_op))
                .setStencilTestEnable(desc.depth_stencil_state.stencil_test_enable)
                .setFront(vk::StencilOpState{}
                    .setFailOp(To_Vk(desc.depth_stencil_state.stencil_front_op.fail_op))
                    .setPassOp(To_Vk(desc.depth_stencil_state.stencil_front_op.pass_op))
                    .setCompareOp(To_Vk(desc.depth_stencil_state.stencil_front_op.compare_op))
                    .setDepthFailOp(To_Vk(desc.depth_stencil_state.stencil_front_op.depth_fail_op))
                    .setCompareMask(desc.depth_stencil_state.stencil_front_op.compare_mask)
                    .setWriteMask(desc.depth_stencil_state.stencil_front_op.write_mask)
                )
                .setBack(vk::StencilOpState{}
                    .setFailOp(To_Vk(desc.depth_stencil_state.stencil_back_op.fail_op))
                    .setPassOp(To_Vk(desc.depth_stencil_state.stencil_back_op.pass_op))
                    .setCompareOp(To_Vk(desc.depth_stencil_state.stencil_back_op.compare_op))
                    .setDepthFailOp(To_Vk(desc.depth_stencil_state.stencil_back_op.depth_fail_op))
                    .setCompareMask(desc.depth_stencil_state.stencil_back_op.compare_mask)
                    .setWriteMask(desc.depth_stencil_state.stencil_back_op.write_mask)
                );

        std::vector<vk::DynamicState> dynamic_states {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        if (desc.depth_stencil_state.stencil_test_enable) 
            dynamic_states.push_back(vk::DynamicState::eStencilReference);

        const auto dynamic_state_create_info
            = vk::PipelineDynamicStateCreateInfo{}
                .setDynamicStates(dynamic_states);

        const auto input_assembly_state_create_info
            = vk::PipelineInputAssemblyStateCreateInfo{}
                .setTopology(To_Vk(desc.rasterization_state.primitive_topology))
                ;

        const auto vertex_input_state_create_info
            = vk::PipelineVertexInputStateCreateInfo{}
                ;

        const auto rasterization_state_create_info
            = vk::PipelineRasterizationStateCreateInfo{}
                .setCullMode(To_Vk(desc.rasterization_state.cull_mode))
                .setPolygonMode(To_Vk(desc.rasterization_state.polygon_mode))
                .setLineWidth(1.f)
                .setFrontFace(To_Vk(desc.rasterization_state.front_face))
                ;
                
        const auto viewport_state_create_info
            = vk::PipelineViewportStateCreateInfo{}
                .setViewportCount(desc.rasterization_state.viewport_count)
                .setScissorCount(desc.rasterization_state.scissor_count)
                ;

        const auto multisample_state_create_info
            = vk::PipelineMultisampleStateCreateInfo{}
                .setRasterizationSamples(GetVkDevice.GetSamples(desc.rasterization_state.sample_count))
                .setSampleShadingEnable(desc.rasterization_state.sample_shading_enable)
                ;

        const auto create_flags_2
            = vk::PipelineCreateFlags2CreateInfo{}
                .setFlags(vk::PipelineCreateFlagBits2::eDescriptorHeapEXT)
                ;

        std::vector<vk::Format> attachments_formats;
        for (const auto &attachment : desc.color_output_state.color_attachment_formats)
            attachments_formats.push_back(To_Vk(attachment.format));

        const auto rendering_info
            = vk::PipelineRenderingCreateInfo{}
                .setColorAttachmentFormats(attachments_formats)
                .setDepthAttachmentFormat(To_Vk(desc.depth_stencil_state.depth_attachment))
                .setStencilAttachmentFormat(To_Vk(desc.depth_stencil_state.stencil_attachment))
                .setPNext(GetVkDeviceFeatures.descriptor_heap ? &create_flags_2 : nullptr)
                ;

        m_pipeline = GetVkDevice.GetDevice().createGraphicsPipeline(
                {},
                vk::GraphicsPipelineCreateInfo{}
                    .setLayout(GetVkDevice.GetDescriptorManager()->GetPipelineLayout())
                    .setStages(shader_stages)
                    .setPColorBlendState(&color_blend_state_create_info)
                    .setPDepthStencilState(&depth_stencil_state_create_info)
                    .setPDynamicState(&dynamic_state_create_info)
                    .setPInputAssemblyState(&input_assembly_state_create_info)
                    .setPVertexInputState(&vertex_input_state_create_info)
                    .setPRasterizationState(&rasterization_state_create_info)
                    .setPViewportState(&viewport_state_create_info)
                    .setPMultisampleState(&multisample_state_create_info)
                    .setPNext(&rendering_info)
            ).value;

        if (!desc.name.empty()) SET_DEBUG_NAME_VK_OBJECT(GetVkDevice.GetDevice(), m_pipeline, desc.name.data());
    }
}