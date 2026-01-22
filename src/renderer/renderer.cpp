#include "renderer.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <pathconfig.inl>

#include "rhi/barrier.hpp"

namespace application {

    namespace {

        std::filesystem::path s_modelpath(pathconfig::model_directory);
    
    }

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

        m_camera = std::make_unique<Camera>(m_width, m_height, 0.001f, 1000.0f, dispatcher);

        if (auto model = load_gltf("sponza/main_sponza/NewSponza_Main_glTF_003.gltf")) {
            m_renderables.insert(m_renderables.begin(), model.value().begin(), model.value().end());
        }

        if (auto model = load_gltf("sponza/pkg_a_curtains/NewSponza_Curtains_glTF.gltf")) {
            m_renderables.insert(m_renderables.begin(), model.value().begin(), model.value().end());
        }

        if (auto model = load_gltf("sponza/pkg_b_ivy/NewSponza_IvyGrowth_glTF.gltf")) {
            m_renderables.insert(m_renderables.begin(), model.value().begin(), model.value().end());
        }

        // if (auto model = load_gltf("gearbox/gearbox.glb")) {
        //     m_renderables = model.value();
        // } else {
        //     std::println("model not loaded");
        // }

        m_storage_image = std::make_unique<Image>(m_device.get(), Image::Info {
            .extent = { m_width, m_height, 1 },
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .memory = VMA_MEMORY_USAGE_GPU_ONLY
        });

        m_depth_image = std::make_unique<Image>(m_device.get(), Image::Info {
            .extent = { m_width, m_height, 1 },
            .format = VK_FORMAT_D32_SFLOAT,
            .aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
            .usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .memory = VMA_MEMORY_USAGE_GPU_ONLY
        });

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
            .depth_format = m_depth_image->format(),
            .cull_mode = VK_CULL_MODE_NONE,
            .depth_test = true,
            .depth_write = true,
            .depth_compare = VK_COMPARE_OP_LESS
        });
    }

    Renderer::~Renderer()
    {
        m_device->wait_idle();
    }

    bool Renderer::begin_frame()
    {
        auto& frame = m_frames[m_frame_index % s_frames_in_flight];

        m_timeline->sync(frame.fence);

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

    void Renderer::draw(f32 dt)
    {
        auto& frame = m_frames[m_frame_index % s_frames_in_flight];

        auto cmd = frame.command_pool->allocate();

        cmd.begin();

        // --- GRAPHICS DRAW ---

        auto barrier = BarrierBatch()
            .image(*m_storage_image,
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_ASPECT_COLOR_BIT
            )
            .image(*m_depth_image,
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                VK_IMAGE_ASPECT_DEPTH_BIT
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
                    .color = { 0.01f, 0.01f, 0.01f, 1.0f }
                }
            }
        };

        VkRenderingAttachmentInfo depth_attachment {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = m_depth_image->view(),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {
                .depthStencil = {
                    .depth = 1.0f
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
            .pDepthAttachment = &depth_attachment,
            .pStencilAttachment = nullptr
        };

        cmd.begin_render(render_info);

        cmd.bind_pipeline(*m_mesh_pipeline);

        cmd.set_viewport(0.0f, 0.0f, static_cast<f32>(m_storage_image->width()), static_cast<f32>(m_storage_image->height()), 0.0f, 1.0f);
        cmd.set_scissor(0, 0, m_storage_image->width(), m_storage_image->height());

        m_camera->update(dt);

        for (const auto& renderable : m_renderables) {
            const auto& mesh = renderable.mesh;
            const auto& transform = renderable.transform;

            GPUDrawPushConstants constants {
                .camera = m_camera->shader_data(),
                .transform = transform,
                .vertex_buffer = mesh->buffers.vertex_buffer->address()
            };

            cmd.push_constants(*m_mesh_pipeline, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &constants);
            cmd.bind_index_buffer(mesh->buffers.index_buffer->buffer(), 0);

            for (const auto& surface : mesh->surfaces) {
                cmd.draw_indexed(surface.count, 1, surface.start_index, 0, 0);
            }
        }

        cmd.end_render();

        // --- BLIT TO SWAPCHAIN ---

        barrier = BarrierBatch()
            .image(m_swapchain->current_image(),
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_ASPECT_COLOR_BIT
            )
            .image(*m_storage_image,
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_IMAGE_ASPECT_COLOR_BIT
            );

        cmd.barrier(barrier);

        cmd.copy_image(m_storage_image->image(), m_storage_image->extent(), m_swapchain->current_image(), { m_swapchain->width(), m_swapchain->height(), 1 });

        // --- PRESENT ---

        barrier = BarrierBatch()
            .image(m_swapchain->current_image(),
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_ASPECT_COLOR_BIT
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

    GPUMeshBuffers Renderer::upload_mesh(std::span<Vertex> vertices, std::span<u32> indices)
    {
        const usize vb_size = vertices.size() * sizeof(Vertex);
        const usize ib_size = indices.size() * sizeof(u32);

        GPUMeshBuffers mesh;

        mesh.vertex_buffer = std::make_unique<Buffer>(m_device.get(), Buffer::Info {
            .size = vb_size,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memory = VMA_MEMORY_USAGE_GPU_ONLY
        });

        mesh.index_buffer = std::make_unique<Buffer>(m_device.get(), Buffer::Info {
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

        cmd.copy_buffer(staging.buffer(), 0, mesh.vertex_buffer->buffer(), 0, vb_size);
        cmd.copy_buffer(staging.buffer(), vb_size, mesh.index_buffer->buffer(), 0, ib_size);

        auto barrier = BarrierBatch()
            .buffer(*mesh.vertex_buffer,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT
            )
            .buffer(*mesh.index_buffer,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, VK_ACCESS_2_MEMORY_READ_BIT
            );

        cmd.barrier(barrier);

        cmd.end();

        std::vector<VkCommandBufferSubmitInfo> cmds { cmd.submit_info() };
        m_graphics_queue->submit(cmds, {}, {});

        m_graphics_queue->wait_idle();

        return mesh;
    }

    std::optional<std::vector<RenderObject>> Renderer::load_gltf(const std::string& filename)
    {
        std::println("loading gltf: {}", filename);

        auto model_path = s_modelpath / filename;

        fastgltf::Parser parser;
        constexpr auto gltf_options = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::LoadExternalBuffers;

        auto data = fastgltf::GltfDataBuffer::FromPath(model_path);
        if (data.error() != fastgltf::Error::None) {
            std::println("failed to load gltf data: {}", static_cast<i32>(data.error()));
            return std::nullopt;
        }

        auto asset = parser.loadGltf(data.get(), model_path.parent_path(), gltf_options);
        if (asset.error() != fastgltf::Error::None) {
            std::println("failed to parse gltf: {}", static_cast<i32>(asset.error()));
            return std::nullopt;
        }

        auto& gltf = asset.get();

        std::vector<std::shared_ptr<MeshAsset>> meshes;
        meshes.reserve(gltf.meshes.size());

        for (auto& mesh : gltf.meshes) {
            MeshAsset new_mesh;
            new_mesh.name = mesh.name;

            std::vector<Vertex> vertices;
            std::vector<u32> indices;

            for (auto& primitive : mesh.primitives) {
                GeometrySurface surface;
                surface.start_index = static_cast<u32>(indices.size());

                auto pos_it = primitive.findAttribute("POSITION");
                if (pos_it == primitive.attributes.end()) {
                    continue;
                }

                auto& pos_accessor = gltf.accessors[pos_it->accessorIndex];
                usize initial_vtx = vertices.size();
                vertices.resize(vertices.size() + pos_accessor.count);

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, pos_accessor, [&](glm::vec3 v, usize index) {
                    vertices[initial_vtx + index] = Vertex {
                        .position = v,
                        .normal = { 0.0f, 1.0f, 0.0f },
                        .uv = { 0.0f, 0.0f },
                        .color = { 1.0f, 0.0f, 1.0f, 1.0f }
                    };
                });

                auto normal_it = primitive.findAttribute("NORMAL");
                if (normal_it != primitive.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[normal_it->accessorIndex], [&](glm::vec3 v, usize index) {
                        vertices[initial_vtx + index].normal = v;
                    });
                }

                auto uv_it = primitive.findAttribute("TEXCOORD_0");
                if (uv_it != primitive.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[uv_it->accessorIndex], [&](glm::vec2 v, usize index) {
                        vertices[initial_vtx + index].uv = v;
                    });
                }

                auto color_it = primitive.findAttribute("COLOR_0");
                if (color_it != primitive.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[color_it->accessorIndex], [&](glm::vec4 v, usize index) {
                        vertices[initial_vtx + index].color = v;
                    });
                }

                if (primitive.indicesAccessor.has_value()) {
                    auto& accessor = gltf.accessors[primitive.indicesAccessor.value()];
                    surface.count = static_cast<u32>(accessor.count);
                    indices.reserve(indices.size() + accessor.count);

                    fastgltf::iterateAccessor<u32>(gltf, accessor, [&](u32 idx) {
                        indices.push_back(idx + static_cast<u32>(initial_vtx));
                    });
                } else {
                    surface.count = static_cast<u32>(pos_accessor.count);
                    for (size_t i = 0; i < pos_accessor.count; ++i) {
                        indices.push_back(static_cast<u32>(initial_vtx + i));
                    }
                }

                new_mesh.surfaces.push_back(surface);
            }

            if (vertices.empty()) continue;

            new_mesh.buffers = upload_mesh(vertices, indices);

            meshes.push_back(std::make_shared<MeshAsset>(std::move(new_mesh)));
        }

        std::println("loaded {} meshes", meshes.size());

        std::vector<RenderObject> renderables;

        std::function<void(const fastgltf::Node&, const glm::mat4&)> traverse_node;

        traverse_node = [&](const fastgltf::Node& node, const glm::mat4& parent_transform) {
            auto local = fastgltf::getLocalTransformMatrix(node);

            glm::mat4 local_transform = glm::make_mat4(local.data());
            glm::mat4 global_transform = parent_transform * local_transform;

            if (node.meshIndex.has_value()) {
                renderables.push_back(RenderObject {
                    .mesh = meshes[node.meshIndex.value()],
                    .transform = global_transform
                });
            }

            for (auto& children : node.children) {
                traverse_node(gltf.nodes[children], global_transform);
            }
        };

        if (auto scene_index = gltf.defaultScene) {
            auto& scene = gltf.scenes[scene_index.value()];

            glm::mat4 root(1.0f);

            for (auto node : scene.nodeIndices) {
                traverse_node(gltf.nodes[node], root);
            }
        }

        std::println(" - {} renderables", renderables.size());

        return renderables;
    }

}
