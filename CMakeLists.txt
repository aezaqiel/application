cmake_minimum_required(VERSION 3.10)

project(application C CXX)

find_package(Vulkan REQUIRED)
find_program(SLANG_COMPILER NAMES slangc REQUIRED)

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
add_subdirectory(thirdparty/glm)

set(SHADER_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)
set(SHADER_SOURCES
    "gradient.slang"
    "vertex.slang"
    "fragment.slang"
)

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
    src/renderer/deletion_queue.hpp
    src/renderer/deletion_queue.cpp

    src/renderer/rhi/vktypes.hpp
    src/renderer/rhi/context.hpp
    src/renderer/rhi/context.cpp
    src/renderer/rhi/device.hpp
    src/renderer/rhi/device.cpp
    src/renderer/rhi/swapchain.hpp
    src/renderer/rhi/swapchain.cpp
    src/renderer/rhi/command.hpp
    src/renderer/rhi/command.cpp
    src/renderer/rhi/queue.hpp
    src/renderer/rhi/queue.cpp
    src/renderer/rhi/semaphores.hpp
    src/renderer/rhi/semaphores.cpp
    src/renderer/rhi/barrier.hpp
    src/renderer/rhi/barrier.cpp
    src/renderer/rhi/image.hpp
    src/renderer/rhi/image.cpp
    src/renderer/rhi/buffer.hpp
    src/renderer/rhi/buffer.cpp
    src/renderer/rhi/descriptor.hpp
    src/renderer/rhi/descriptor.cpp
    src/renderer/rhi/shader.hpp
    src/renderer/rhi/shader.cpp
    src/renderer/rhi/pipeline.hpp
    src/renderer/rhi/pipeline.cpp

    src/platform/vma_impl.cpp
)

target_include_directories(${PROJECT_NAME}
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    glfw
    volk
    VulkanMemoryAllocator
    glm
)

target_compile_definitions(${PROJECT_NAME}
PRIVATE
    GLFW_INCLUDE_NONE
    GLM_ENABLE_EXPERIMENTAL
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

function(target_add_slang_shader target_name)
    set(options)
    set(oneValueArgs FILE ENTRY_POINT PROFILE OUTPUT_NAME)
    set(multiValueArgs DEFINES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # 1. Validate Input
    if(NOT ARG_FILE)
        message(FATAL_ERROR "No FILE specified for slang shader compilation.")
    endif()

    # 2. Setup Paths
    set(SHADER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${ARG_FILE}")
    
    # Auto-generate output name if not provided (e.g., "myshader.slang" -> "myshader.spv")
    if(NOT ARG_OUTPUT_NAME)
        get_filename_component(FILE_NAME ${ARG_FILE} NAME_WE)
        set(ARG_OUTPUT_NAME "${FILE_NAME}.spv")
    endif()
    
    set(SHADER_OUTPUT "${SHADER_DIRECTORY}/${ARG_OUTPUT_NAME}")
    set(DEP_FILE "${SHADER_OUTPUT}.d")

    # 3. Construct Flags
    set(FLAGS "")
    
    if(ARG_ENTRY_POINT)
        list(APPEND FLAGS -entry ${ARG_ENTRY_POINT})
    endif()

    if(ARG_PROFILE)
        list(APPEND FLAGS -profile ${ARG_PROFILE})
    endif()

    foreach(DEF ${ARG_DEFINES})
        list(APPEND FLAGS -D${DEF})
    endforeach()

    # 4. Define the Command
    # We use ${CMAKE_COMMAND} -E make_directory to ensure the output folder exists
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_DIRECTORY}"
        COMMAND ${SLANG_COMPILER}
            ${SHADER_SOURCE}
            -target spirv
            -o ${SHADER_OUTPUT}
            -depfile ${DEP_FILE}
            ${FLAGS}
            # Debug/Release flags from your original code 
            $<IF:$<CONFIG:Debug>,-g,-O3>
        DEPENDS ${SHADER_SOURCE}
        DEPFILE ${DEP_FILE}
        COMMENT "Compiling Slang shader ${ARG_FILE} -> ${ARG_OUTPUT_NAME}"
    )

    # 5. Attach to Target
    target_sources(${target_name} PRIVATE ${SHADER_OUTPUT})
endfunction()

foreach(SRC ${SHADER_SOURCES})
    target_add_slang_shader(${PROJECT_NAME} 
        FILE ${SRC}
    )
endforeach()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/pathconfig.inl
    ${CMAKE_CURRENT_BINARY_DIR}/generated/pathconfig.inl
    @ONLY
)
