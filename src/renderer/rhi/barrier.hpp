#pragma once

#include "vktypes.hpp"
#include "image.hpp"

namespace application {

    class BarrierBatch
    {
    public:
        BarrierBatch() = default;
        ~BarrierBatch() = default;

        VkDependencyInfo dependency() const;
        void clear();

        // BarrierBatch& buffer(
        //     const Buffer& buffer,
        //     VkPipelineStageFlags2 src_stage,
        //     VkAccessFlags2 src_access,
        //     VkPipelineStageFlags2 dst_stage,
        //     VkAccessFlags2 dst_access,
        //     u32 src_queue = VK_QUEUE_FAMILY_IGNORED,
        //     u32 dst_queue = VK_QUEUE_FAMILY_IGNORED
        // );

        BarrierBatch& buffer(
            VkBuffer buffer, VkDeviceSize size,
            VkPipelineStageFlags2 src_stage,
            VkAccessFlags2 src_access,
            VkPipelineStageFlags2 dst_stage,
            VkAccessFlags2 dst_access,
            u32 src_queue = VK_QUEUE_FAMILY_IGNORED,
            u32 dst_queue = VK_QUEUE_FAMILY_IGNORED
        );

        BarrierBatch& image(
            const Image& image,
            VkPipelineStageFlags2 src_stage,
            VkAccessFlags2 src_access,
            VkPipelineStageFlags2 dst_stage,
            VkAccessFlags2 dst_access,
            VkImageLayout old_layout,
            VkImageLayout new_layout,
            VkImageAspectFlags aspect,
            u32 src_queue = VK_QUEUE_FAMILY_IGNORED,
            u32 dst_queue = VK_QUEUE_FAMILY_IGNORED
        );

        BarrierBatch& image(
            VkImage image,
            VkPipelineStageFlags2 src_stage,
            VkAccessFlags2 src_access,
            VkPipelineStageFlags2 dst_stage,
            VkAccessFlags2 dst_access,
            VkImageLayout old_layout,
            VkImageLayout new_layout,
            u32 src_queue = VK_QUEUE_FAMILY_IGNORED,
            u32 dst_queue = VK_QUEUE_FAMILY_IGNORED
        );

        BarrierBatch& memory(
            VkPipelineStageFlags2 src_stage,
            VkAccessFlags2 src_access,
            VkPipelineStageFlags2 dst_stage,
            VkAccessFlags2 dst_access
        );

    private:
        std::vector<VkBufferMemoryBarrier2> m_buffers;
        std::vector<VkImageMemoryBarrier2> m_images;
        std::vector<VkMemoryBarrier2> m_memory;
    };

}
