#pragma once

#include "vktypes.hpp"
#include "device.hpp"
#include "shader.hpp"

namespace application {

    class ComputePipeline
    {
    public:
        ComputePipeline(const Device* device, const std::string& shader, std::span<VkDescriptorSetLayout> layouts, std::span<VkPushConstantRange> constants = {});
        ~ComputePipeline();

        ComputePipeline(const ComputePipeline&) = delete;
        ComputePipeline& operator=(const ComputePipeline&) = delete;

        VkPipelineLayout layout() const { return m_layout; }
        VkPipeline pipeline() const { return m_pipeline; }

    private:
        const Device* m_device { nullptr };

        std::unique_ptr<Shader> m_shader;

        VkPipelineLayout m_layout { VK_NULL_HANDLE };
        VkPipeline m_pipeline { VK_NULL_HANDLE };
    };

}
