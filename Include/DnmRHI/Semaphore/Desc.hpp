#pragma once

#include <DnmRHI/Types.hpp>
#include <string>

namespace DnmRHI {
    struct SemaphoreDesc {
        std::string name;
        u64 initial_value;

        constexpr auto &SetName(std::string_view name) noexcept { this->name = name; return *this; }
        constexpr auto &SetInitialValue(u64 initial_value) noexcept { this->initial_value = initial_value; return *this; }
    };
}
