#pragma once

#include <GLFW/glfw3.h>
#include <webgpu/webgpu-raii.hpp>

#include "app.hpp"
#include "gpu_context.hpp"

struct Renderer {
    const GPUContext& ctx;
    GLFWwindow *window;
    wgpu::raii::Surface surface;

    wgpu::TextureFormat preferred_fmt = WGPUTextureFormat_RGBA8Unorm;
    wgpu::SurfaceConfiguration surface_config;

    App app;

    Renderer(const GPUContext& ctx) : ctx(ctx), app(ctx) {};

    // Base display
    bool init();
    void terminate();
    void main_loop();
    bool is_running();

    void display_app();
};
