#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/GraphicsPipeline/GraphicsPipeline.hpp>

namespace DnmRHI::Vk {
    class GraphicsPipeline final : public DnmRHI::GraphicsPipeline {
    public:
        explicit GraphicsPipeline(Vk::Device &device, const GraphicsPipelineDesc &desc);
        virtual ~GraphicsPipeline() override { 
            GetVkDevice.GetDevice().waitIdle();
            GetVkDevice.GetDevice().destroy(m_pipeline); 
        }

        [[nodiscard]] auto GetPipeline() const noexcept { return m_pipeline; }
    private:
        vk::Pipeline m_pipeline;
    };
};