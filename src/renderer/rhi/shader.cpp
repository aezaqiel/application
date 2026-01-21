#include "shader.hpp"

#include <pathconfig.inl>

namespace application {

    namespace {

        std::filesystem::path s_shaderpath(pathconfig::shader_directory);

        std::vector<char> read_shader(const std::filesystem::path& filepath)
        {
            std::ifstream file(filepath, std::ios::ate | std::ios::binary);

            if (!file.is_open()) {
                std::println("failed to open shader: {}", filepath.string());
                return {};
            }

            usize size = file.tellg();

            std::vector<char> buffer(size);

            file.seekg(SEEK_SET);

            file.read(buffer.data(), size);

            file.close();

            return buffer;
        }

    }

    Shader::Shader(const Device* device, const std::string& filename, VkShaderStageFlagBits stage)
        : m_device(device), m_stage(stage)
    {
        auto src = read_shader(s_shaderpath / filename);

        VkShaderModuleCreateInfo module_info {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = src.size(),
            .pCode = reinterpret_cast<const u32*>(src.data())
        };

        VK_CHECK(vkCreateShaderModule(device->device(), &module_info, nullptr, &m_module));

        std::println("loaded shader: {}", filename);
    }

    Shader::~Shader()
    {
        vkDestroyShaderModule(m_device->device(), m_module, nullptr);
    }

    VkPipelineShaderStageCreateInfo Shader::stage_info() const
    {
        return VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = m_stage,
            .module = m_module,
            .pName = "main",
            .pSpecializationInfo = nullptr
        };
    }

}
