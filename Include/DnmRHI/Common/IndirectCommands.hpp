#pragma once

#include <DnmRHI/Types.hpp>

namespace DnmRHI {
    struct IndirectDrawCommand {
        u32 vertex_count;
        u32 instance_count;
        u32 first_vertex;
        u32 first_instance;
    };

    struct IndirectDrawIndexedCommand {
        u32 index_count;
        u32 instance_count;
        u32 first_index;
        i32 vertex_offset;
        u32 first_instance;
    };
}
