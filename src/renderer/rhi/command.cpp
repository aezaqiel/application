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
