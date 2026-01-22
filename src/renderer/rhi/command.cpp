#include "command.hpp"

namespace application {

    CommandList::CommandList(VkCommandBuffer cmd)
        : m_cmd(cmd)
    {
    }

    void CommandList::begin() const
    {
        VkCommandBufferBeginInfo begin_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr
        };

        VK_CHECK(vkBeginCommandBuffer(m_cmd, &begin_info));
    }

    void CommandList::end() const
    {
        VK_CHECK(vkEndCommandBuffer(m_cmd));
    }

    VkCommandBuffer CommandList::record(std::function<void(VkCommandBuffer)> task)
    {
        begin();
        std::invoke(std::forward<std::function<void(VkCommandBuffer)>>(task), m_cmd);
        end();

        return m_cmd;
    }

    VkCommandBufferSubmitInfo CommandList::submit_info() const 
    {
        return VkCommandBufferSubmitInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBuffer = m_cmd,
            .deviceMask = 0
        };
    }

    void CommandList::barrier(BarrierBatch& barrier) const
    {
        auto dependency = barrier.dependency();
        vkCmdPipelineBarrier2(m_cmd, &dependency);

        barrier.clear();
    }

    void CommandList::clear_image(VkImage image, VkImageLayout layout, VkClearColorValue color, const std::vector<VkImageSubresourceRange>& ranges)
    {
        vkCmdClearColorImage(m_cmd, image, layout, &color, static_cast<u32>(ranges.size()), ranges.data());
    }

    void CommandList::copy_image(VkImage src, VkExtent3D src_extent, VkImage dst, VkExtent3D dst_extent)
    {
        VkImageBlit2 region {
            .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
            .pNext = nullptr,
            .srcSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .srcOffsets = {
                { 0, 0, 0 },
                {
                    static_cast<i32>(src_extent.width),
                    static_cast<i32>(src_extent.height),
                    static_cast<i32>(src_extent.depth)
                }
            },
            .dstSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .dstOffsets = {
                { 0, 0, 0 },
                {
                    static_cast<i32>(dst_extent.width),
                    static_cast<i32>(dst_extent.height),
                    static_cast<i32>(dst_extent.depth)
                }
            }
        };

        VkBlitImageInfo2 blit_info {
            .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
            .pNext = nullptr,
            .srcImage = src,
            .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .dstImage = dst,
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = 1,
            .pRegions = &region,
            .filter = VK_FILTER_LINEAR
        };

        vkCmdBlitImage2(m_cmd, &blit_info);
    }

    void CommandList::copy_buffer(VkBuffer src, VkDeviceSize src_offset, VkBuffer dst, VkDeviceSize dst_offset, VkDeviceSize size)
    {
        VkBufferCopy region {
            .srcOffset = src_offset,
            .dstOffset = dst_offset,
            .size = size
        };

        vkCmdCopyBuffer(m_cmd, src, dst, 1, &region);
    }

    void CommandList::bind_pipeline(const ComputePipeline& pipeline)
    {
        vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipeline());
    }

    void CommandList::bind_pipeline(const GraphicsPipeline& pipeline)
    {
        vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline());
    }

    void CommandList::bind_set(const ComputePipeline& pipeline, std::span<VkDescriptorSet> sets, u32 first)
    {
        vkCmdBindDescriptorSets(m_cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.layout(), first, static_cast<u32>(sets.size()), sets.data(), 0, nullptr);
    }

    void CommandList::bind_set(const GraphicsPipeline& pipeline, std::span<VkDescriptorSet> sets, u32 first)
    {
        vkCmdBindDescriptorSets(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout(), first, static_cast<u32>(sets.size()), sets.data(), 0, nullptr);
    }

    void CommandList::push_constants(const ComputePipeline& pipeline, u32 offset, u32 size, const void* data)
    {
        vkCmdPushConstants(m_cmd, pipeline.layout(), VK_SHADER_STAGE_COMPUTE_BIT, offset, size, data);
    }

    void CommandList::push_constants(const GraphicsPipeline& pipeline, VkShaderStageFlags stage, u32 offset, u32 size, const void* data)
    {
        vkCmdPushConstants(m_cmd, pipeline.layout(), stage, offset, size, data);
    }

    void CommandList::bind_index_buffer(VkBuffer buffer, VkDeviceSize offset)
    {
        vkCmdBindIndexBuffer(m_cmd, buffer, offset, VK_INDEX_TYPE_UINT32);
    }

    void CommandList::dispatch(u32 x, u32 y, u32 z)
    {
        vkCmdDispatch(m_cmd, x, y, z);
    }

    void CommandList::begin_render(const VkRenderingInfo& info)
    {
        vkCmdBeginRendering(m_cmd, &info);
    }

    void CommandList::end_render()
    {
        vkCmdEndRendering(m_cmd);
    }

    void CommandList::set_viewport(f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth)
    {
        VkViewport viewport { x, y, width, height, min_depth, max_depth };
        vkCmdSetViewport(m_cmd, 0, 1, &viewport);
    }

    void CommandList::set_scissor(i32 x, i32 y, u32 width, u32 height)
    {
        VkRect2D scissor { { x, y, }, { width, height } };
        vkCmdSetScissor(m_cmd, 0, 1, &scissor);
    }

    void CommandList::draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance)
    {
        vkCmdDraw(m_cmd, vertex_count, instance_count, first_vertex, first_instance);
    }

    void CommandList::draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance)
    {
        vkCmdDrawIndexed(m_cmd, index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    CommandPool::CommandPool(const Device* device, u32 queue_family)
        : m_device(device)
    {
        VkCommandPoolCreateInfo pool_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = queue_family
        };

        VK_CHECK(vkCreateCommandPool(device->device(), &pool_info, nullptr, &m_pool));
    }

    CommandPool::~CommandPool()
    {
        vkDestroyCommandPool(m_device->device(), m_pool, nullptr);
    }

    void CommandPool::reset(VkCommandPoolResetFlags flags)
    {
        VK_CHECK(vkResetCommandPool(m_device->device(), m_pool, flags));
    }

    CommandList CommandPool::allocate()
    {
        VkCommandBufferAllocateInfo allocate_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer cmd;
        VK_CHECK(vkAllocateCommandBuffers(m_device->device(), &allocate_info, &cmd));

        return CommandList(cmd);
    }

}
