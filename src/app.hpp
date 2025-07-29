#pragma once

#include "shader/manager.hpp"

#include <webgpu/webgpu.h>
#include "context.hpp"

struct App {
    Context& ctx;
    
    App(Context& ctx);
    void display();

  private:
    ShaderManager shader_manager;
    bool initiliazed = false;
};

