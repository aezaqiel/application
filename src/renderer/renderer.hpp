#pragma once

#include "core/window.hpp"
#include "core/events.hpp"

#include "rhi/context.hpp"
#include "rhi/device.hpp"
#include "rhi/swapchain.hpp"
#include "rhi/command.hpp"

namespace application {

    class Renderer
    {
    public:
        Renderer(const Window& window, EventDispatcher& dispatcher);
        ~Renderer();

        void draw();

    private:
        inline static constexpr usize s_frames_in_flight { 2 };

        template <typename T>
        using PerFrame = std::array<T, s_frames_in_flight>;

    private:
        struct FrameData
        {
            std::unique_ptr<CommandPool> command_pool;
        };

    private:
        u32 m_width { 0 };
        u32 m_height { 0 };

        std::unique_ptr<Context> m_context;
        std::unique_ptr<Device> m_device;

        std::unique_ptr<Swapchain> m_swapchain;

        PerFrame<FrameData> m_frames;
    };

}
