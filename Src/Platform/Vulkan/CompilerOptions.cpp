#include <DnmRHI/Platform/Vulkan/Device.hpp>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-com-helper.h>

namespace DnmRHI::Vk {
    // slang::ISession *Device::CreateCompilerSession(slang::IGlobalSession *global_session) noexcept {
    //     std::vector<slang::CompilerOptionEntry> options;
        
    //     options.emplace_back(slang::CompilerOptionEntry {
    //         slang::CompilerOptionName::Optimization,
    //         { slang::CompilerOptionValueKind::Int, 3 * !GetSettings().shader_debug }
    //     });
    //     options.emplace_back(slang::CompilerOptionEntry {
    //         slang::CompilerOptionName::EmitSpirvDirectly,
    //         { slang::CompilerOptionValueKind::Int, !m_vk_settings.generate_spirv_code_via_glsl }
    //     });
    //     options.emplace_back(slang::CompilerOptionEntry {
    //         slang::CompilerOptionName::EmitSpirvViaGLSL,
    //         { slang::CompilerOptionValueKind::Int, m_vk_settings.generate_spirv_code_via_glsl }
    //     });
    //     options.emplace_back(slang::CompilerOptionEntry {
    //         slang::CompilerOptionName::MatrixLayoutColumn,
    //         { slang::CompilerOptionValueKind::Int, 1 }
    //     });
    //     options.emplace_back(slang::CompilerOptionEntry {
    //         slang::CompilerOptionName::DebugInformation,
    //         { slang::CompilerOptionValueKind::Int, 
    //             static_cast<int32_t>(GetSettings().shader_debug ? SLANG_DEBUG_INFO_LEVEL_MAXIMAL : SLANG_DEBUG_INFO_LEVEL_NONE) }
    //     });
    //     options.emplace_back(slang::CompilerOptionEntry {
    //         slang::CompilerOptionName::Capability,
    //         { slang::CompilerOptionValueKind::Int, global_session->findCapability("vk_mem_model") }
    //     });
    //     options.emplace_back(slang::CompilerOptionEntry {
    //         slang::CompilerOptionName::Capability,
    //         { slang::CompilerOptionValueKind::Int, global_session->findCapability("spvVulkanMemoryModelKHR") }
    //     });
    //     options.emplace_back(slang::CompilerOptionEntry {
    //         slang::CompilerOptionName::Capability,
    //         { slang::CompilerOptionValueKind::Int, global_session->findCapability("spvVulkanMemoryModelDeviceScopeKHR") }
    //     });
    //     options.emplace_back(slang::CompilerOptionEntry {
    //         slang::CompilerOptionName::Capability,
    //         { slang::CompilerOptionValueKind::Int, global_session->findCapability("spvDemoteToHelperInvocation") }
    //     });
    //     if (GetSettings().shader_debug) {
    //         options.emplace_back(slang::CompilerOptionEntry {
    //             slang::CompilerOptionName::DebugInformation,
    //             { slang::CompilerOptionValueKind::Int, SLANG_DEBUG_INFO_LEVEL_MAXIMAL }
    //         });
    //         options.emplace_back(slang::CompilerOptionEntry {
    //             slang::CompilerOptionName::LineDirectiveMode,
    //             { slang::CompilerOptionValueKind::Int, SLANG_LINE_DIRECTIVE_MODE_NONE }
    //         });
    //     }
    //     options.emplace_back(slang::CompilerOptionEntry {
    //         slang::CompilerOptionName::GLSLForceScalarLayout,
    //         { slang::CompilerOptionValueKind::Int, 1 }
    //     });
    //     if (m_vk_device_features.descirptor_heap) {
    //         options.emplace_back(slang::CompilerOptionEntry {
    //             slang::CompilerOptionName::Capability,
    //             { slang::CompilerOptionValueKind::Int, global_session->findCapability("spvDescriptorHeapEXT") }
    //         });
    //     }
    //     if (m_vk_device_features.shader_stencil_export) {
    //         options.emplace_back(slang::CompilerOptionEntry {
    //             slang::CompilerOptionName::Capability,
    //             { slang::CompilerOptionValueKind::Int, global_session->findCapability("spvStencilExportEXT") }
    //         });
    //     }

    //     slang::TargetDesc target_desc{};
    //     target_desc.compilerOptionEntries = options.data();
    //     target_desc.compilerOptionEntryCount = options.size();
    //     target_desc.forceGLSLScalarBufferLayout = true;
    //     target_desc.format = SLANG_SPIRV;
    //     target_desc.profile = global_session->findProfile("sm_6_6");

    //     const auto str_subgroup_size = std::to_string(m_device_properties.subgroup_size);
    //     const slang::PreprocessorMacroDesc macro{"SUBGROUP_SIZE", str_subgroup_size.c_str()}; 

    //     slang::SessionDesc session_desc{};
    //     session_desc.preprocessorMacros = &macro; 
    //     session_desc.preprocessorMacroCount = 1;
    //     session_desc.targetCount = 1;
    //     session_desc.targets = &target_desc;

    //     slang::ISession *session;
    //     global_session->createSession(session_desc, &session);
    //     return session;
    // }
}