#pragma once

#include <volk.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#define VK_CHECK(expr) do {                                      \
    VkResult result_ = (expr);                                   \
    if (result_ != VK_SUCCESS) {                                 \
        std::println("VK_CHECK failed: {} at {}:{} returned {}", \
            #expr,                                               \
            __FILE__,                                            \
            __LINE__,                                            \
            string_VkResult(result_));                           \
    }                                                            \
} while (false)
