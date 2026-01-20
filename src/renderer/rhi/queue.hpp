#pragma once

#include "vktypes.hpp"
#include "device.hpp"

namespace application {

    class Queue
    {
    public:
        Queue(const Device& device, u32 queue_family);
        ~Queue();

        Queue(const Queue&) = delete;
        Queue& operator=(const Queue&) = delete;

        VkQueue queue() const { return m_queue; }

        void submit(const std::vector<VkCommandBufferSubmitInfo>& cmds, const std::vector<VkSemaphoreSubmitInfo>& waits, std::vector<VkSemaphoreSubmitInfo>& signals);
        VkSemaphoreSubmitInfo wait_info(VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT) const;

    private:
        const Device& m_device;

        VkQueue m_queue { VK_NULL_HANDLE };
    };

}
