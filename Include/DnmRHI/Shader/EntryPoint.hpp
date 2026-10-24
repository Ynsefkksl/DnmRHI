#pragma once

#include <DnmRHI/Common/ShaderStages.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace DnmRHI {
    struct ShaderEntryPoint {
        std::filesystem::path file;
        std::string name;
        ShaderStageFlagBit flag_bit;
        std::vector<u8> byte_codes;
    };
}