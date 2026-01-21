#include "pipeline.hpp"

namespace application {

    ComputePipeline::ComputePipeline(const Device* device, const std::string& shader, std::span<VkDescriptorSetLayout> layouts, std::span<VkPushConstantRange> constants)
        : m_device(device), m_shader(std::make_unique<Shader>(device, shader, VK_SHADER_STAGE_COMPUTE_BIT))
    {
        VkPipelineLayoutCreateInfo layout_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = static_cast<u32>(layouts.size()),
            .pSetLayouts = layouts.data(),
            .pushConstantRangeCount = static_cast<u32>(constants.size()),
            .pPushConstantRanges = constants.data()
        };

        VK_CHECK(vkCreatePipelineLayout(device->device(), &layout_info, nullptr, &m_layout));

        VkComputePipelineCreateInfo pipeline_info {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = m_shader->stage_info(),
            .layout = m_layout,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        VK_CHECK(vkCreateComputePipelines(device->device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_pipeline));
    }

    ComputePipeline::~ComputePipeline()
    {
        vkDestroyPipeline(m_device->device(), m_pipeline, nullptr);
        vkDestroyPipelineLayout(m_device->device(), m_layout, nullptr);
    }

}
