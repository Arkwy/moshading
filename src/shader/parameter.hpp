#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <imgui.h>

namespace UserParam {

enum class WidgetKind {
    Slider,
    Field,
    Checkbox,
    Dropdown,
};


template <typename T>
concept Widget = requires(T t) {
    { t.display() } -> std::same_as<void>;
};


template <Widget... Ws>
struct WidgetGroup {
    static constexpr size_t size = sizeof...(Ws);

    std::tuple<Ws&...> widgets;

    WidgetGroup(Ws&... widgets) : widgets(widgets...) {}

    void display() {
        std::apply([](auto&... widget) { (widget.display(), ...); }, widgets);
    }
};  // WidgetGroup is itself a widget


template <typename T>
concept WidgetGroupType =
    requires { []<Widget... Ws>(WidgetGroup<Ws...>) {}(std::declval<T>()); } || std::is_same_v<T, void>;



template <WidgetKind W>
struct Float {
    const std::string name;
    const float min;
    const float max;

    float state;

    Float(const std::string& name, const float& min, const float& max, const float& initial_value)
        : name(name), min(min), max(max), state(initial_value) {}

    void display();
};



template <WidgetKind W>
struct Integer {
    const std::string name;
    const int min;
    const int max;

    int state;

    Integer(const std::string& name, const int& min, const int& max, const int& initial_value)
        : name(name), min(min), max(max), state(initial_value) {}

    Integer(const Integer<W>& other) { std::cout << "copy !!! ò_ó" << std::endl; }

    void display();
};



template <typename T>
using optional_ref = std::conditional_t<std::is_void_v<T>, std::monostate, std::optional<std::reference_wrapper<T>>>;


template <typename OptRef>
void display_if_present(OptRef& maybe_ref) {
    if constexpr (!std::is_same_v<std::remove_const_t<OptRef>, std::monostate>) {
        if (maybe_ref) maybe_ref->get().display();
    }
}

template <size_t I = 0, typename... OptRefs>
static void display_indexed(size_t index, const std::tuple<OptRefs...>& t) {
    if constexpr (I < sizeof...(OptRefs)) {
        if (index == I) {
            display_if_present(std::get<I>(t));
        } else {
            display_indexed<I + 1>(index, t);
        }
    }
}


template <WidgetKind W, WidgetGroupType Childs = void>
struct Feature {
    const std::string name;
    optional_ref<Childs> childs;

    bool state;

    Feature(const std::string& name, const bool& initial_state)
        requires std::is_same_v<Childs, void>
    : name(name), childs(std::monostate{}), state(initial_state) {}

    Feature(const std::string& name, const bool& initial_state, const optional_ref<Childs>& childs)
        requires(!std::is_same_v<Childs, void>)
        : name(name), childs(childs), state(initial_state) {}


    bool get_state() const { return state; }

    void display()
        requires(W == WidgetKind::Checkbox)
    {
        ImGui::Checkbox(name.c_str(), &state);
        if (state) display_if_present(childs);
    }
};



template <WidgetKind W, WidgetGroupType... Childs>
struct Choice {
    const std::string name;
    const std::array<std::string, sizeof...(Childs)> values;
    const std::tuple<optional_ref<Childs>...> childs;

    size_t state;

    Choice(
        const std::string& name,
        const size_t& initial_state_idx,
        const std::array<std::string, sizeof...(Childs)>& values,
        const optional_ref<Childs>&... childs
    )
        : name(name), values(values), childs(childs...), state(initial_state_idx) {}

    Choice(
        const std::string& name,
        const size_t& initial_state_idx,
        const std::array<std::string, sizeof...(Childs)>& values
    )
        : name(name), values(values), childs(), state(initial_state_idx) {}

    void display()
        requires(W == WidgetKind::Dropdown)
    {
        if (ImGui::BeginCombo(name.c_str(), values[state].c_str())) {
            for (int i = 0; i < values.size(); i++) {
                const bool is_selected = (i == state);

                if (ImGui::Selectable(values[i].c_str(), is_selected)) state = i;

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        display_indexed(state, childs);
    }
};

template <Widget W>
void window(W& params) {
    params.display();
}

}  // namespace ShaderParam
