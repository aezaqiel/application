#pragma once

#include "vktypes.hpp"
#include "device.hpp"

namespace application {

    class TimelineSemaphore
    {
    public:
        TimelineSemaphore(const Device* device);
        ~TimelineSemaphore();

        TimelineSemaphore(const TimelineSemaphore&) = delete;
        TimelineSemaphore& operator=(const TimelineSemaphore&) = delete;

        VkSemaphore semaphore() const { return m_semaphore; }

        u64 value();
        void sync(u64 value);

        VkSemaphoreSubmitInfo submit_info(u64 value, VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);

    private:
        const Device* m_device;

        VkSemaphore m_semaphore { VK_NULL_HANDLE };
    };

    class BinarySemaphore
    {
    public:
        BinarySemaphore(const Device* device);
        ~BinarySemaphore();

        BinarySemaphore(const BinarySemaphore&) = delete;
        BinarySemaphore& operator=(const BinarySemaphore&) = delete;

        VkSemaphore semaphore() const { return m_semaphore; }

        VkSemaphoreSubmitInfo submit_info(VkPipelineStageFlags2 stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);

    private:
        const Device* m_device;

        VkSemaphore m_semaphore { VK_NULL_HANDLE };
    };

}
