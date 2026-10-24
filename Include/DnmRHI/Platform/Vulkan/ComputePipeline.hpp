#pragma once

#include <DnmRHI/Platform/Vulkan/Device.hpp>
#include <DnmRHI/ComputePipeline/ComputePipeline.hpp>

namespace DnmRHI::Vk {
    class ComputePipeline final : public DnmRHI::ComputePipeline {
    public:
        explicit ComputePipeline(Vk::Device &device, const ComputePipelineDesc &desc);
        virtual ~ComputePipeline() override { 
            GetVkDevice.GetDevice().waitIdle();
            GetVkDevice.GetDevice().destroy(m_pipeline); 
        }

        [[nodiscard]] auto GetPipeline() const noexcept { return m_pipeline; }
    private:
        vk::Pipeline m_pipeline;
    };
};