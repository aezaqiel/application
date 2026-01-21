#pragma once

#include "vktypes.hpp"
#include "device.hpp"

namespace application {

    class Shader
    {
    public:
        Shader(const Device* device, const std::string& filename, VkShaderStageFlagBits stage);
        ~Shader();

        VkShaderModule module() const { return m_module; }
        VkPipelineShaderStageCreateInfo stage_info() const;

    private:
        const Device* m_device;

        VkShaderModule m_module { VK_NULL_HANDLE };
        VkShaderStageFlagBits m_stage { VK_SHADER_STAGE_ALL };
    };

}
