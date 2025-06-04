#pragma once

#include <string>
#include <utility>

#include "shader_parameter.hpp"

struct Shader {
    std::string name;
    const std::string def;
};


template <UserParam::WidgetGroupType P>
struct ParametrisedShader : public Shader {
    P parmeters;
};

template <typename T>
concept is_parametrised_shader = requires { []<UserParam::WidgetGroupType P>(ParametrisedShader<P>) {}(std::declval<T>()); } || std::is_same_v<T, void>;
