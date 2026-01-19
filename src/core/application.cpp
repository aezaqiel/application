#include "application.hpp"

namespace application {

    Application::Application()
    {
        m_dispatcher = std::make_unique<EventDispatcher>();

        m_window = std::make_unique<Window>(1280, 720, "application");
        m_window->bind_event_callback([this](const Event& event) constexpr {
            dispatch_events(event);
        });

        m_input = std::make_unique<Input>(*m_dispatcher);

        m_dispatcher->subscribe<WindowClosedEvent>([this](const WindowClosedEvent& e) -> bool {
            m_running = false;
            return true;
        });

        m_dispatcher->subscribe<WindowMinimizedEvent>([this](const WindowMinimizedEvent& e) -> bool {
            m_minimized = e.minimized;
            return false;
        });
    }

    void Application::run()
    {
        while (m_running) {
            m_input->update();
            Window::poll_events();

            if (Input::key_down(KeyCode::Escape)) m_running = false;

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
