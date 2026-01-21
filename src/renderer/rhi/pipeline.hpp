#pragma once

#include "vktypes.hpp"
#include "device.hpp"
#include "shader.hpp"

namespace application {

    class Pipeline
    {
    public:
        virtual ~Pipeline();

        VkPipelineLayout layout() const { return m_layout; }
        VkPipeline pipeline() const { return m_pipeline; }

    protected:
        Pipeline(const Device* device, std::span<VkDescriptorSetLayout> layouts, std::span<VkPushConstantRange> constants);

    protected:
        const Device* m_device { nullptr };
        VkPipelineLayout m_layout { VK_NULL_HANDLE };
        VkPipeline m_pipeline { VK_NULL_HANDLE };
    };

    class ComputePipeline final : public Pipeline
    {
    public:
        ComputePipeline(const Device* device, std::span<VkDescriptorSetLayout> layouts, std::span<VkPushConstantRange> constants, const std::string& shader_name);
        virtual ~ComputePipeline() = default;

        ComputePipeline(const ComputePipeline&) = delete;
        ComputePipeline& operator=(const ComputePipeline&) = delete;
    };

    class GraphicsPipeline final : public Pipeline
    {
    public:
        struct Info
        {
            std::string vertex_name;
            std::string fragment_name;

            std::vector<VkFormat> color_formats;
            VkFormat depth_format { VK_FORMAT_UNDEFINED };

            VkPrimitiveTopology topology { VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };

            VkPolygonMode polygon_mode { VK_POLYGON_MODE_FILL };
            VkCullModeFlags cull_mode { VK_CULL_MODE_BACK_BIT };
            VkFrontFace front_face { VK_FRONT_FACE_COUNTER_CLOCKWISE };
            f32 line_width { 1.0f };

            bool depth_test { true };
            bool depth_write { true };
            VkCompareOp depth_compare { VK_COMPARE_OP_LESS_OR_EQUAL };

            bool enable_blend { false };
        };

    public:
        GraphicsPipeline(const Device* device, std::span<VkDescriptorSetLayout> layouts, std::span<VkPushConstantRange> constants, const Info& info);
        virtual ~GraphicsPipeline() = default;

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
    };

}
