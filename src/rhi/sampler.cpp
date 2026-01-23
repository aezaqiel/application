#include "sampler.hpp"

namespace application {

    Sampler::Sampler(const Device* device, const Info& info)
        : m_device(device)
    {
        VkSamplerCreateInfo sampler_info {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = info.mag_filter,
            .minFilter = info.min_filter,
            .mipmapMode = info.mipmap_mode,
            .addressModeU = info.address_mode_u,
            .addressModeV = info.address_mode_v,
            .addressModeW = info.address_mode_w,
            .mipLodBias = info.mip_lod_bias,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = device->props().limits.maxSamplerAnisotropy,
            .compareEnable = info.compare ? VK_TRUE : VK_FALSE,
            .compareOp = info.compare_op,
            .minLod = info.min_lod,
            .maxLod = info.max_lod,
            .borderColor = info.border_color,
            .unnormalizedCoordinates = info.unnormalized_coords ? VK_TRUE : VK_FALSE
        };

        VK_CHECK(vkCreateSampler(m_device->device(), &sampler_info, nullptr, &m_sampler));
    }

    Sampler::~Sampler()
    {
        vkDestroySampler(m_device->device(), m_sampler, nullptr);
    }

}
