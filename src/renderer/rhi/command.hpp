#pragma once

#include "vktypes.hpp"
#include "device.hpp"
#include "barrier.hpp"

namespace application {

    class CommandList
    {
    public:
        CommandList(VkCommandBuffer cmd);
        ~CommandList() = default;

        CommandList(const CommandList&) = delete;
        CommandList& operator=(const CommandList&) = delete;

        VkCommandBuffer cmd() const { return m_cmd; }

        void begin() const;
        void end() const;

        VkCommandBufferSubmitInfo submit_info() const;

        void barrier(BarrierBatch& barrier) const;
        void clear_image(VkImage image, VkImageLayout layout, VkClearColorValue color, const std::vector<VkImageSubresourceRange>& ranges);
        void copy_image(VkImage src, VkExtent3D src_extent, VkImage dst, VkExtent3D dst_extent);

    private:
        VkCommandBuffer m_cmd { VK_NULL_HANDLE };
    };

    class CommandPool
    {
    public:
        CommandPool(const Device& device, u32 queue_family);
        ~CommandPool();

        CommandPool(const CommandPool&) = delete;
        CommandPool& operator=(const CommandPool&) = delete;

        void reset(VkCommandPoolResetFlags flags = 0);

        CommandList allocate();

    private:
        const Device& m_device;

        VkCommandPool m_pool { VK_NULL_HANDLE };
    };

}
