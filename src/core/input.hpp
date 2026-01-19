#pragma once

#include "events.hpp"

namespace application {

    class Input
    {
        friend class Application;
    public:
        Input(EventDispatcher& dispatcher);
        ~Input();

        void update();

        static bool key_pressed(KeyCode key);
        static bool key_held(KeyCode key);
        static bool key_down(KeyCode key);
        static bool key_released(KeyCode key);

        static bool mouse_button_pressed(MouseButton button);
        static bool mouse_button_held(MouseButton button);
        static bool mouse_button_down(MouseButton button);
        static bool mouse_button_released(MouseButton button);

        static f32 mouse_x();
        static f32 mouse_y();
        static std::pair<f32, f32> mouse_position();

    private:
        struct InputState
        {
            KeyState state { KeyState::None };
            KeyState old_state { KeyState::None };
        };

    private:
        inline static Input* s_instance { nullptr };

        std::array<InputState, 512> m_key_data;
        std::array<InputState, 16> m_button_data;

        f32 m_mouse_x { 0.0f };
        f32 m_mouse_y { 0.0f };
    };

}
