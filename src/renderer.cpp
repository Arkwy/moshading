#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu-raii.hpp>

#include "renderer.hpp"


bool Renderer::is_running() {
    return !glfwWindowShouldClose(this->window);
}

void Renderer::terminate() {
    
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    this->surface->release();  // released manually as it must be relesead before instance is

    glfwDestroyWindow(this->window);
    glfwTerminate();
}

void Renderer::display_app() {
    this->app.display();
}
