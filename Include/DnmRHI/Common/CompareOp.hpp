#pragma once

#include <DnmRHI/Types.hpp>

namespace DnmRHI {
    enum class CompareOp : u8 {
        eNone,
        eNever,
        eAlways,
        eEqual,
        eNotEqual,
        eLess,
        eLessOrEqual,
        eGreater,
        eGreaterOrEqual,
    };
}
