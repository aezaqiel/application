#pragma once

#include "vktypes.hpp"
#include "device.hpp"

namespace application {

    class Sampler
    {
    public:
        struct Info
        {
            VkFilter min_filter { VK_FILTER_LINEAR };
            VkFilter mag_filter { VK_FILTER_LINEAR };
            VkSamplerMipmapMode mipmap_mode { VK_SAMPLER_MIPMAP_MODE_LINEAR };
            VkSamplerAddressMode address_mode_u { VK_SAMPLER_ADDRESS_MODE_REPEAT };
            VkSamplerAddressMode address_mode_v { VK_SAMPLER_ADDRESS_MODE_REPEAT };
            VkSamplerAddressMode address_mode_w { VK_SAMPLER_ADDRESS_MODE_REPEAT };
            f32 mip_lod_bias { 0.0f };
            bool compare { false };
            VkCompareOp compare_op { VK_COMPARE_OP_ALWAYS };
            f32 min_lod { 0.0f };
            f32 max_lod { 1.0f };
            VkBorderColor border_color { VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK };
            bool unnormalized_coords { false };
        };

    public:
        Sampler(const Device* device, const Info& info);
        ~Sampler();

        VkSampler sampler() const { return m_sampler; }

    private:
        const Device* m_device { nullptr };

        VkSampler m_sampler { VK_NULL_HANDLE };
    };

}
