#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "core/events.hpp"

namespace application {

    class Camera
    {
    public:
        struct Data
        {
            glm::mat4 view;
            glm::mat4 proj;
            glm::vec3 position;
            glm::vec4 params; // [vfov, ar, near, far]
        };

    public:
        Camera(u32 width, u32 height, f32 near, f32 far, EventDispatcher& dispatcher);

        Data shader_data() const { return m_data; }

        void update(f32 dt);

    private:
        struct State
        {
            glm::vec3 position { 0.0f };
            glm::quat rotation { 1.0f, 0.0f, 0.0f, 0.0f };

            f32 vfov { 45.0f };
            f32 focal_distance { 10.0f };
            f32 aperture { 16.0f };
        };

    private:
        f32 m_aspect { 1.0f };
        f32 m_near { 0.001f };
        f32 m_far { 1000.0f };

        Data m_data;
        State m_state;

        f32 m_move_speed { 5.0f };
        f32 m_rotation_speed { 0.2f };
        f32 m_move_boost { 4.0f };
        f32 m_damping { 0.2f };

        glm::vec3 m_velocity { 0.0f };
        glm::vec2 m_rotation_velocity { 0.0f };

        glm::vec2 m_last_mouse_pos { 0.0f };

        glm::vec3 m_current_pos { -12.0f, 1.8f, 0.0f };
        f32 m_pitch { 0.0f };
        f32 m_yaw { -90.0f };
    };

}
