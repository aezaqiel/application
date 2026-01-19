cmake_minimum_required(VERSION 3.10)

project(application C CXX)

find_package(Vulkan REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/volk)
add_subdirectory(thirdparty/vma)

add_executable(${PROJECT_NAME}
    src/main.cpp

    src/core/types.hpp
    src/core/keycodes.hpp
    src/core/events.hpp
    src/core/window.hpp
    src/core/window.cpp
    src/core/input.hpp
    src/core/input.cpp
    src/core/application.hpp
    src/core/application.cpp

    src/renderer/renderer.hpp
    src/renderer/renderer.cpp

    src/platform/vma_impl.cpp
)

target_include_directories(${PROJECT_NAME}
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    glfw
    volk
    VulkanMemoryAllocator
)

target_compile_definitions(${PROJECT_NAME}
PRIVATE
    GLFW_INCLUDE_NONE
)

target_precompile_headers(${PROJECT_NAME}
PRIVATE
    src/pch.hpp
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        NOMINMAX
        GLFW_EXPOSE_NATIVE_WIN32
    )
endif()

if(NOT MSVC)
    target_compile_options(${PROJECT_NAME}
    PRIVATE
        -fno-rtti
        -fno-exceptions
        -Wno-nullability-completeness
    )
else()
    target_compile_options(${PROJECT_NAME}
    PRIVATE
        /GR-
        /D_HAS_EXCEPTIONS=0
    )
endif()
