#include "input.hpp"

namespace application {

    bool Input::key_pressed(KeyCode key)
    {
        const auto index = static_cast<usize>(key);
        if (!s_instance || index >= s_instance->m_key_data.size()) return false;

        const auto state = s_instance->m_key_data[index].state;
        return state == KeyState::Pressed;
    }

    bool Input::key_held(KeyCode key)
    {
        const auto index = static_cast<usize>(key);
        if (!s_instance || index >= s_instance->m_key_data.size()) return false;

        const auto state = s_instance->m_key_data[index].state;
        return state == KeyState::Held;
    }

    bool Input::key_down(KeyCode key)
    {
        const auto index = static_cast<usize>(key);
        if (!s_instance || index >= s_instance->m_key_data.size()) return false;

        const auto state = s_instance->m_key_data[index].state;
        return state == KeyState::Pressed || state == KeyState::Held;
    }

    bool Input::key_released(KeyCode key)
    {
        const auto index = static_cast<usize>(key);
        if (!s_instance || index >= s_instance->m_key_data.size()) return false;

        const auto state = s_instance->m_key_data[index].state;
        return state == KeyState::Released;
    }

    bool Input::mouse_button_pressed(MouseButton button)
    {
        const auto index = static_cast<usize>(button);
        if (!s_instance || index >= s_instance->m_button_data.size()) return false;

        const auto state = s_instance->m_button_data[index].state;
        return state == KeyState::Pressed;
    }

    bool Input::mouse_button_held(MouseButton button)
    {
        const auto index = static_cast<usize>(button);
        if (!s_instance || index >= s_instance->m_button_data.size()) return false;

        const auto state = s_instance->m_button_data[index].state;
        return state == KeyState::Held;
    }

    bool Input::mouse_button_down(MouseButton button)
    {
        const auto index = static_cast<usize>(button);
        if (!s_instance || index >= s_instance->m_button_data.size()) return false;

        const auto state = s_instance->m_button_data[index].state;
        return state == KeyState::Pressed || state == KeyState::Held;
    }

    bool Input::mouse_button_released(MouseButton button)
    {
        const auto index = static_cast<usize>(button);
        if (!s_instance || index >= s_instance->m_button_data.size()) return false;

        const auto state = s_instance->m_button_data[index].state;
        return state == KeyState::Released;
    }

    f32 Input::mouse_x()
    {
        if (!s_instance) return 0.0f;
        return s_instance->m_mouse_x;
    }

    f32 Input::mouse_y()
    {
        if (!s_instance) return 0.0f;
        return s_instance->m_mouse_y;
    }

    std::pair<f32, f32> Input::mouse_position()
    {
        if (!s_instance) return std::make_pair(0.0f, 0.0f);
        return std::make_pair(s_instance->m_mouse_x, s_instance->m_mouse_y);
    }

    Input::Input(EventDispatcher& dispatcher)
    {
        s_instance = this;

        m_key_data.fill({});
        m_button_data.fill({});

        dispatcher.subscribe<KeyPressedEvent>([this](const KeyPressedEvent& e) -> bool {
            if (e.repeat) return false;

            const auto code = static_cast<usize>(e.keycode);
            if (code < m_key_data.size()) {
                m_key_data[code].state = KeyState::Pressed;
            }

            return false;
        });

        dispatcher.subscribe<KeyReleasedEvent>([this](const KeyReleasedEvent& e) -> bool {
            const auto code = static_cast<usize>(e.keycode);
            if (code < m_key_data.size()) {
                m_key_data[code].state = KeyState::Released;
            }

            return false;
        });

        dispatcher.subscribe<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& e) -> bool {
            const auto code = static_cast<usize>(e.button);
            if (code < m_button_data.size()) {
                m_button_data[code].state = KeyState::Pressed;
            }

            return false;
        });

        dispatcher.subscribe<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& e) -> bool {
            const auto code = static_cast<usize>(e.button);
            if (code < m_button_data.size()) {
                m_button_data[code].state = KeyState::Released;
            }

            return false;
        });

        dispatcher.subscribe<MouseMovedEvent>([this](const MouseMovedEvent& e) -> bool {
            m_mouse_x = e.x;
            m_mouse_y = e.y;

            return false;
        });
    }

    Input::~Input()
    {
        if (s_instance == this) s_instance = nullptr;
    }

    void Input::update()
    {
        for (auto& data : m_key_data) {
            data.old_state = data.state;
            if (data.state == KeyState::Pressed) {
                data.state = KeyState::Held;
            } else if (data.state == KeyState::Released) {
                data.state = KeyState::None;
            }
        }

        for (auto& data : m_button_data) {
            data.old_state = data.state;
            if (data.state == KeyState::Pressed) {
                data.state = KeyState::Held;
            } else if (data.state == KeyState::Released) {
                data.state = KeyState::None;
            }
        }
    }

}
