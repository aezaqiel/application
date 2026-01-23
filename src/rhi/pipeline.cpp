#include "pipeline.hpp"

namespace application {

    Pipeline::Pipeline(const Device* device, std::span<VkDescriptorSetLayout> layouts, std::span<VkPushConstantRange> constants)
        : m_device(device)
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
    }

    Pipeline::~Pipeline()
    {
        vkDestroyPipeline(m_device->device(), m_pipeline, nullptr);
        vkDestroyPipelineLayout(m_device->device(), m_layout, nullptr);
    }

    ComputePipeline::ComputePipeline(const Device* device, std::span<VkDescriptorSetLayout> layouts, std::span<VkPushConstantRange> constants, const std::string& shader_name)
        : Pipeline(device, layouts, constants)
    {
        Shader shader(device, shader_name, VK_SHADER_STAGE_COMPUTE_BIT);

        VkComputePipelineCreateInfo pipeline_info {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = shader.stage_info(),
            .layout = m_layout,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        VK_CHECK(vkCreateComputePipelines(device->device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_pipeline));
    }

    GraphicsPipeline::GraphicsPipeline(const Device* device, std::span<VkDescriptorSetLayout> layouts, std::span<VkPushConstantRange> constants, const Info& info)
        : Pipeline(device, layouts, constants)
    {
        Shader vs(device, info.vertex_name, VK_SHADER_STAGE_VERTEX_BIT);
        Shader fs(device, info.fragment_name, VK_SHADER_STAGE_FRAGMENT_BIT);

        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages {
            vs.stage_info(),
            fs.stage_info()
        };

        VkPipelineVertexInputStateCreateInfo vertex_input {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = info.topology,
            .primitiveRestartEnable = VK_FALSE
        };

        VkPipelineViewportStateCreateInfo viewport_state {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr
        };

        VkPipelineRasterizationStateCreateInfo rasterizer {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = info.polygon_mode,
            .cullMode = info.cull_mode,
            .frontFace = info.front_face,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = info.line_width
        };

        VkPipelineMultisampleStateCreateInfo multisampling {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 0.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        VkPipelineDepthStencilStateCreateInfo depth_stencil {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = info.depth_test ? VK_TRUE : VK_FALSE,
            .depthWriteEnable = info.depth_write ? VK_TRUE : VK_FALSE,
            .depthCompareOp = info.depth_compare,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f
        };

        VkPipelineColorBlendAttachmentState color_blend_attachment {
            .blendEnable = info.enable_blend ? VK_TRUE : VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        std::vector<VkPipelineColorBlendAttachmentState> blend_attachments(info.color_formats.size(), color_blend_attachment);

        VkPipelineColorBlendStateCreateInfo color_blending {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_NO_OP,
            .attachmentCount = static_cast<u32>(blend_attachments.size()),
            .pAttachments = blend_attachments.data(),
            .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
        };

        std::array<VkDynamicState, 2> dynamic_states {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = static_cast<u32>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data()
        };

        VkPipelineRenderingCreateInfo rendering_info {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .pNext = nullptr,
            .viewMask = 0,
            .colorAttachmentCount = static_cast<u32>(info.color_formats.size()),
            .pColorAttachmentFormats = info.color_formats.data(),
            .depthAttachmentFormat = info.depth_format,
            .stencilAttachmentFormat = (info.depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT || info.depth_format == VK_FORMAT_D24_UNORM_S8_UINT) ? info.depth_format : VK_FORMAT_UNDEFINED
        };

        VkGraphicsPipelineCreateInfo pipeline_info {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &rendering_info,
            .flags = 0,
            .stageCount = static_cast<u32>(shader_stages.size()),
            .pStages = shader_stages.data(),
            .pVertexInputState = &vertex_input,
            .pInputAssemblyState = &input_assembly,
            .pTessellationState = nullptr,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depth_stencil,
            .pColorBlendState = &color_blending,
            .pDynamicState = &dynamic_state_info,
            .layout = m_layout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        VK_CHECK(vkCreateGraphicsPipelines(device->device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_pipeline));
    }

}
