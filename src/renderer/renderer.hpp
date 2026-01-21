#pragma once

#include "core/window.hpp"
#include "core/events.hpp"

#include "deletion_queue.hpp"

#include "rhi/context.hpp"
#include "rhi/device.hpp"
#include "rhi/swapchain.hpp"
#include "rhi/queue.hpp"
#include "rhi/command.hpp"
#include "rhi/semaphores.hpp"
#include "rhi/image.hpp"
#include "rhi/descriptor.hpp"
#include "rhi/pipeline.hpp"

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
        inline static constexpr usize s_frames_in_flight { 2 };

        template <typename T>
        using PerFrame = std::array<T, s_frames_in_flight>;

    private:
        struct FrameData
        {
            DeletionQueue gc;

            u64 fence { 0 };
            std::unique_ptr<CommandPool> command_pool;

            std::unique_ptr<DescriptorAllocator> descriptor_allocator;
            VkDescriptorSet compute_descriptor;
        };

    private:
        u32 m_width { 0 };
        u32 m_height { 0 };

        DeletionQueue m_gc;

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

        std::unique_ptr<DescriptorLayout> m_compute_layout;
        std::unique_ptr<ComputePipeline> m_compute_pipeline;

        std::unique_ptr<DescriptorLayout> m_triangle_layout;
        std::unique_ptr<GraphicsPipeline> m_triangle_pipeline;
    };

}
