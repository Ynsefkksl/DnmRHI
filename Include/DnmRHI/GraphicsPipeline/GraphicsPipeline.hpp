#pragma once

#include <DnmRHI/GraphicsPipeline/Desc.hpp>

namespace DnmRHI {
    class GraphicsPipeline : public Object {
    protected:
        explicit GraphicsPipeline(Device &device, const GraphicsPipelineDesc &desc) noexcept : m_desc(desc), Object(device, ObjectType::eGraphicsPipeline) {}
    public:
        [[nodiscard]] const auto &GetDesc() const noexcept { return m_desc; }
    protected:
        GraphicsPipelineDesc m_desc;
    };
}
