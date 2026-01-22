#pragma once

#include <glm/glm.hpp>

#include "rhi/buffer.hpp"

namespace application {

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec4 color;
    };

    struct GPUMeshBuffers
    {
        std::unique_ptr<Buffer> index_buffer;
        std::unique_ptr<Buffer> vertex_buffer;
    };

    struct GPUDrawPushConstants
    {
        glm::mat4 world_transform { 1.0f };
        VkDeviceAddress vertex_buffer;
    };

    struct GeometrySurface
    {
        u32 start_index;
        u32 count;
    };

    struct MeshAsset
    {
        std::string name;

        std::vector<GeometrySurface> surfaces;
        GPUMeshBuffers buffers;
    };

}
