#pragma once

#include "vktypes.hpp"
#include "device.hpp"

namespace application {

    class TimelineSemaphore
    {
    public:
        TimelineSemaphore(const Device& device);
        ~TimelineSemaphore();

        u64 value();
        void sync(u64 value);

        VkSemaphoreSubmitInfo submit_info(u64 value, VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);

    private:
        const Device& m_device;

        VkSemaphore m_semaphore { VK_NULL_HANDLE };
    };

}
