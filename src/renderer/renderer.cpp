#include "renderer.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace application {

    Renderer::Renderer(const Window& window, EventDispatcher& dispatcher)
        : m_width(window.width()), m_height(window.height())
    {
        m_context = std::make_unique<Context>(window);
        m_device = std::make_unique<Device>(*m_context);

        m_swapchain = std::make_unique<Swapchain>(*m_context, *m_device);

        for (auto& frame : m_frames) {
            frame.command_pool = std::make_unique<CommandPool>(*m_device, m_device->graphics_family());
        }
    }

    Renderer::~Renderer()
    {
        m_device->wait_idle();
    }

    void Renderer::draw()
    {
    }

}
