#pragma once

// #include <memory>
// #include <vector>

// #include "image.hpp"
#include "shader/manager.hpp"

#include <webgpu/webgpu.h>
#include "gpu_context.hpp"

struct App {
    const GPUContext& ctx;
    
    App(const GPUContext& ctx);
    void display();

  private:
    unsigned int shader_render_width = 1920;
    unsigned int shader_render_height = 1080;

    ShaderManager shader_manager;
    bool initiliazed = false;
};

