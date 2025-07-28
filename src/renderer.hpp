#pragma once

#include <GLFW/glfw3.h>
#include <webgpu/webgpu-raii.hpp>

#include "app.hpp"
#include "context.hpp"

struct Renderer {
    Context& ctx;
    GLFWwindow *window;
    wgpu::raii::Surface surface;

    wgpu::TextureFormat preferred_fmt = WGPUTextureFormat_RGBA8Unorm;
    wgpu::SurfaceConfiguration surface_config;

    App app;

    Renderer(Context& ctx) : ctx(ctx), app(ctx) {};

    // Base display
    bool init();
    void set_style();
    void main_loop();
    bool is_running();
    void display_app();
    void terminate();
};
