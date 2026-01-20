#pragma once

#include "vktypes.hpp"
#include "device.hpp"

namespace application {

    class CommandList
    {
    public:
        CommandList(VkCommandBuffer cmd);
        ~CommandList() = default;

        VkCommandBuffer cmd() const { return m_cmd; }

        void begin();
        void end();

    private:
        VkCommandBuffer m_cmd { VK_NULL_HANDLE };
    };

    class CommandPool
    {
    public:
        CommandPool(const Device& device, u32 queue_family);
        ~CommandPool();

        void reset(VkCommandPoolResetFlags flags = 0);

        VkCommandBuffer allocate();

    private:
        const Device& m_device;

        VkCommandPool m_pool { VK_NULL_HANDLE };
    };

}
