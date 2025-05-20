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

    ShaderParam::Integer<ShaderParam::WidgetKind::Slider> int_slider("int slider", 0, 10, 0);
    ShaderParam::Integer<ShaderParam::WidgetKind::Field> int_field("int field", 0, 10, 0);

    ShaderParam::Float<ShaderParam::WidgetKind::Slider> float_slider("float slider", 0, 10, 0);
    ShaderParam::Float<ShaderParam::WidgetKind::Field> float_field("float field", 0, 10, 0);


    // ShaderParam::WidgetGroup params(int_slider, int_field, float_slider, float_field);
    // ShaderParam::Feature<ShaderParam::WidgetKind::Checkbox, decltype(params)> enable("enable", false, params);
    // ShaderParam::Feature<ShaderParam::WidgetKind::Checkbox, void> enable("enable", false);

    ShaderParam::WidgetGroup int_params(int_slider, int_field);
    ShaderParam::WidgetGroup float_params(float_slider, float_field);

    ShaderParam::Choice<ShaderParam::WidgetKind::Dropdown, void, decltype(int_params), decltype(float_params)>
        enable("number type", 0, {"none", "int", "float"}, std::monostate{}, int_params, float_params);


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

        ShaderParam::window<decltype(enable)>(enable);

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
