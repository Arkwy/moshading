#include <algorithm>

#include <imgui.h>

#include "shader_parameter.hpp"

using namespace ShaderParam;


template <>
void Float<WidgetKind::Slider>::display() {
    ImGui::SliderFloat(name.c_str(), &state, min, max);
}

template <>
void Float<WidgetKind::Field>::display() {
    if (ImGui::InputFloat(name.c_str(), &state)) {
        state = std::clamp(state, min, max);
    };
}

template <>
void Integer<WidgetKind::Slider>::display() {
    ImGui::SliderInt(name.c_str(), &state, min, max);
}

template <>
void Integer<WidgetKind::Field>::display() {
    if (ImGui::InputInt(name.c_str(), &state)) {
        state = std::clamp(state, min, max);
    };
}


