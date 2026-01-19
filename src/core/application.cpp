#include "application.hpp"

namespace application {

    Application::Application()
    {
        m_dispatcher = std::make_unique<EventDispatcher>();

        m_dispatcher->subscribe<WindowClosedEvent>([this](const WindowClosedEvent& e) -> bool {
            m_running = false;
            return true;
        });

        m_dispatcher->subscribe<WindowMinimizedEvent>([this](const WindowMinimizedEvent& e) -> bool {
            m_minimized = e.minimized;
            return false;
        });

        m_window = std::make_unique<Window>(1280, 720, "application");
        m_window->bind_event_callback([this](const Event& event) constexpr {
            dispatch_events(event);
        });
    }

    void Application::run()
    {
        while (m_running) {
            Window::poll_events();

            if (!m_minimized) {
            }
        }
    }

    void Application::dispatch_events(const Event& event)
    {
        // TODO: event queue?
        m_dispatcher->dispatch(event);
    }

}
