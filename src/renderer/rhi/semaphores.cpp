#include "semaphores.hpp"

namespace application {

    TimelineSemaphore::TimelineSemaphore(const Device& device)
        : m_device(device)
    {
        VkSemaphoreTypeCreateInfo type_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = 0
        };

        VkSemaphoreCreateInfo semaphore_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &type_info,
            .flags = 0
        };

        VK_CHECK(vkCreateSemaphore(m_device.device(), &semaphore_info, nullptr, &m_semaphore));
    }

    TimelineSemaphore::~TimelineSemaphore()
    {
        vkDestroySemaphore(m_device.device(), m_semaphore, nullptr);
    }

    u64 TimelineSemaphore::value()
    {
        u64 value = 0;
        VK_CHECK(vkGetSemaphoreCounterValue(m_device.device(), m_semaphore, &value));

        return value;
    }

    void TimelineSemaphore::sync(u64 value)
    {
        if (this->value() >= value) return;

        VkSemaphoreWaitInfo wait_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .pNext = nullptr,
            .flags = 0,
            .semaphoreCount = 1,
            .pSemaphores = &m_semaphore,
            .pValues = &value
        };

        VK_CHECK(vkWaitSemaphores(m_device.device(), &wait_info, std::numeric_limits<u64>::max()));
    }

    VkSemaphoreSubmitInfo TimelineSemaphore::submit_info(u64 value, VkPipelineStageFlags2 stage)
    {
        return VkSemaphoreSubmitInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = m_semaphore,
            .value = value,
            .stageMask = stage,
            .deviceIndex = 0
        };
    }

}
