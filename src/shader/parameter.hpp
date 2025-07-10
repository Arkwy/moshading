#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "src/log.hpp"

namespace UserParam {

enum class WidgetKind {
    Slider,
    Field,
    DragField,
    Checkbox,
    Dropdown,
};


template <typename T>
concept Widget = requires(T t) {
    { t.display() } -> std::same_as<bool>;
    t.state;
};


template <Widget... Ws>
struct WidgetGroup {
    static constexpr size_t size = sizeof...(Ws);

    std::tuple<Ws&...> widgets;

    WidgetGroup(Ws&... widgets) : widgets(widgets...) {}

    bool display() {
        std::apply([](auto&... widget) { return (widget.display() && ...); }, widgets);
    }



};  // WidgetGroup is itself a widget


template <typename T>
concept WidgetGroupType =
    requires { []<Widget... Ws>(WidgetGroup<Ws...>) {}(std::declval<T>()); } || std::is_same_v<T, void>;



template <size_t N, WidgetKind W>
    requires(W == WidgetKind::Slider || W == WidgetKind::Field || W == WidgetKind::DragField)
struct Float {
    const std::string name;
    const std::optional<std::array<std::string, N>> field_names;
    const std::array<float, N> min;
    const std::array<float, N> max;

    std::span<float, N> state;

    Float(
        const std::string& name,
        const std::array<float, N>& min,
        const std::array<float, N>& max,
        const std::span<float, N>& state,
        const std::optional<std::array<std::string, N>>& field_names = std::nullopt
    )
        : name(name), field_names(field_names), min(min), max(max), state(state) {}

    Float(Float<N, W>& other) = delete;
    Float(Float<N, W>&& other)
        : name(std::move(other.name)),
          field_names(std::move(other.field_names)),
          min(std::move(other.min)),
          max(std::move(other.max)),
          state(std::move(other.state)) {
        Log::warn(std::format("copy -> {}", static_cast<const void*>(state.data())));
    }

    bool display() {
        if constexpr (W == WidgetKind::Slider) {
            return ImGui::SliderScalarN(
                name.c_str(),
                ImGuiDataType_Float,
                state,
                N,
                &min,
                &max,
                "%.3f",
                ImGuiSliderFlags_None
            );
        } else if constexpr (W == WidgetKind::DragField) {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems) return false;

            ImGuiContext& g = *GImGui;
            bool value_changed = false;
            ImGui::BeginGroup();
            ImGui::PushID(name.c_str());
            ImGui::PushMultiItemsWidths(N, ImGui::CalcItemWidth());
            for (int i = 0; i < N; i++) {
                ImGui::PushID(field_names.has_value() ? field_names.value()[i].c_str() : std::to_string(i).c_str());
                if (i > 0) ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
                value_changed |= ImGui::DragScalar(
                    "",
                    ImGuiDataType_Float,
                    state.data() + i,
                    max[i] == min[i] ? 0.1 : (max[i] - min[i]) / 100.0,
                    min.data() + i,
                    max.data() + i,
                    field_names.has_value() ? std::format("{}: %.3f", field_names.value()[i]).c_str() : "%.3f",
                    0
                );
                ImGui::PopID();
                ImGui::PopItemWidth();
            }
            ImGui::PopID();

            ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
            ImGui::Text("%s", name.c_str());

            ImGui::EndGroup();
            return value_changed;
        }
    }
};



template <WidgetKind W>
struct Integer {
    const std::string name;
    const int min;
    const int max;

    int state;

    Integer(const std::string& name, const int& min, const int& max, int const state)
        : name(name), min(min), max(max), state(state) {}

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

}  // namespace UserParam
