#include <algorithm>

#include <imgui.h>

#include "shader_parameter.hpp"

using namespace ShaderParam;


template <>
void Float<Widget::Slider>::display() {
    ImGui::SliderFloat(name.c_str(), &state, min, max);
}

template <>
void Float<Widget::Field>::display() {
    if (ImGui::InputFloat(name.c_str(), &state)) {
        state = std::clamp(state, min, max);
    };
}

template <>
void Integer<Widget::Slider>::display() {
    ImGui::SliderInt(name.c_str(), &state, min, max);
}

template <>
void Integer<Widget::Field>::display() {
    if (ImGui::InputInt(name.c_str(), &state)) {
        state = std::clamp(state, min, max);
    };
}

template <>
void Feature<Widget::Checkbox>::display() {
    ImGui::Checkbox(name.c_str(), &state);
    if (state)
        for (auto& p : childs) 
            p->display();
}

template <>
void Choice<Widget::Dropdown>::display() {
    ImGui::BeginCombo(name.c_str(), values[state].c_str());
    for (int i = 0; i < values.size(); i++) 
            {
                const bool is_selected = (i == state);
                if (ImGui::Selectable(values[i].c_str(), is_selected))
                    state = i;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
    ImGui::EndCombo();

    for (auto& p : childs[state])
        p->display();
}

void ShaderParam::window(std::vector<std::unique_ptr<Param>>& params) {
    ImGui::Begin("Parameters", nullptr);
    for (auto& p : params)
        p->display();
    ImGui::End();
}
