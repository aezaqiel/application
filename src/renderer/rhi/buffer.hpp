#pragma once

#include "vktypes.hpp"
#include "device.hpp"

namespace application {

    class Buffer
    {
    public:
        struct Info
        {
            VkDeviceSize size;
            VkBufferUsageFlags usage;
            VmaMemoryUsage memory;
            VmaAllocationCreateFlags flags { 0 };
        };

    public:
        Buffer(const Device& device, const Info& info);
        ~Buffer();

        VkBuffer buffer() const { return m_buffer; }
        VkDeviceSize size() const { return m_size; }

        VkDeviceAddress address() const;

        void* map();
        void unmap();

    private:
        const Device& m_device;

        VkBuffer m_buffer { VK_NULL_HANDLE };
        VmaAllocation m_allocation { VK_NULL_HANDLE };
        VmaAllocationInfo m_info;

        VkDeviceSize m_size { 0 };
        bool m_mapped { false };
    };

}
