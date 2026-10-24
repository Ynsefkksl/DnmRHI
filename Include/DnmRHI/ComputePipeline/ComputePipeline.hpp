#pragma once

#include <DnmRHI/ComputePipeline/Desc.hpp>

namespace DnmRHI {
    class ComputePipeline : public Object {
    protected:
        explicit ComputePipeline(Device &device, const ComputePipelineDesc &desc) noexcept : m_desc(desc), Object(device, ObjectType::eComputePipeline) {}
    public:
        [[nodiscard]] const auto &GetDesc() const noexcept { return m_desc; }
    protected:
        ComputePipelineDesc m_desc;
    };
}