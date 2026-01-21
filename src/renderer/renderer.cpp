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

        for (auto& frame : m_frames) {
            frame.command_pool = std::make_unique<CommandPool>(m_device.get(), m_device->graphics_family());
            frame.descriptor_allocator = std::make_unique<DescriptorAllocator>(m_device.get());
        }

        m_timeline = std::make_unique<TimelineSemaphore>(m_device.get());

        m_storage_image = std::make_unique<Image>(m_device.get(), Image::Info {
            .extent = { m_width, m_height, 1 },
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .memory = VMA_MEMORY_USAGE_GPU_ONLY
        });

        m_compute_layout = DescriptorLayout::Builder(m_device.get())
            .add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
            .build();

        std::vector<VkDescriptorSetLayout> compute_layouts {
            m_compute_layout->layout()
        };

        m_compute_pipeline = std::make_unique<ComputePipeline>(m_device.get(), compute_layouts, std::span<VkPushConstantRange>{}, "gradient.spv");

        m_triangle_layout = DescriptorLayout::Builder(m_device.get()).build();

        std::vector<VkDescriptorSetLayout> triangle_layouts {
            m_triangle_layout->layout()
        };

        m_triangle_pipeline = std::make_unique<GraphicsPipeline>(m_device.get(), triangle_layouts, std::span<VkPushConstantRange>{}, GraphicsPipeline::Info {
            .vertex_name = "vertex.spv",
            .fragment_name = "fragment.spv",
            .color_formats = { m_storage_image->format() },
            .front_face = VK_FRONT_FACE_CLOCKWISE,
            .depth_test = false,
            .depth_write = false,
        });
    }

    Renderer::~Renderer()
    {
        m_device->wait_idle();
        m_gc.flush();
    }

    bool Renderer::begin_frame()
    {
        auto& frame = m_frames[m_frame_index % s_frames_in_flight];

        m_timeline->sync(frame.fence);

        frame.gc.flush();
        m_swapchain->cleanup();

        if (!m_swapchain->acquire()) {
            m_swapchain->create(VkExtent2D { m_width, m_height });
            return false;
        }

        frame.command_pool->reset();
        frame.descriptor_allocator->reset();

        return true;
    }

    void Renderer::end_frame()
    {
        if (!m_swapchain->present(m_graphics_queue->queue())) {
            m_swapchain->create(VkExtent2D { m_width, m_height });
        }
    }

    void Renderer::draw()
    {
        auto& frame = m_frames[m_frame_index % s_frames_in_flight];

        auto cmd = frame.command_pool->allocate();

        cmd.begin();

        // --- COMPUTE DRAW ---

        auto barrier = BarrierBatch()
            .image(m_storage_image->image(),
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL
            );

        cmd.barrier(barrier);

        frame.compute_descriptor = frame.descriptor_allocator->allocate(*m_compute_layout);

        DescriptorWriter(m_device.get())
            .write_storage_image(0, *m_storage_image, VK_IMAGE_LAYOUT_GENERAL)
            .update(frame.compute_descriptor);

        std::array<VkDescriptorSet, 1> compute_sets { frame.compute_descriptor };
        
        cmd.bind_pipeline(*m_compute_pipeline);
        cmd.bind_set(*m_compute_pipeline, compute_sets, 0);
        cmd.dispatch(std::ceil(m_storage_image->width() / 16.0f), std::ceil(m_storage_image->height() / 16.0f), 1);

        // --- GRAPHICS DRAW ---

        barrier = BarrierBatch()
            .image(m_storage_image->image(),
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            );

        cmd.barrier(barrier);

        std::array<VkRenderingAttachmentInfo, 1> render_attachments {
            VkRenderingAttachmentInfo {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext = nullptr,
                .imageView = m_storage_image->view(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .resolveImageView = VK_NULL_HANDLE,
                .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {
                    .color = { 0.0f, 0.0f, 0.0f, 1.0f }
                }
            }
        };

        VkRenderingInfo render_info {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderArea = {
                .offset = { 0, 0 },
                .extent = { m_storage_image->width(), m_storage_image->height() }
            },
            .layerCount = 1,
            .viewMask = 0,
            .colorAttachmentCount = static_cast<u32>(render_attachments.size()),
            .pColorAttachments = render_attachments.data(),
            .pDepthAttachment = nullptr,
            .pStencilAttachment = nullptr
        };

        cmd.begin_render(render_info);

        cmd.bind_pipeline(*m_triangle_pipeline);

        cmd.set_viewport(0.0f, 0.0f, static_cast<f32>(m_storage_image->width()), static_cast<f32>(m_storage_image->height()), 0.0f, 1.0f);
        cmd.set_scissor(0, 0, m_storage_image->width(), m_storage_image->height());

        cmd.draw(3, 1, 0, 0);

        cmd.end_render();

        // --- BLIT TO SWAPCHAIN ---

        barrier = BarrierBatch()
            .image(m_swapchain->current_image(),
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            )
            .image(m_storage_image->image(),
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
            );

        cmd.barrier(barrier);

        cmd.copy_image(m_storage_image->image(), m_storage_image->extent(), m_swapchain->current_image(), { m_swapchain->width(), m_swapchain->height(), 1 });

        // --- PRESENT ---

        barrier = BarrierBatch()
            .image(m_swapchain->current_image(),
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
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
    }

}
