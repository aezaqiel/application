#pragma once

#include "events.hpp"
#include "window.hpp"
#include "input.hpp"

#include "renderer/renderer.hpp"

namespace application {

    class Application
    {
    public:
        Application();
        ~Application() = default;

        void run();
    
    private:
        void dispatch_events(const Event& event);

    private:
        bool m_running { true };
        bool m_minimized { false };

        std::unique_ptr<EventDispatcher> m_dispatcher;
        std::unique_ptr<Window> m_window;
        std::unique_ptr<Input> m_input;

        std::unique_ptr<Renderer> m_renderer;
    };

}
