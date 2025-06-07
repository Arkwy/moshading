#include "shader.hpp"

#include <imgui.h>

#include <memory>


Shader::Shader(const std::string& name, const char* const vertex_code, const char* const frag_code)
    : name(name), vertex_code(vertex_code), frag_code(frag_code) {}


void Shader::display() {
    float v = 10;
    ImGui::SliderFloat("tt", &v, 0, 20);
}
