#pragma once

#include "core/window.hpp"
#include "core/events.hpp"

#include "rhi/context.hpp"
#include "rhi/device.hpp"

namespace application {

    class Renderer
    {
    public:
        Renderer(const Window& window, EventDispatcher& dispatcher);
        ~Renderer();

        void draw();

    private:
        u32 m_width { 0 };
        u32 m_height { 0 };

        std::unique_ptr<Context> m_context;
        std::unique_ptr<Device> m_device;
    };

}
