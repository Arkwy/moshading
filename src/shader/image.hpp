#pragma once 

#include <string>
#include <imgui.h>

#include "src/vec.hpp"


struct Img {
    vec2 pos;
    vec2 size;
    std::string name;
    const std::string path;
};
