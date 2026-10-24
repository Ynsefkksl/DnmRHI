#pragma once

#include <DnmRHI/Types.hpp>
#include <DnmRHI/Buffer/Flags.hpp>

namespace DnmRHI {
    enum class MemoryType : u8 {
        eAuto,
        eDevice,
        eHost,
    };
}
