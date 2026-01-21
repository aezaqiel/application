#include "image.hpp"

namespace application {

    Image::Image(const Device* device, const Info& info)
        : m_device(device)
        , m_extent(info.extent)
        , m_format(info.format)
        , m_aspect(info.aspect)
        , m_mips(info.mips)
        , m_layers(info.layers)
    {
        VkImageCreateInfo image_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = m_extent.depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D,
            .format = m_format,
            .extent = m_extent,
            .mipLevels = m_mips,
            .arrayLayers = m_layers,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = info.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VmaAllocationCreateInfo allocation_info {
            .usage = info.memory,
            .priority = 1.0f
        };

        VK_CHECK(vmaCreateImage(device->allocator(), &image_info, &allocation_info, &m_image, &m_allocation, nullptr));

        VkImageViewCreateInfo view_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = m_image,
            .viewType = m_layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : (m_extent.depth > 1 ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D),
            .format = m_format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = m_aspect,
                .baseMipLevel = 0,
                .levelCount = m_mips,
                .baseArrayLayer = 0,
                .layerCount = m_layers
            }
        };

        VK_CHECK(vkCreateImageView(device->device(), &view_info, nullptr, &m_view));
    }

    Image::~Image()
    {
        vkDestroyImageView(m_device->device(), m_view, nullptr);
        vmaDestroyImage(m_device->allocator(), m_image, m_allocation);
    }

}
