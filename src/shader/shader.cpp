#include <imgui.h>

#include "shader.hpp"


Shader::Shader(const std::string& name, const char* const vertex_code, const char* const frag_code)
    : name(name), vertex_code(vertex_code), frag_code(frag_code) {}


void Shader::display() {
    ImGui::BeginChild(name.c_str());
    ImGui::EndChild();
}
