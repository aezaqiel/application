#pragma once

#include "core/window.hpp"
#include "core/events.hpp"

#include "rhi/context.hpp"
#include "rhi/device.hpp"
#include "rhi/swapchain.hpp"
#include "rhi/queue.hpp"
#include "rhi/command.hpp"
#include "rhi/semaphores.hpp"
#include "rhi/image.hpp"
#include "rhi/descriptor.hpp"
#include "rhi/pipeline.hpp"

#include "mesh.hpp"

namespace application {

    class Renderer
    {
    public:
        Renderer(const Window& window, EventDispatcher& dispatcher);
        ~Renderer();

        bool begin_frame();
        void end_frame();
        void draw();

    private:
        GPUMeshBuffers upload_mesh(std::span<Vertex> vertices, std::span<u32> indices);
        std::optional<std::vector<RenderObject>> load_gltf(const std::string& filename);

    private:
        inline static constexpr usize s_frames_in_flight { 2 };

        template <typename T>
        using PerFrame = std::array<T, s_frames_in_flight>;

    private:
        struct FrameData
        {
            u64 fence { 0 };
            std::unique_ptr<CommandPool> command_pool;

            std::unique_ptr<DescriptorAllocator> descriptor_allocator;
            VkDescriptorSet mesh_descriptor;
        };

    private:
        u32 m_width { 0 };
        u32 m_height { 0 };

        std::unique_ptr<Context> m_context;
        std::unique_ptr<Device> m_device;

        std::unique_ptr<Swapchain> m_swapchain;

        std::unique_ptr<Queue> m_graphics_queue;
        std::unique_ptr<Queue> m_compute_queue;
        std::unique_ptr<Queue> m_transfer_queue;

        PerFrame<FrameData> m_frames;

        std::unique_ptr<TimelineSemaphore> m_timeline;
        u64 m_frame_index { 0 };

        std::unique_ptr<Image> m_storage_image;
        std::unique_ptr<Image> m_depth_image;

        // std::vector<std::shared_ptr<MeshAsset>> m_meshes;
        std::vector<RenderObject> m_renderables;

        std::unique_ptr<DescriptorLayout> m_mesh_layouts;
        std::unique_ptr<GraphicsPipeline> m_mesh_pipeline;
    };

}
