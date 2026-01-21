#pragma once

#include "vktypes.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "image.hpp"
// #include "acceleration_structure.hpp"

namespace application {

    class DescriptorLayout
    {
    public:
        struct Binding
        {
            u32 binding;
            VkDescriptorType type;
            VkShaderStageFlags stage;
            u32 count { 1 };
        };

        class Builder
        {
        public:
            Builder(const Device* device);

            Builder& add_binding(u32 binding, VkDescriptorType type, VkShaderStageFlags stage, u32 count = 1);
            std::unique_ptr<DescriptorLayout> build();

        private:
            const Device* m_device;
            std::vector<Binding> m_bindings;
        };

    public:
        DescriptorLayout(const Device* device, std::span<Binding> bindings);
        ~DescriptorLayout();

        VkDescriptorSetLayout  layout() const { return m_layout; }

    private:
        const Device* m_device;
        VkDescriptorSetLayout m_layout { VK_NULL_HANDLE };
    };

    class DescriptorAllocator
    {
    public:
        struct PoolSizeRatio
        {
            VkDescriptorType type;
            f32 ratio;
        };

    public:
        DescriptorAllocator(const Device* device, u32 max_sets = 1024, std::span<PoolSizeRatio> pool_ratios = {});
        ~DescriptorAllocator();

        VkDescriptorSet allocate(const DescriptorLayout& layout);
        void reset();

    private:
        VkDescriptorPool get_pool();
        VkDescriptorPool create_pool(u32 count, VkDescriptorPoolCreateFlags flags);

    private:
        const Device* m_device;

        std::vector<PoolSizeRatio> m_ratios;

        VkDescriptorPool m_current_pool { VK_NULL_HANDLE };
        std::vector<VkDescriptorPool> m_used_pools;
        std::vector<VkDescriptorPool> m_free_pools;

        u32 m_sets_per_pool { 0 };
    };

    class DescriptorWriter
    {
    public:
        DescriptorWriter(const Device* device);
        ~DescriptorWriter() = default;

        DescriptorWriter& write_buffer(u32 binding, const Buffer& buffer, u64 offset = 0, u64 range = VK_WHOLE_SIZE, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        DescriptorWriter& write_image(u32 binding, const Image& image, VkSampler sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        DescriptorWriter& write_storage_image(u32 binding, const Image& image, VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);
        // auto write_as(u32 binding, const AccelerationStructure& as) -> DescriptorWriter&;

        void update(VkDescriptorSet set);
        void push(VkCommandBuffer cmd, VkPipelineBindPoint bind, VkPipelineLayout layout);

        void clear();

    private:
        const Device* m_device;

        std::vector<VkWriteDescriptorSet> m_writes;

        std::deque<VkDescriptorBufferInfo> m_buffers;
        std::deque<VkDescriptorImageInfo> m_images;
        std::deque<VkAccelerationStructureKHR> m_accelerations;
        std::deque<VkWriteDescriptorSetAccelerationStructureKHR> m_acceleration_infos;
    };

}
