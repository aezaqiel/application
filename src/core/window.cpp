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

        glfwSetKeyCallback(m_window, [](GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data.callback) return;

            switch (action) {
                case GLFW_PRESS: {
                    data.callback(KeyPressedEvent(static_cast<KeyCode>(key), false));
                } break;
                case GLFW_RELEASE: {
                    data.callback(KeyReleasedEvent(static_cast<KeyCode>(key)));
                } break;
                case GLFW_REPEAT: {
                    data.callback(KeyPressedEvent(static_cast<KeyCode>(key), true));
                } break;
                default: std::println("unknown key action ({})", action); return;
            }
        });

        glfwSetCharCallback(m_window, [](GLFWwindow* window, u32 codepoint) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            if (data.callback) {
                data.callback(KeyTypedEvent(codepoint));
            }
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, i32 button, i32 action, i32 mods) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (!data.callback) return;

            switch (action) {
                case GLFW_PRESS: {
                    data.callback(MouseButtonPressedEvent(static_cast<MouseButton>(button)));
                } break;
                case GLFW_RELEASE: {
                    data.callback(MouseButtonReleasedEvent(static_cast<MouseButton>(button)));
                } break;
                default: std::println("unknown mouse button action ({})", action); return;
            }
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, f64 x, f64 y) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            if (data.callback) {
                data.callback(MouseMovedEvent(static_cast<f32>(x), static_cast<f32>(y)));
            }
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow* window, f64 x, f64 y) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            if (data.callback) {
                data.callback(MouseScrolledEvent(static_cast<f32>(x), static_cast<f32>(y)));
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
