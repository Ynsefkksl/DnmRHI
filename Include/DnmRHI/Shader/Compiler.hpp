#pragma once

#include <DnmRHI/Types.hpp>
#include <DnmRHI/Common/ShaderStages.hpp>
#include <DnmRHI/Shader/EntryPoint.hpp>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-com-helper.h>
#include <span>
#include <unordered_map>

namespace DnmRHI {
    struct ShaderFileData {
        ShaderFileData() = default;
        std::string name;
        ShaderStageFlags shader_stages;
        std::unordered_map<std::string, ShaderEntryPoint> entry_points;
    };

    struct ShaderData {
        std::vector<u8> shader_binary;
        ShaderStageFlags shader_stages;
        std::string name = "Unknown";
        u64 handle; // vk::ShaderModule, etc.
        std::unordered_map<ShaderStageFlagBit, std::span<const u8>> per_shader_stage_data; // for dx
    };

    class ShaderCompiler {
    private:
        ShaderCompiler(Device &device) noexcept;
    public:
        ~ShaderCompiler() noexcept;

        const ShaderEntryPoint &GetEntryPointBinary(std::string_view file, std::string_view entry_point) noexcept;
    private:
        ShaderFileData CompileFile(std::filesystem::path file_path);
        Slang::ComPtr<slang::IGlobalSession> m_global_session;
        Slang::ComPtr<slang::ISession> m_session;
        std::unordered_map<std::string, ShaderFileData> m_shader_file_data_cache{};

        Device &m_device;
        friend Device;
    };
}