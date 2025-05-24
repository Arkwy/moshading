#pragma once

#include <GLFW/glfw3.h>
#include <webgpu-raii.hpp>

struct Application {
    // Initialize everything and return true if it went all right
    bool Initialize();

    // Uninitialize everything that was initialized
    void Terminate();

    // Draw a frame and handle events
    void MainLoop();

    // Return true as long as the main loop should keep on running
    bool IsRunning();

private:
    // We put here all the variables that are shared between init and main loop
    GLFWwindow *window;
    wgpu::raii::Device device;
    wgpu::raii::Queue queue;
    wgpu::raii::Surface surface;
};
