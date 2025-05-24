#ifndef STATE_H
#define STATE_H

#include <vector>
#include <imgui.h>

#include "image.hpp"

struct State {

    std::vector<Img> images;

    State(): images(0) {
        
    }
};

#endif
