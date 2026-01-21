#include "queue.hpp"

namespace application {

    Queue::Queue(const Device* device, u32 queue_family)
    {
        vkGetDeviceQueue(device->device(), queue_family, 0, &m_queue);
    }

    Queue::~Queue()
    {
        vkQueueWaitIdle(m_queue);
    }

    void Queue::submit(const std::vector<VkCommandBufferSubmitInfo>& cmds, const std::vector<VkSemaphoreSubmitInfo>& waits, std::vector<VkSemaphoreSubmitInfo>& signals)
    {
        VkSubmitInfo2 submit {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = nullptr,
            .flags = 0,
            .waitSemaphoreInfoCount = static_cast<u32>(waits.size()),
            .pWaitSemaphoreInfos = waits.data(),
            .commandBufferInfoCount = static_cast<u32>(cmds.size()),
            .pCommandBufferInfos = cmds.data(),
            .signalSemaphoreInfoCount = static_cast<u32>(signals.size()),
            .pSignalSemaphoreInfos = signals.data()
        };

        VK_CHECK(vkQueueSubmit2(m_queue, 1, &submit, VK_NULL_HANDLE));
    }

}
