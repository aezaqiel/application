#include "swapchain.hpp"

namespace application {

    Swapchain::Swapchain(const Context& context, const Device& device)
        : m_context(context), m_device(device)
    {
    }

    Swapchain::~Swapchain()
    {
        m_frame_index = std::numeric_limits<u64>::max();
        cleanup();

        for (auto& view : m_views) {
            vkDestroyImageView(m_device.device(), view, nullptr);
        }

        vkDestroySwapchainKHR(m_device.device(), m_swapchain, nullptr);
    }

    void Swapchain::create(VkExtent2D extent)
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device.physical(), m_context.surface(), &m_capabilities);

        u32 format_count = 0;
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_device.physical(), m_context.surface(), &format_count, nullptr));
        std::vector<VkSurfaceFormatKHR> available_formats(format_count);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_device.physical(), m_context.surface(), &format_count, available_formats.data()));

        m_surface_format = available_formats.at(0);
        for (const auto& format : available_formats) {
            if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                m_surface_format = format;
                break;
            }
        }

        u32 mode_count = 0;
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_device.physical(), m_context.surface(), &mode_count, nullptr));
        std::vector<VkPresentModeKHR> available_modes(mode_count);
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_device.physical(), m_context.surface(), &mode_count, available_modes.data()));

        m_present_mode = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto& mode : available_modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                m_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
        }

        if (m_capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
            m_extent = m_capabilities.currentExtent;
        } else {
            m_extent = {
                .width = std::clamp(extent.width, m_capabilities.minImageExtent.width, m_capabilities.maxImageExtent.width),
                .height = std::clamp(extent.height, m_capabilities.minImageExtent.height, m_capabilities.maxImageExtent.height)
            };
        }

        m_image_count = m_capabilities.minImageCount + 1;
        if (m_capabilities.maxImageCount > 0) {
            m_image_count = std::min(m_image_count, m_capabilities.maxImageCount);
        }

        VkSwapchainKHR old = m_swapchain;

        VkSwapchainCreateInfoKHR swapchain_info {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = m_context.surface(),
            .minImageCount = m_image_count,
            .imageFormat = m_surface_format.format,
            .imageColorSpace = m_surface_format.colorSpace,
            .imageExtent = m_extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = m_capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = m_present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = old
        };

        VK_CHECK(vkCreateSwapchainKHR(m_device.device(), &swapchain_info, nullptr, &m_swapchain));

        if (old != VK_NULL_HANDLE) {
            u64 fence = m_frame_index + static_cast<u64>(m_views.size());

            m_retired.emplace(m_retired.begin(), RetiredResources {
                .views = std::move(m_views),
                .acquire_semaphores = std::move(m_image_acquired_semaphores),
                .present_semaphores = std::move(m_present_signal_semaphores),
                .swapchain = old,
                .fence = fence
            });

            m_views.clear();
            m_image_acquired_semaphores.clear();
            m_present_signal_semaphores.clear();
            m_images.clear();
        }

        VK_CHECK(vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &m_image_count, nullptr));

        m_images.resize(m_image_count);
        m_views.resize(m_image_count);
        m_image_acquired_semaphores.reserve(m_image_count);
        m_present_signal_semaphores.reserve(m_image_count);

        VK_CHECK(vkGetSwapchainImagesKHR(m_device.device(), m_swapchain, &m_image_count, m_images.data()));

        VkImageViewCreateInfo view_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = VK_NULL_HANDLE,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_surface_format.format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        for (usize i = 0; i < m_images.size(); ++i) {
            view_info.image = m_images[i];
            VK_CHECK(vkCreateImageView(m_device.device(), &view_info, nullptr, &m_views[i]));

            m_image_acquired_semaphores.push_back(std::make_unique<BinarySemaphore>(m_device));
            m_present_signal_semaphores.push_back(std::make_unique<BinarySemaphore>(m_device));
        }

        std::println("swapchain created ({}, {}) with {} images", m_extent.width, m_extent.height, m_image_count);
    }

    bool Swapchain::acquire()
    {
        auto semaphore = m_image_acquired_semaphores[m_frame_index % m_image_count]->semaphore();
        VkResult result = vkAcquireNextImageKHR(m_device.device(), m_swapchain, std::numeric_limits<u64>::max(), semaphore, VK_NULL_HANDLE, &m_image_index);

        if (result == VK_SUCCESS) return true;
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) return false;

        VK_CHECK(result);
        return false;
    }

    bool Swapchain::present(VkQueue queue)
    {
        auto semaphore = m_present_signal_semaphores[m_image_index]->semaphore();

        VkPresentInfoKHR present_info {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &semaphore,
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain,
            .pImageIndices = &m_image_index,
            .pResults = nullptr
        };

        VkResult result = vkQueuePresentKHR(queue, &present_info);

        // m_sync_index = (m_sync_index + 1) % m_image_count;
        m_frame_index++;

        if (result == VK_SUCCESS) return true;
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) return false;

        VK_CHECK(result);
        return false;
    }

    VkSemaphoreSubmitInfo Swapchain::acquire_wait_info()
    {
        return m_image_acquired_semaphores[m_frame_index % m_image_count]->submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    VkSemaphoreSubmitInfo Swapchain::present_signal_info()
    {
        return m_present_signal_semaphores[m_image_index]->submit_info(VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
    }

    void Swapchain::cleanup()
    {
        while (!m_retired.empty()) {
            const auto& retired = m_retired.back();

            if (m_frame_index <= retired.fence) break;

            for (usize i = 0; i < retired.views.size(); ++i) {
                vkDestroyImageView(m_device.device(), retired.views[i], nullptr);
            }

            vkDestroySwapchainKHR(m_device.device(), retired.swapchain, nullptr);

            m_retired.pop_back();
        }
    }

}
