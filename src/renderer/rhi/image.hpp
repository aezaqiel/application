#pragma once

#include "vktypes.hpp"
#include "device.hpp"

namespace application {

    class Image
    {
    public:
        struct Info
        {
            VkExtent3D extent;
            VkFormat format;
            VkImageAspectFlags aspect;
            VkImageUsageFlags usage;
            u32 mips { 1 };
            u32 layers { 1 };
            VmaMemoryUsage memory { VMA_MEMORY_USAGE_AUTO };
        };

    public:
        Image(const Device& device, const Info& info);
        ~Image();

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;

        VkImage image() const { return m_image; }
        VkImageView view() const { return m_view; }

        VkExtent3D extent() const { return m_extent; }
        u32 width() const { return m_extent.width; }
        u32 height() const { return m_extent.height; }
        u32 depth() const { return m_extent.depth; }

        VkFormat format() const { return m_format; }

        u32 mips() const { return m_mips; }
        u32 layers() const { return m_layers; }

    private:
        const Device& m_device;

        VkImage m_image { VK_NULL_HANDLE };
        VkImageView m_view { VK_NULL_HANDLE };
        VmaAllocation m_allocation { VK_NULL_HANDLE };

        VkExtent3D m_extent { 0, 0, 0 };
        VkFormat m_format { VK_FORMAT_UNDEFINED };
        VkImageAspectFlags m_aspect { VK_IMAGE_ASPECT_NONE };

        u32 m_mips { 1 };
        u32 m_layers { 1 };
    };

}
