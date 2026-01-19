#pragma once

#include "vktypes.hpp"
#include "context.hpp"

namespace application {

    struct QueueFamilyIndices
    {
        u32 graphics { std::numeric_limits<u32>::max() };
        u32 compute  { std::numeric_limits<u32>::max() };
        u32 transfer { std::numeric_limits<u32>::max() };
    };

    class Device
    {
    public:
        Device(const Context& context);
        ~Device();

        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

        VkPhysicalDevice physical() const { return m_physical_device; }
        VkDevice device() const { return m_device; }
        VmaAllocator allocator() const { return m_allocator; }

        u32 graphics_index() const { return m_queue_indices.graphics; }
        u32 compute_index()  const { return m_queue_indices.compute;  }
        u32 transfer_index() const { return m_queue_indices.transfer; }

        VkPhysicalDeviceProperties props() const { return m_props.properties; }
        VkPhysicalDeviceAccelerationStructurePropertiesKHR as_props() const { return m_as_props; }
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_props() const { return m_rt_props; }

        void wait_idle() const;

    private:
        const Context& m_context;

        VkPhysicalDevice m_physical_device { VK_NULL_HANDLE };
        VkDevice m_device { VK_NULL_HANDLE };
        VmaAllocator m_allocator { VK_NULL_HANDLE };

        QueueFamilyIndices m_queue_indices;

        VkPhysicalDeviceProperties2 m_props;
        VkPhysicalDeviceAccelerationStructurePropertiesKHR m_as_props;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rt_props;
    };

}