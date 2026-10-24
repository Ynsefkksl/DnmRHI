#pragma once

#include <DnmRHI/Types.hpp>

namespace DnmRHI {
    enum class QueueType : u8 {
        eNone,
        eGeneral,
        eCompute,
        eCopy,
        Count
    };
}
