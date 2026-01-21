#include "renderer.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "rhi/barrier.hpp"

#include "rhi/shader.hpp"

namespace application {

    Renderer::Renderer(const Window& window, EventDispatcher& dispatcher)
        : m_width(window.width()), m_height(window.height())
    {
        m_context = std::make_unique<Context>(window);
        m_device = std::make_unique<Device>(m_context.get());

        m_swapchain = std::make_unique<Swapchain>(m_context.get(), m_device.get());
        m_swapchain->create(VkExtent2D { m_width, m_height });

        dispatcher.subscribe<WindowResizedEvent>([this](const WindowResizedEvent& e) -> bool {
            m_width = e.width;
            m_height = e.height;

            return false;
        });

        m_graphics_queue = std::make_unique<Queue>(m_device.get(), m_device->graphics_family());
        m_compute_queue = std::make_unique<Queue>(m_device.get(), m_device->compute_family());
        m_transfer_queue = std::make_unique<Queue>(m_device.get(), m_device->transfer_family());

        m_descriptor_allocator = std::make_unique<DescriptorAllocator>(m_device.get());

        for (auto& frame : m_frames) {
            frame.command_pool = std::make_unique<CommandPool>(m_device.get(), m_device->graphics_family());
        }

        m_timeline = std::make_unique<TimelineSemaphore>(m_device.get());

        m_draw_image = std::make_unique<Image>(m_device.get(), Image::Info {
            .extent = { m_width, m_height, 1 },
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .memory = VMA_MEMORY_USAGE_GPU_ONLY
        });

        m_draw_layout = DescriptorLayout::Builder(m_device.get())
            .add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT)
            .build();

        m_draw_set = m_descriptor_allocator->allocate(*m_draw_layout);

        Shader test(m_device.get(), "gradient.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    }

    Renderer::~Renderer()
    {
        m_device->wait_idle();
        m_gc.flush();
    }

    void Renderer::draw()
    {
        auto& frame = m_frames[m_frame_index % s_frames_in_flight];

        m_timeline->sync(frame.fence);

        frame.gc.flush();
        m_swapchain->cleanup();

        if (!m_swapchain->acquire()) {
            m_swapchain->create(VkExtent2D { m_width, m_height });
            return;
        }

        frame.command_pool->reset();
        auto cmd = frame.command_pool->allocate();

        cmd.begin();

        DescriptorWriter(m_device.get())
            .write_storage_image(0, *m_draw_image)
            .update(m_draw_set);

        auto barrier = BarrierBatch()
            .image(m_draw_image->image(),
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_CLEAR_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL
            );

        cmd.barrier(barrier);

        VkClearColorValue clear_color = {{
            std::abs(std::cos(m_frame_index / 120.0f)),
            std::abs(std::sin(m_frame_index / 120.0f)),
            std::abs(std::tan(m_frame_index / 120.0f)),
            1.0f
        }};

        std::vector<VkImageSubresourceRange> clear_ranges {
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS
            }
        };

        cmd.clear_image(m_draw_image->image(), VK_IMAGE_LAYOUT_GENERAL, clear_color, clear_ranges);

        barrier = BarrierBatch()
            .image(m_swapchain->current_image(),
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            )
            .image(m_draw_image->image(),
                VK_PIPELINE_STAGE_2_CLEAR_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
            );

        cmd.barrier(barrier);

        cmd.copy_image(m_draw_image->image(), m_draw_image->extent(), m_swapchain->current_image(), { m_swapchain->width(), m_swapchain->height(), 1 });

        barrier = BarrierBatch()
            .image(m_swapchain->current_image(),
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_NONE,
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
            m_swapchain->create(VkExtent2D { m_width, m_height });
        }
    }

}
