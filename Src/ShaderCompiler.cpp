#include <DnmRHI/Utility/Counter.hpp>
#include <DnmRHI/Shader/Compiler.hpp>
#include <DnmRHI/Device/Device.hpp>
#include <filesystem>
#include <fstream>

namespace DnmRHI {
    static std::string ReadShaderFile(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) return {};
        std::ifstream file(path);

        if (!file.is_open()) return {};

        return { std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>() };
    }

    constexpr ShaderStageFlagBit ToSlangShaderStageFlagBits(SlangStage stage) noexcept {
        switch (stage) {
            case SLANG_STAGE_VERTEX:        return ShaderStageFlagBit::eVertex;
            case SLANG_STAGE_FRAGMENT:      return ShaderStageFlagBit::eFragment;
            case SLANG_STAGE_COMPUTE:       return ShaderStageFlagBit::eCompute;
            case SLANG_STAGE_MESH:          return ShaderStageFlagBit::eMesh;
            case SLANG_STAGE_AMPLIFICATION: return ShaderStageFlagBit::eTask;
            case SLANG_STAGE_ANY_HIT:       return ShaderStageFlagBit::eAnyHit;
            case SLANG_STAGE_CALLABLE:      return ShaderStageFlagBit::eCallable;
            case SLANG_STAGE_CLOSEST_HIT:   return ShaderStageFlagBit::eClosestHit;
            case SLANG_STAGE_INTERSECTION:  return ShaderStageFlagBit::eIntersection;
            case SLANG_STAGE_MISS:          return ShaderStageFlagBit::eMiss;
            case SLANG_STAGE_RAY_GENERATION:return ShaderStageFlagBit::eRaygen;
            default:                        return ShaderStageFlagBit{};
        }
    }

    ShaderCompiler::ShaderCompiler(Device &device) noexcept : m_device(device) {
        slang::createGlobalSession(m_global_session.writeRef());
        m_session = m_device.CreateCompilerSession(m_global_session);
    }

    ShaderCompiler::~ShaderCompiler() noexcept {}

    ShaderFileData ShaderCompiler::CompileFile(std::filesystem::path shader_file) {
        auto name = shader_file.filename().string();

        Slang::ComPtr<slang::IModule> slang_module;
        {
            const auto file= ReadShaderFile(shader_file);

            Slang::ComPtr<slang::IBlob> diagnostics_blob;
            slang_module = m_session->loadModuleFromSourceString(
                name.data(), 
                shader_file.string().c_str(), 
                file.c_str(), 
                diagnostics_blob.writeRef());
            if (!slang_module) return {}; // Critical("Failed to load module error: {}", 
                // std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
            if (diagnostics_blob) {
                // Error("{}", std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
            }
        }

        std::vector<slang::IComponentType*> component_types{};

        std::vector<Slang::ComPtr<slang::IEntryPoint>> entryPoints(slang_module->getDefinedEntryPointCount());
        for (const auto i : Counter(slang_module->getDefinedEntryPointCount())) {
            slang_module->getDefinedEntryPoint(i, entryPoints[i].writeRef());
            component_types.emplace_back(entryPoints[i]);
        }

        Slang::ComPtr<slang::IComponentType> composed_program;
        {
            Slang::ComPtr<slang::IBlob> diagnostics_blob;
            
            m_session->createCompositeComponentType(
                component_types.data(),
                component_types.size(),
                composed_program.writeRef(),
                diagnostics_blob.writeRef());

            if (!composed_program) return {};// Critical("Shader compose error: {}", 
                // std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
            if (diagnostics_blob) {
                return {}; // Error("{}", std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
            }
        }

        Slang::ComPtr<slang::IComponentType> linked_program;
        {
            Slang::ComPtr<slang::IBlob> diagnostics_blob;
            SlangResult result = composed_program->link(
                linked_program.writeRef(),
                diagnostics_blob.writeRef());

            if (!linked_program) return {}; // Critical("Linked program error: {}",
                // std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
            if (diagnostics_blob) {
                return {}; // Error("{}", std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
            }
        }


        ShaderFileData file_data;
        for (const auto i : Counter(linked_program->getLayout()->getEntryPointCount())) {
            const auto shader_stage = ToSlangShaderStageFlagBits(linked_program->getLayout()->getEntryPointByIndex(i)->getStage());
            file_data.shader_stages |= shader_stage;

            Slang::ComPtr<slang::IBlob> code_blob;
            linked_program->getEntryPointCode(i, 0, code_blob.writeRef());
            std::string name = linked_program->getLayout()->getEntryPointByIndex(i)->getName();
            file_data.entry_points[name] = ShaderEntryPoint{
                shader_file,
                name,
                shader_stage,
                {static_cast<const u8 *>(code_blob->getBufferPointer()), 
                    (static_cast<const u8 *>(code_blob->getBufferPointer()) + code_blob->getBufferSize())}
            };
        }

        return file_data;
    }

    const ShaderEntryPoint &ShaderCompiler::GetEntryPointBinary(std::string_view file, std::string_view entry_point) noexcept {
        std::string file_str(file);
        auto [it, is_emplaced] = m_shader_file_data_cache.try_emplace(file_str, ShaderFileData{});
        if (is_emplaced) {
            it->second = CompileFile(file);
        }
        const auto &file_data = it->second;
        auto entry_point_str = std::string(entry_point);
        if (file_data.entry_points.contains(entry_point_str)) {
            return file_data.entry_points.at(entry_point_str);
        }
        //! TODO: why there is unreachable
        std::unreachable();
    }
}