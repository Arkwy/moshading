#pragma once

#include <string>
#include <webgpu/webgpu-raii.hpp>

// #include "parameter.hpp"

struct Shader {
    const std::string name;
    const char* const vertex_code;
    const char* const frag_code; // must point to a shader definition from automatically built "shaders_code.cpp"

    Shader(const std::string& name, const char* const vertex_code, const char* const frag_code);

    void display();
};

// TODO
// template <UserParam::WidgetGroupType P>
// struct ParametrisedShader : public Shader {
//     P parmeters;
// };

// template <typename T>
// concept is_parametrised_shader = requires { []<UserParam::WidgetGroupType P>(ParametrisedShader<P>) {}(std::declval<T>()); } || std::is_same_v<T, void>;

