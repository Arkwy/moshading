// #include <backends/imgui_impl_wgpu.h>
// #include <backends/imgui_impl_glfw.h>
#include <webgpu-raii.hpp>

#include "application.hpp"
#include "glfw3webgpu.hpp"


bool Application::Initialize() {
    wgpu::InstanceDescriptor desc;

    wgpu::raii::Instance instance = wgpu::createInstance(desc);

    if (!instance) {
        std::cerr << "Could not initialize WebGPU!" << std::endl;
        return false;
    }

    *surface = glfwGetWGPUSurface(*instance, window);

    wgpu::raii::Adapter adapter = instance->requestAdapter(wgpu::RequestAdapterOptions());  // sync request


    device = adapter->requestDevice(wgpu::DeviceDescriptor());  // sync request


    queue = device->getQueue();

    return true;
}

void Application::Terminate() {
    // Move all the release/destroy/terminate calls here
}

void Application::MainLoop() {
    // glfwPollEvents();

}

bool Application::IsRunning() {
    // return !glfwWindowShouldClose(window);
}
