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

    CommandPool::CommandPool(const Device& device, u32 queue_family)
        : m_device(device)
    {
        VkCommandPoolCreateInfo pool_info {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = queue_family
        };

        VK_CHECK(vkCreateCommandPool(device.device(), &pool_info, nullptr, &m_pool));
    }

    CommandPool::~CommandPool()
    {
        vkDestroyCommandPool(m_device.device(), m_pool, nullptr);
    }

    void CommandPool::reset(VkCommandPoolResetFlags flags)
    {
        VK_CHECK(vkResetCommandPool(m_device.device(), m_pool, flags));
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
        VK_CHECK(vkAllocateCommandBuffers(m_device.device(), &allocate_info, &cmd));

        return CommandList(cmd);
    }

}
