#include "renderer.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "rhi/barrier.hpp"

namespace application {

    Renderer::Renderer(const Window& window, EventDispatcher& dispatcher)
        : m_width(window.width()), m_height(window.height())
    {
        m_context = std::make_unique<Context>(window);
        m_device = std::make_unique<Device>(*m_context);

        m_swapchain = std::make_unique<Swapchain>(*m_context, *m_device);
        m_swapchain->create(VkExtent2D { m_width, m_height }, 0);

        m_graphics_queue = std::make_unique<Queue>(*m_device, m_device->graphics_family());
        m_compute_queue = std::make_unique<Queue>(*m_device, m_device->compute_family());
        m_transfer_queue = std::make_unique<Queue>(*m_device, m_device->transfer_family());

        for (auto& frame : m_frames) {
            frame.command_pool = std::make_unique<CommandPool>(*m_device, m_device->graphics_family());
        }

        m_timeline = std::make_unique<TimelineSemaphore>(*m_device);

        dispatcher.subscribe<WindowResizedEvent>([this](const WindowResizedEvent& e) -> bool {
            m_width = e.width;
            m_height = e.height;

            return false;
        });
    }

    Renderer::~Renderer()
    {
        m_device->wait_idle();
    }

    void Renderer::draw()
    {
        auto& frame = m_frames[m_frame_index % s_frames_in_flight];

        m_timeline->sync(frame.fence);

        if (!m_swapchain->acquire()) {
            m_swapchain->create(VkExtent2D { m_width, m_height }, m_frame_index);
            return;
        }

        frame.command_pool->reset();
        auto cmd = frame.command_pool->allocate();

        cmd.begin();

        auto barrier = BarrierBatch()
            .image(m_swapchain->current_image(),
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_CLEAR_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            );

        cmd.barrier(barrier);

        VkClearColorValue clear_color = {{
            std::abs(std::tan(m_frame_index / 120.0f)),
            std::abs(std::cos(m_frame_index / 120.0f)),
            std::abs(std::sin(m_frame_index / 120.0f)),
            1.0f
        }};

        VkImageSubresourceRange clear_range {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        };

        vkCmdClearColorImage(cmd.cmd(), m_swapchain->current_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &clear_range);

        barrier = BarrierBatch()
            .image(m_swapchain->current_image(),
                VK_PIPELINE_STAGE_2_CLEAR_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            );

        cmd.barrier(barrier);

        cmd.end();

        std::vector<VkCommandBufferSubmitInfo> cmds = {
            cmd.submit_info()
        };

        std::vector<VkSemaphoreSubmitInfo> waits = {
            m_swapchain->acquire_wait_info()
        };

        std::vector<VkSemaphoreSubmitInfo> signals = {
            m_swapchain->present_signal_info(),
            m_timeline->submit_info(m_frame_index+1, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT)
        };

        m_graphics_queue->submit(cmds, waits, signals);

        frame.fence = ++m_frame_index;

        if (!m_swapchain->present(m_graphics_queue->queue())) {
            m_swapchain->create(VkExtent2D { m_width, m_height }, m_frame_index);
        }
    }

}
