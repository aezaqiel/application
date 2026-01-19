#pragma once

#include "vktypes.hpp"
#include "core/window.hpp"

struct GLFWwindow;

namespace application {

    class Context
    {
    public:
        Context(const Window& window);
        ~Context();

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;

        VkInstance instance() const { return m_instance; }
        VkSurfaceKHR surface()  const { return m_surface; }

    private:
        VkInstance m_instance { VK_NULL_HANDLE };
        VkDebugUtilsMessengerEXT m_messenger { VK_NULL_HANDLE };

        VkSurfaceKHR m_surface { VK_NULL_HANDLE };
    };

}
