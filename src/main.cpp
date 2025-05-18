#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <chrono>
#include <vector>

#include "shader.hpp"
#include "shader_parameter.hpp"

extern GLuint tex;


int main() {
    // Init GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* win = glfwCreateWindow(1280, 720, "ImGui Shader Window", NULL, NULL);
    if (!win) return -1;
    glfwMakeContextCurrent(win);
    glewInit();

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Init rendering
    GLuint prog = CreateShaderProgram();
    InitQuad();
    CreateFBO(512, 512);  // default size

    std::vector<std::unique_ptr<ShaderParam::Param>> params;
    params.push_back(std::make_unique<ShaderParam::Float<ShaderParam::Widget::Slider>>("parmates", 0, 10, 5));
    params.push_back(std::make_unique<ShaderParam::Float<ShaderParam::Widget::Field>>("parmate", 0, 10, 5));
    params.push_back(std::make_unique<ShaderParam::Integer<ShaderParam::Widget::Slider>>("parmat", 0, 10, 5));
    params.push_back(std::make_unique<ShaderParam::Integer<ShaderParam::Widget::Field>>("parma", 0, 10, 5));


    params.push_back([]() {
        auto f = std::make_unique<ShaderParam::Feature<ShaderParam::Widget::Checkbox>>("parm", false);
        f->add_child<ShaderParam::Float<ShaderParam::Widget::Slider>>("paa", 0, 10, 5);
        return f;
    }());
    // params.push_back(f);



    auto start = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        int display_w, display_h;
        glfwGetFramebufferSize(win, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);

        auto now = std::chrono::steady_clock::now();
        float time = std::chrono::duration<float>(now - start).count();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        create_window(prog, time);

        ShaderParam::window(params);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(win);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
