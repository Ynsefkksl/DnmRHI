#include <DnmRHI/Common/ShaderStages.hpp>
#include <span>
#include <string_view>

namespace DnmRHI {
    struct ShaderByteCodeHeader {
        alignas(4) u32 magic{};
        alignas(4) ShaderStageFlagBit stage{};
        alignas(4) u32 entry_point_name_length{};
        char entry_point_name[];

        std::string_view GetShaderEntryPointName() const noexcept {
            return std::string_view(entry_point_name);
        }
        std::span<const u8> GetShaderByteCode(std::span<const u8> owner_byte_code) const noexcept {
            const u32 total_header_size = sizeof(ShaderByteCodeHeader) + entry_point_name_length + 1 /* for null terminator*/;
            return std::span<const u8>(owner_byte_code.data() + total_header_size, owner_byte_code.size() - total_header_size);
        }
    };
}