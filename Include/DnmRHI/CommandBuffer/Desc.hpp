#pragma once

#include <DnmRHI/Common/QueueType.hpp>
#include <string>

namespace DnmRHI {
    enum class CommandBufferType : u8 {
        ePrimary,
        eSecondary
    };

    struct CommandBufferDesc { 
        std::string name;
        CommandBufferType type;
        QueueType queue_type;
        
        constexpr auto &SetName(std::string_view name) noexcept { this->name = name; return *this; }
        constexpr auto &SetType(CommandBufferType type) noexcept { this->type = type; return *this; }
        constexpr auto &SetQueueType(QueueType type) noexcept { this->queue_type = type; return *this; }
    };
}
