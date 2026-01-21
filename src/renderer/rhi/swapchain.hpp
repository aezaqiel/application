#pragma once

#include "vktypes.hpp"
#include "context.hpp"
#include "device.hpp"
#include "semaphores.hpp"

namespace application {

    class Swapchain
    {
    public:
        Swapchain(const Context* context, const Device* devce);
        ~Swapchain();

        Swapchain(const Swapchain&) = delete;
        Swapchain& operator=(const Swapchain&) = delete;

        VkSwapchainKHR swapchain() const { return m_swapchain; }

        VkExtent2D extent() const { return m_extent; }
        u32 width() const { return m_extent.width; }
        u32 height() const { return m_extent.height; }

        VkSurfaceCapabilitiesKHR surface_capabilities() const { return m_capabilities; }
        VkSurfaceFormatKHR surface_format() const { return m_surface_format; }
        VkPresentModeKHR present_mode() const { return m_present_mode; }

        u32 image_index() const { return m_image_index; }

        const std::vector<VkImage>& images() const { return m_images; }
        const std::vector<VkImageView>& views() const { return m_views; }

        VkImage current_image() const { return m_images[m_image_index]; }
        VkImageView current_view() const { return m_views[m_image_index]; }

        void create(VkExtent2D extent);

        bool acquire();
        bool present(VkQueue queue);

        VkSemaphoreSubmitInfo acquire_wait_info();
        VkSemaphoreSubmitInfo present_signal_info();

        void cleanup();

    private:
        struct RetiredResources
        {
            std::vector<VkImageView> views;
            std::vector<std::unique_ptr<BinarySemaphore>> acquire_semaphores;
            std::vector<std::unique_ptr<BinarySemaphore>> present_semaphores;

            VkSwapchainKHR swapchain;

            u64 fence;
        };

    private:
        const Context* m_context;
        const Device* m_device;

        VkSwapchainKHR m_swapchain { VK_NULL_HANDLE };

        VkSurfaceCapabilitiesKHR m_capabilities;
        VkSurfaceFormatKHR m_surface_format;
        VkPresentModeKHR m_present_mode;
        VkExtent2D m_extent;

        u32 m_image_count { 0 };
        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_views;

        std::vector<std::unique_ptr<BinarySemaphore>> m_image_acquired_semaphores;
        std::vector<std::unique_ptr<BinarySemaphore>> m_present_signal_semaphores;

        u32 m_image_index { 0 };

        u64 m_frame_index { 0 };
        std::vector<RetiredResources> m_retired;
    };

}
