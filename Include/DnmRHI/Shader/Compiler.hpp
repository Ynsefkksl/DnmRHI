#pragma once

#include <DnmRHI/Types.hpp>
#include <DnmRHI/Common/ShaderStages.hpp>
#include <DnmRHI/Shader/EntryPoint.hpp>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-com-helper.h>
#include <unordered_map>

namespace DnmRHI {
    struct ShaderFileData {
        ShaderFileData() = default;
        std::string name;
        ShaderStageFlags shader_stages;
        std::unordered_map<std::string, ShaderEntryPoint> entry_points;
    };

    class ShaderCompiler {
    private:
        ShaderCompiler(Device &device) noexcept;
    public:
        ~ShaderCompiler() noexcept;

        ResultOr<const ShaderEntryPoint *> GetEntryPointBinary(std::string_view file, std::string_view entry_point) noexcept;
    private:
        ResultOr<ShaderFileData> CompileFile(std::filesystem::path file_path);
        Slang::ComPtr<slang::IGlobalSession> m_global_session;
        Slang::ComPtr<slang::ISession> m_session;
        std::unordered_map<std::string, ShaderFileData> m_shader_file_data_cache{};

        Device &m_device;
        friend Device;
    };
}