#pragma once

#include "events.hpp"

struct GLFWwindow;

namespace application {

    class Window
    {
        #ifdef __cpp_lib_move_only_function
            using EventCallback = std::move_only_function<void(const Event&)>;
        #else
            using EventCallback = std::function<void(const Event&)>;
        #endif

        friend class Application;
    public:
        Window(u32 width, u32 height, const std::string& title);
        ~Window();

        u32 width() const { return m_data.width; }
        u32 height() const { return m_data.height; }

        void bind_event_callback(EventCallback callback)
        {
            m_data.callback = std::forward<EventCallback>(callback);
        }

    protected:
        static void poll_events();

    private:
        struct WindowData
        {
            u32 width { 0 };
            u32 height { 0 };

            EventCallback callback { nullptr };
        };

    private:
        inline static std::atomic<usize> s_instance { 0 };

        GLFWwindow* m_window { nullptr };
        WindowData m_data;
    };

}
