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

        std::array<Vertex, 4> vertices {
            Vertex {
                .position = { 0.5f, -0.5f, 0.0f },
                .color = { 0.0f, 0.0f, 0.0f, 1.0f }
            },
            Vertex {
                .position = { 0.5f, 0.5f, 0.0f },
                .color = { 0.5f, 0.5f, 0.5f, 1.0f }
            },
            Vertex {
                .position = { -0.5f, -0.5f, 0.0f },
                .color = { 1.0f, 0.0f, 0.0f, 1.0f }
            },
            Vertex {
                .position = { -0.5f, 0.5f, 0.0f },
                .color = { 0.0f, 1.0f, 0.0f, 1.0f }
            }
        };

        std::array<u32, 6> indices {
            0, 1, 2,
            2, 1, 3
        };

        upload_mesh(vertices, indices);

        m_mesh_layouts = DescriptorLayout::Builder(m_device.get()).build();

        std::vector<VkDescriptorSetLayout> mesh_layouts {
            m_mesh_layouts->layout()
        };

        std::vector<VkPushConstantRange> mesh_constants {
            VkPushConstantRange {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(GPUDrawPushConstants)
            }
        };

        m_mesh_pipeline = std::make_unique<GraphicsPipeline>(m_device.get(), mesh_layouts, mesh_constants, GraphicsPipeline::Info {
            .vertex_name = "vertex.spv",
            .fragment_name = "fragment.spv",
            .color_formats = { m_storage_image->format() },
            .cull_mode = VK_CULL_MODE_NONE,
            .front_face = VK_FRONT_FACE_CLOCKWISE,
            .depth_test = false,
            .depth_write = false
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

        // --- GRAPHICS DRAW ---

        auto barrier = BarrierBatch()
            .image(m_storage_image->image(),
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
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
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {
                    .color = { 0.1f, 0.1f, 0.1f, 1.0f }
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

        cmd.bind_pipeline(*m_mesh_pipeline);

        GPUDrawPushConstants constants {
            .world_transform = glm::mat4(1.0f),
            .vertex_buffer = m_mesh->vertex_buffer->address()
        };

        cmd.push_constants(*m_mesh_pipeline, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &constants);

        cmd.bind_index_buffer(m_mesh->index_buffer->buffer(), 0);

        cmd.set_viewport(0.0f, 0.0f, static_cast<f32>(m_storage_image->width()), static_cast<f32>(m_storage_image->height()), 0.0f, 1.0f);
        cmd.set_scissor(0, 0, m_storage_image->width(), m_storage_image->height());

        cmd.draw_indexed(6, 1, 0, 0, 0);

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

    void Renderer::upload_mesh(std::span<Vertex> vertices, std::span<u32> indices)
    {
        const usize vb_size = vertices.size() * sizeof(Vertex);
        const usize ib_size = indices.size() * sizeof(u32);

        m_mesh = std::make_unique<GPUMeshBuffers>();

        m_mesh->vertex_buffer = std::make_unique<Buffer>(m_device.get(), Buffer::Info {
            .size = vb_size,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memory = VMA_MEMORY_USAGE_GPU_ONLY
        });

        m_mesh->index_buffer = std::make_unique<Buffer>(m_device.get(), Buffer::Info {
            .size = ib_size,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memory = VMA_MEMORY_USAGE_GPU_ONLY
        });

        Buffer staging(m_device.get(), Buffer::Info {
            .size = vb_size + ib_size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .memory = VMA_MEMORY_USAGE_CPU_ONLY
        });

        void* data = staging.map();

        std::memcpy(data, vertices.data(), vb_size);
        std::memcpy(static_cast<std::byte*>(data) + vb_size, indices.data(), ib_size);

        staging.unmap();

        CommandPool pool(m_device.get(), m_device->graphics_family());

        auto cmd = pool.allocate();
        cmd.begin();

        cmd.copy_buffer(staging.buffer(), 0, m_mesh->vertex_buffer->buffer(), 0, vb_size);
        cmd.copy_buffer(staging.buffer(), vb_size, m_mesh->index_buffer->buffer(), 0, ib_size);

        auto barrier = BarrierBatch()
            .buffer(*m_mesh->vertex_buffer,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT, VK_ACCESS_2_MEMORY_READ_BIT
            )
            .buffer(*m_mesh->index_buffer,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, VK_ACCESS_2_MEMORY_READ_BIT
            );

        cmd.barrier(barrier);

        cmd.end();

        std::vector<VkCommandBufferSubmitInfo> cmds { cmd.submit_info() };
        m_graphics_queue->submit(cmds, {}, {});

        m_graphics_queue->wait_idle();
    }

}
