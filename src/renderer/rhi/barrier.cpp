#include "barrier.hpp"

namespace application {

    VkDependencyInfo BarrierBatch::dependency() const
    {
        return VkDependencyInfo {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = 0,
            .memoryBarrierCount = static_cast<u32>(m_memory.size()),
            .pMemoryBarriers = m_memory.data(),
            .bufferMemoryBarrierCount = static_cast<u32>(m_buffers.size()),
            .pBufferMemoryBarriers = m_buffers.data(),
            .imageMemoryBarrierCount = static_cast<u32>(m_images.size()),
            .pImageMemoryBarriers = m_images.data()
        };
    }

    void BarrierBatch::clear()
    {
        m_buffers.clear();
        m_images.clear();
        m_memory.clear();
    }

    BarrierBatch& BarrierBatch::buffer(
        const Buffer& buffer,
        VkPipelineStageFlags2 src_stage,
        VkAccessFlags2 src_access,
        VkPipelineStageFlags2 dst_stage,
        VkAccessFlags2 dst_access,
        u32 src_queue,
        u32 dst_queue
    )
    {
        m_buffers.push_back(VkBufferMemoryBarrier2 {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = src_stage,
            .srcAccessMask = src_access,
            .dstStageMask = dst_stage,
            .dstAccessMask = dst_access,
            .srcQueueFamilyIndex = src_queue,
            .dstQueueFamilyIndex = dst_queue,
            .buffer = buffer.buffer(),
            .offset = 0,
            .size = buffer.size()
        });

        return *this;
    }

    BarrierBatch& BarrierBatch::buffer(
        VkBuffer buffer, VkDeviceSize size,
        VkPipelineStageFlags2 src_stage,
        VkAccessFlags2 src_access,
        VkPipelineStageFlags2 dst_stage,
        VkAccessFlags2 dst_access,
        u32 src_queue,
        u32 dst_queue
    )
    {
        m_buffers.push_back(VkBufferMemoryBarrier2 {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = src_stage,
            .srcAccessMask = src_access,
            .dstStageMask = dst_stage,
            .dstAccessMask = dst_access,
            .srcQueueFamilyIndex = src_queue,
            .dstQueueFamilyIndex = dst_queue,
            .buffer = buffer,
            .offset = 0,
            .size = size
        });

        return *this;
    }

    BarrierBatch& BarrierBatch::image(
        const Image& image,
        VkPipelineStageFlags2 src_stage,
        VkAccessFlags2 src_access,
        VkPipelineStageFlags2 dst_stage,
        VkAccessFlags2 dst_access,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        VkImageAspectFlags aspect,
        u32 src_queue,
        u32 dst_queue
    )
    {
        m_images.push_back(VkImageMemoryBarrier2 {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = src_stage,
            .srcAccessMask = src_access,
            .dstStageMask = dst_stage,
            .dstAccessMask = dst_access,
            .oldLayout = old_layout,
            .newLayout = new_layout,
            .srcQueueFamilyIndex = src_queue,
            .dstQueueFamilyIndex = dst_queue,
            .image = image.image(),
            .subresourceRange = {
                .aspectMask = aspect,
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS
            }
        });

        return *this;
    }

    BarrierBatch& BarrierBatch::image(
        VkImage image,
        VkPipelineStageFlags2 src_stage,
        VkAccessFlags2 src_access,
        VkPipelineStageFlags2 dst_stage,
        VkAccessFlags2 dst_access,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        u32 src_queue,
        u32 dst_queue
    )
    {
        m_images.push_back(VkImageMemoryBarrier2 {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = src_stage,
            .srcAccessMask = src_access,
            .dstStageMask = dst_stage,
            .dstAccessMask = dst_access,
            .oldLayout = old_layout,
            .newLayout = new_layout,
            .srcQueueFamilyIndex = src_queue,
            .dstQueueFamilyIndex = dst_queue,
            .image = image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS
            }
        });

        return *this;
    }

    BarrierBatch& BarrierBatch::memory(
        VkPipelineStageFlags2 src_stage,
        VkAccessFlags2 src_access,
        VkPipelineStageFlags2 dst_stage,
        VkAccessFlags2 dst_access
    )
    {
        m_memory.push_back(VkMemoryBarrier2 {
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = src_stage,
            .srcAccessMask = src_access,
            .dstStageMask = dst_stage,
            .dstAccessMask = dst_access
        });

        return *this;
    }

}
