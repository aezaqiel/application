#include "buffer.hpp"

namespace application {

    Buffer::Buffer(const Device* device, const Info& info)
        : m_device(device)
        , m_size(info.size)
    {
        VkBufferCreateInfo buffer_info {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = m_size,
            .usage = info.usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr
        };

        VmaAllocationCreateInfo allocation_info {
            .flags = info.flags,
            .usage = info.memory
        };

        VK_CHECK(vmaCreateBuffer(device->allocator(), &buffer_info, &allocation_info, &m_buffer, &m_allocation, &m_info));

        if (info.flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
            m_mapped = true;
        }
    }

    Buffer::~Buffer()
    {
        if (m_mapped) unmap();
        vmaDestroyBuffer(m_device->allocator(), m_buffer, m_allocation);
    }

    VkDeviceAddress Buffer::address() const
    {
        VkBufferDeviceAddressInfo address_info {
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .pNext = nullptr,
            .buffer = m_buffer
        };

        return vkGetBufferDeviceAddress(m_device->device(), &address_info);
    }

    void* Buffer::map()
    {
        if (m_mapped) return static_cast<void*>(m_info.pMappedData);

        void* data;
        VK_CHECK(vmaMapMemory(m_device->allocator(), m_allocation, &data));

        m_mapped = true;

        return data;
    }

    void Buffer::unmap()
    {
        if (!m_mapped) return;

        vmaUnmapMemory(m_device->allocator(), m_allocation);
        m_mapped = false;
    }

}
