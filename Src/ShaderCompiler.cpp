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

    ResultOr<ShaderFileData> ShaderCompiler::CompileFile(std::filesystem::path shader_file) {
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
            if (diagnostics_blob) {
                // TODO: fix massage ids
                m_device.Message(!slang_module ? MessageSeverity::eError : MessageSeverity::eWarning, MessageCatagory::eShaderCompiler, 
                    MessageID{0}, std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
                if (!slang_module) {
                    return std::unexpected(Result::eCompileError);
                }
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

            if (diagnostics_blob) {
                // TODO: fix massage ids
                m_device.Message(!composed_program ? MessageSeverity::eError : MessageSeverity::eWarning, MessageCatagory::eShaderCompiler, 
                    MessageID{0}, std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
                if (!composed_program) {
                    return std::unexpected(Result::eCompileError);
                }
            }
        }

        Slang::ComPtr<slang::IComponentType> linked_program;
        {
            Slang::ComPtr<slang::IBlob> diagnostics_blob;
            SlangResult result = composed_program->link(
                linked_program.writeRef(),
                diagnostics_blob.writeRef());

            if (diagnostics_blob) {
                // TODO: fix massage ids
                m_device.Message(!linked_program ? MessageSeverity::eError : MessageSeverity::eWarning, MessageCatagory::eShaderCompiler, 
                    MessageID{0}, std::string_view{static_cast<const char *>(diagnostics_blob->getBufferPointer()), diagnostics_blob->getBufferSize()});
                if (!linked_program) {
                    return std::unexpected(Result::eCompileError);
                }
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

    ResultOr<const ShaderEntryPoint *> ShaderCompiler::GetEntryPointBinary(std::string_view file, std::string_view entry_point) noexcept {
        if (std::filesystem::exists(file))
            return std::unexpected(Result::eFileNotFound);

        const std::string file_str(file);

        auto [it, is_emplaced] = m_shader_file_data_cache.try_emplace(file_str, ShaderFileData{});
        if (is_emplaced) {
            auto result_or_data = CompileFile(file);
            if (!result_or_data.has_value())
                return std::unexpected(result_or_data.error());
            it->second = result_or_data.value();
        }
        
        const auto &file_data = it->second;

        const auto entry_point_str = std::string(entry_point);
        if (file_data.entry_points.contains(entry_point_str)) {
            return &file_data.entry_points.at(entry_point_str);
        }
        return std::unexpected(Result::eEntryPointNotFound);
    }
}