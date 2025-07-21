#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include <cmath>  // for cos, sin
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>


enum class WidgetKind {
    Slider,
    Field,
    DragField,
    Box2D,
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

    std::tuple<Ws...> widgets;

    WidgetGroup(Ws&&... widgets) : widgets(widgets...) {}

    bool display() const {
        bool change = false;
        std::apply([&](auto&... widget) { ((change |= widget.display()), ...); }, widgets);
        return change;
    }



};  // WidgetGroup is itself a widget


template <typename T>
concept WidgetGroupType =
    requires { []<Widget... Ws>(WidgetGroup<Ws...>) {}(std::declval<T>()); } || std::is_same_v<T, void>;



template <size_t N, WidgetKind... WKs>
    requires(
        sizeof...(WKs) > 0 &&
        ((WKs == WidgetKind::Slider || WKs == WidgetKind::Field || WKs == WidgetKind::DragField) && ...)
    )
struct Float {
    std::string name;
    std::optional<std::array<std::string, N>> field_names;
    std::array<float, N> speed;
    std::array<float, N> min;
    std::array<float, N> max;
    std::span<float, N> state;

    Float(
        const std::string& name,
        const std::array<float, N>& speed,
        const std::array<float, N>& min,
        const std::array<float, N>& max,
        const std::span<float, N>& state,
        const std::optional<std::array<std::string, N>>& field_names = std::nullopt
    )
        : name(name), field_names(field_names), speed(speed), min(min), max(max), state(state) {}

    bool display() const {
        bool change = false;
        ((change |= display_impl<WKs>()), ...);
        return change;
    }

  private:
    template <WidgetKind W>
    bool display_impl() const {
        bool value_changed = false;
        if constexpr (W == WidgetKind::Slider) {
            static_assert(false, "No implementation yet.");  // TODO
        } else if constexpr (W == WidgetKind::Field) {
            static_assert(false, "No implementation yet.");  // TODO
        } else if constexpr (W == WidgetKind::DragField) {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems) return false;

            ImGuiContext& g = *GImGui;
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
                    speed[i],
                    &min[i],
                    &max[i],
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
        }
        return value_changed;
    }
};

struct Box2D {
    std::string name;
    std::optional<std::array<std::string, 2>> axis_names;
    std::array<float, 2> min;
    std::array<float, 2> max;
    bool soft_limits;
    std::span<float, 2> state;
    std::span<float, 2> box_dim;
    float& rotation;

    Box2D(
        const std::string& name,
        const std::array<float, 2>& min,
        const std::array<float, 2>& max,
        const bool& soft_limits,
        const std::span<float, 2>& state,
        const std::span<float, 2>& box_dim,
        float& rotation,
        const std::optional<std::array<std::string, 2>>& axis_names = std::nullopt
    );

    bool display() const;

  private:
    static void draw_image_border(
        ImDrawList* draw_list,
        ImVec2 center,
        ImVec2 size,
        float angle_rad,
        ImVec2 draw_area_min,
        ImVec2 draw_area_max,
        ImU32 color,
        bool filled = true
    );
    static std::vector<ImVec2> clip_polygon_edge(const std::vector<ImVec2>& poly, const ImVec2& p1, const ImVec2& p2);
};


template <WidgetKind W>
struct Integer {
    const std::string name;
    const int min;
    const int max;

    int state;

    Integer(const std::string& name, const int& min, const int& max, int const state)
        : name(name), min(min), max(max), state(state) {}

    Integer(const Integer<W>& other) {
        std::cout << "copy !!! ò_ó" << std::endl;
    }

    bool display() const;
};



template <typename T>
using opt = std::conditional_t<std::is_void_v<T>, std::monostate, std::optional<T>>;

template <typename OptChild>
void display_if_present(OptChild& maybe_child) {
    if constexpr (!std::is_same_v<std::remove_const_t<OptChild>, std::monostate>) {
        if (maybe_child) maybe_child.value().display();
    }
}

template <WidgetKind W, WidgetGroupType Childs = void>
struct Feature {
    const std::string name;
    const opt<Childs> childs;

    bool& state;

    Feature(const std::string& name, bool& state)
        requires std::is_same_v<Childs, void>
        : name(name), childs(std::monostate{}), state(state) {}

    Feature(const std::string& name, bool& initial_state, Childs&& childs)
        requires(!std::is_same_v<Childs, void>)
        : name(name), childs(std::make_optional(std::forward(childs))), state(state) {}


    bool display() const
        requires(W == WidgetKind::Checkbox)
    {
        bool change = ImGui::Checkbox(name.c_str(), &state);
        if (state) display_if_present(childs);
        return change;
    }
};



template <size_t I = 0, WidgetGroupType... WGs>
bool display_indexed(size_t index, const std::tuple<WGs...>& t) {
    if constexpr (I < sizeof...(WGs)) {
        if (index == I) {
            return std::get<I>(t).display();
        } else {
            return display_indexed<I + 1>(index, t);
        }
    }
}


template <WidgetKind W, WidgetGroupType... Childs>
struct Choice {
    const std::string name;
    const std::array<std::string, sizeof...(Childs)> values;
    const std::tuple<Childs...> childs;

    unsigned int& state;

    Choice(
        const std::string& name,
        unsigned int& state,
        const std::array<std::string, sizeof...(Childs)>& values,
        Childs&&... childs
    )
        : name(name), values(values), childs(childs...), state(state) {}

    Choice(const std::string& name, unsigned int& state, const std::array<std::string, sizeof...(Childs)>& values)
        : name(name), values(values), childs(), state(state) {}

    bool display() const
        requires(W == WidgetKind::Dropdown)
    {
        bool change = ImGui::BeginCombo(name.c_str(), values[state].c_str());
        if (change) {
            for (unsigned int i = 0; i < values.size(); i++) {
                const bool is_selected = (i == state);

                if (ImGui::Selectable(values[i].c_str(), is_selected)) state = i;

                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        change |= display_indexed(state, childs);
        return change;
    }
};

template <Widget W>
void window(W& params) {
    params.display();
}
