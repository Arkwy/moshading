#pragma once

#include <GLFW/glfw3.h>
#include <webgpu/webgpu-raii.hpp>

struct App {
    GLFWwindow *window;
    wgpu::raii::Instance instance;
    wgpu::raii::Device device;
    wgpu::raii::Queue queue;
    wgpu::raii::Surface surface;

    wgpu::TextureFormat preferred_fmt = WGPUTextureFormat_RGBA8Unorm;
    wgpu::SurfaceConfiguration surface_config;

    // Initialize everything and return true if it went all right
    bool initialize();

    // Uninitialize everything that was initialized
    void terminate();

    // Draw a frame and handle events
    void main_loop();

    // Return true as long as the main loop should keep on running
    bool is_running();
};
