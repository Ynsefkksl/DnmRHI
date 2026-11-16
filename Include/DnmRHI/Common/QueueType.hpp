#pragma once

#include <DnmRHI/Types.hpp>

namespace DnmRHI {
    enum class QueueType : u8 {
        eGeneral,
        eCompute,
        eCopy,
        Count
    };
}
