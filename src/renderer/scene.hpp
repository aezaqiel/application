#pragma once

#include <glm/glm.hpp>

namespace application {

    struct SceneData
    {
        glm::vec4 ambient_color;
        glm::vec4 sunlight_direction; // [direction: vec3, power: f32]
        glm::vec4 sunlight_color;
    };

}
