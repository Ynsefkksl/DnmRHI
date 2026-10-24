#pragma once

#include <DnmRHI/Buffer/Flags.hpp>
#include <DnmRHI/Buffer/HostMemoryAccess.hpp>
#include <DnmRHI/Buffer/MemoryType.hpp>
#include <string>

namespace DnmRHI {
    struct BufferDesc {
        std::string name;
        u32 size;
        u16 align;
        BufferFlags flags;
        HostMemoryAccess host_memory_usage;
        MemoryType memory_type;
        
        constexpr auto &SetName(std::string_view name) noexcept { this->name = name; return *this; }
        constexpr auto &SetSize(u32 size) noexcept { this->size = size; return *this; }
        constexpr auto &SetAlign(u16 align) noexcept { this->align = align; return *this; }
        constexpr auto &SetFlags(BufferFlags flags) noexcept { this->flags = flags; return *this; }
        constexpr auto &SetHostMemoryUsage(HostMemoryAccess host_memory_usage) noexcept { this->host_memory_usage = host_memory_usage; return *this; }
        constexpr auto &SetMemoryType(MemoryType memory_type) noexcept { this->memory_type = memory_type; return *this; }
    };
}
