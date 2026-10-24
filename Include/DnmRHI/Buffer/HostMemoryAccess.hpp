#pragma once

#include <DnmRHI/Types.hpp>

namespace DnmRHI {
    enum class HostMemoryAccess : u8 {
        eWrite,
        eReadWrite,
        eNone
    };
}
