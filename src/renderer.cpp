#include "renderer.hpp"

#include <GLFW/glfw3.h>
#include <IconsFontAwesome6.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>

#include <webgpu/webgpu-raii.hpp>

#include "icons.hpp"


bool Renderer::is_running() {
    return !glfwWindowShouldClose(this->window);
}


void Renderer::pause_rendering() {
    paused = true;
}


void Renderer::resume_rendering() {
    paused = false;
}


void Renderer::terminate() {
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    this->surface->release();  // released manualy as it must be released before instance is

    glfwDestroyWindow(this->window);
    glfwTerminate();
}

void Renderer::display_app() {
    this->app.display();
}

void Renderer::set_style() {
    ImGui::GetStyle().ScaleAllSizes(1.5);

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.5;
    io.Fonts->AddFontDefault();
    // 13.0f is the size of the default font. Change to the font size you use.
    float baseFontSize = 13.0f;
    // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
    float iconFontSize = baseFontSize * 2.0f / 3.0f;

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    icons_config.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(icons_data, icons_data_len, iconFontSize, &icons_config, icons_ranges);
}
