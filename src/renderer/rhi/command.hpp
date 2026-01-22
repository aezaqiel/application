#pragma once

#include "vktypes.hpp"
#include "device.hpp"
#include "barrier.hpp"
#include "pipeline.hpp"

namespace application {

    class CommandList
    {
    public:
        CommandList(VkCommandBuffer cmd);
        ~CommandList() = default;

        CommandList(const CommandList&) = delete;
        CommandList& operator=(const CommandList&) = delete;

        VkCommandBuffer cmd() const { return m_cmd; }

        void begin() const;
        void end() const;

        VkCommandBuffer record(std::function<void(VkCommandBuffer)> task);

        VkCommandBufferSubmitInfo submit_info() const;

        void barrier(BarrierBatch& barrier) const;

        void clear_image(VkImage image, VkImageLayout layout, VkClearColorValue color, const std::vector<VkImageSubresourceRange>& ranges);

        void copy_image(VkImage src, VkExtent3D src_extent, VkImage dst, VkExtent3D dst_extent);
        void copy_buffer(VkBuffer src, VkDeviceSize src_offset, VkBuffer dst, VkDeviceSize dst_offset, VkDeviceSize size);

        void bind_pipeline(const ComputePipeline& pipeline);
        void bind_pipeline(const GraphicsPipeline& pipeline);

        void bind_set(const ComputePipeline& pipeline, std::span<VkDescriptorSet> sets, u32 first);
        void bind_set(const GraphicsPipeline& pipeline, std::span<VkDescriptorSet> sets, u32 first);

        void push_constants(const ComputePipeline& pipeline, u32 offset, u32 size, const void* data);
        void push_constants(const GraphicsPipeline& pipeline, VkShaderStageFlags stage, u32 offset, u32 size, const void* data);

        void bind_index_buffer(VkBuffer buffer, VkDeviceSize offset);

        void dispatch(u32 x, u32 y, u32 z);

        void begin_render(const VkRenderingInfo& info);
        void end_render();

        void set_viewport(f32 x, f32 y, f32 width, f32 height, f32 min_depth, f32 max_depth);
        void set_scissor(i32 x, i32 y, u32 width, u32 height);

        void draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance);
        void draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance);

    private:
        VkCommandBuffer m_cmd { VK_NULL_HANDLE };
    };

    class CommandPool
    {
    public:
        CommandPool(const Device* device, u32 queue_family);
        ~CommandPool();

        CommandPool(const CommandPool&) = delete;
        CommandPool& operator=(const CommandPool&) = delete;

        void reset(VkCommandPoolResetFlags flags = 0);

        CommandList allocate();

    private:
        const Device* m_device;

        VkCommandPool m_pool { VK_NULL_HANDLE };
    };

}
