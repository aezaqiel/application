#pragma once

#include <string_view>

namespace pathconfig {

    inline static constexpr std::string_view shader_directory = "@SHADER_DIRECTORY@";
    inline static constexpr std::string_view model_directory = "@MODEL_DIRECTORY@";

}
