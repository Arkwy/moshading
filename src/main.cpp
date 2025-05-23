#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"

#include <webgpu/webgpu.h>
#include <iostream>
// #include "shader.hpp"
// #include "shader_parameter.hpp"

int main() {
    // Init GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* win = glfwCreateWindow(1280, 720, "ImGui Shader Window", NULL, NULL);
    if (!win) return -1;
    glfwMakeContextCurrent(win);
    glewInit();
    glfwDestroyWindow(win);
    glfwTerminate();
    wgpuCreateInstance(nullptr);

    std::cout << "is ok" << std::endl;
    return 0;
}
