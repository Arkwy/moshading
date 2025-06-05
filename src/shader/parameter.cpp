#include <algorithm>

#include <imgui.h>

#include "parameter.hpp"

using namespace UserParam;


template <>
void Float<WidgetKind::Slider>::display() {
    ImGui::SliderFloat(name.c_str(), &state, min, max);
}

template <>
void Float<WidgetKind::Field>::display() {
    ImGui::DragFloat(name.c_str(), &state, (max - min) / 100., min, max);
}

template <>
void Integer<WidgetKind::Slider>::display() {
    ImGui::SliderInt(name.c_str(), &state, min, max);
}

template <>
void Integer<WidgetKind::Field>::display() {
    ImGui::DragInt(name.c_str(), &state, (max - min) / 100., min, max);
}


