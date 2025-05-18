#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <imgui.h>

#include "vec.hpp"


struct Img {
    vec2 pos;
    vec2 size;
    std::string name;

private:
    const std::string path;
};




#endif
