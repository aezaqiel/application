#include "camera.hpp"

#include "core/input.hpp"

namespace application {

    Camera::Camera(u32 width, u32 height, f32 near, f32 far, EventDispatcher& dispatcher)
        : m_aspect(static_cast<f32>(width) / static_cast<f32>(height)), m_near(near), m_far(far)
    {
        dispatcher.subscribe<WindowResizedEvent>([&](const WindowResizedEvent& e) -> bool {
            m_aspect = static_cast<f32>(e.width) / static_cast<f32>(e.height);
            return false;
        });
    }

    void Camera::update(f32 dt)
    {
        // UPDATE STATE

        glm::vec3 input_dir(0.0f);

        f32 speed = m_move_speed;
        if (Input::key_down(KeyCode::LeftShift)) speed *= m_move_boost;

        if (Input::key_down(KeyCode::W)) input_dir.z -= 1.0f;
        if (Input::key_down(KeyCode::S)) input_dir.z += 1.0f;
        if (Input::key_down(KeyCode::A)) input_dir.x -= 1.0f;
        if (Input::key_down(KeyCode::D)) input_dir.x += 1.0f;
        if (Input::key_down(KeyCode::E)) input_dir.y += 1.0f;
        if (Input::key_down(KeyCode::Q)) input_dir.y -= 1.0f;

        if (glm::length(input_dir) > 0.0f) input_dir = glm::normalize(input_dir);

        if (Input::mouse_button_pressed(MouseButton::Right)) {
            m_last_mouse_pos = { Input::mouse_x(), Input::mouse_y() };
        } else if (Input::mouse_button_held(MouseButton::Right)) {
            glm::vec2 current_pos = { Input::mouse_x(), Input::mouse_y() };
            glm::vec2 delta = current_pos - m_last_mouse_pos;
            m_last_mouse_pos = current_pos;

            m_yaw -= delta.x * m_rotation_speed;
            m_pitch -= delta.y * m_rotation_speed;

            m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
        }

        glm::vec3 targetVel = input_dir * speed;
        m_velocity = glm::mix(m_velocity, targetVel, dt / m_damping);

        glm::quat q_pitch = glm::angleAxis(glm::radians(m_pitch), glm::vec3(1, 0, 0));
        glm::quat q_yaw = glm::angleAxis(glm::radians(m_yaw), glm::vec3(0, 1, 0));
        glm::quat orientation = q_yaw * q_pitch;

        glm::vec3 forward = orientation * glm::vec3(0, 0, 1);
        glm::vec3 right = orientation * glm::vec3(1, 0, 0);
        glm::vec3 up = glm::vec3(0, 1, 0);

        m_current_pos += (right * m_velocity.x) * dt;
        m_current_pos += (up * m_velocity.y) * dt;
        m_current_pos += (forward * m_velocity.z) * dt;

        m_state.position = m_current_pos;
        m_state.rotation = orientation;

        // UPDATE DATA

        m_data.view = glm::translate(glm::mat4_cast(glm::conjugate(m_state.rotation)), -m_state.position);
        m_data.proj = glm::perspective(glm::radians(m_state.vfov), m_aspect, m_near, m_far);
        m_data.proj[1][1] *= -1.0f;
        m_data.position = glm::vec4(m_state.position, 1.0f);
        m_data.params = glm::vec4(m_state.vfov, m_aspect, m_near, m_far);
    }

}
