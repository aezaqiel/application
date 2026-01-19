#include "window.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace application {

    Window::Window(u32 width, u32 height, const std::string& title)
    {
        if (s_instance.fetch_add(1, std::memory_order_relaxed) == 0) {
            glfwSetErrorCallback([](i32 code, const char* desc) {
                std::println("glfw error ({}): {}", code, desc);
            });

            glfwInit();
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(static_cast<i32>(width), static_cast<i32>(height), title.c_str(), nullptr, nullptr);

        {
            i32 w;
            i32 h;
            glfwGetFramebufferSize(m_window, &w, &h);

            m_data.width = static_cast<u32>(w);
            m_data.height = static_cast<u32>(h);
        }

        glfwSetWindowUserPointer(m_window, &m_data);

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            if (data.callback) {
                data.callback(WindowClosedEvent());
            }
        });

        glfwSetWindowIconifyCallback(m_window, [](GLFWwindow* window, i32 iconify) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            if (data.callback) {
                data.callback(WindowMinimizedEvent(static_cast<bool>(iconify)));
            }
        });

        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, i32 width, i32 height) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.width = static_cast<u32>(width);
            data.height = static_cast<u32>(height);

            if (data.callback) {
                data.callback(WindowResizedEvent(data.width, data.height));
            }
        });
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_window);

        if (s_instance.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            glfwTerminate();
        }
    }

    void Window::poll_events()
    {
        glfwPollEvents();
    }

}
