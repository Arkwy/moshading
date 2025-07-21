#include "parameter.hpp"

#include <imgui.h>

#include <algorithm>


// template <>
// void Float<WidgetKind::Slider>::display() {
//     ImGui::SliderFloat(name.c_str(), &state, min, max);
// }

// template <>
// void Float<WidgetKind::Field>::display() {
//     ImGui::DragFloat(name.c_str(), &state, (max - min) / 100., min, max);
// }

// template <>
// void Integer<WidgetKind::Slider>::display() const {
//     ImGui::SliderInt(name.c_str(), &state, min, max);
// }

// template <>
// void Integer<WidgetKind::Field>::display() const {
//     ImGui::DragInt(name.c_str(), &state, (max - min) / 100., min, max);
// }



////// Box2D

Box2D::Box2D(
    const std::string& name,
    const std::array<float, 2>& min,
    const std::array<float, 2>& max,
    const bool& soft_limits,
    const std::span<float, 2>& state,
    const std::span<float, 2>& box_dim,
    float& rotation,
    const std::optional<std::array<std::string, 2>>& axis_names
)
    : name(name),
      axis_names(axis_names),
      min(min),
      max(max),
      soft_limits(soft_limits),
      state(state),
      box_dim(box_dim),
      rotation(rotation) {}


bool Box2D::display() const {
    bool change = false;

    ImVec2 offset((max[0] * soft_limits), (max[1] * soft_limits));

    float width = ImGui::CalcItemWidth();
    float scale = width / (max[0] + offset.x);
    float height = (max[1] + offset.y) * scale;

    ImVec2 pad_size(width, height);
    ImVec2 pad_min = ImGui::GetCursorScreenPos();
    ImVec2 pad_max = ImVec2(pad_min.x + pad_size.x, pad_min.y + pad_size.y);


    // Normalize and draw the handle
    ImVec2 pos(state[0], state[1]);

    ImVec2 handle_start = ImVec2(
        pad_min.x + (pos.x * scale) + (offset.x * scale / 2),
        pad_min.y + (pos.y * scale) + (offset.y * scale / 2)
    );
    ImVec2 handle_end = ImVec2(handle_start.x + box_dim[0] * scale, handle_start.y + box_dim[1] * scale);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    // background
    draw_list->AddRectFilled(pad_min, pad_max, IM_COL32(50, 50, 50, 255));

    ImVec2 center((handle_start.x + handle_end.x) / 2, (handle_start.y + handle_end.y) / 2);
    // image frame
    draw_image_border(
        draw_list,
        center,
        ImVec2(handle_end.x - handle_start.x, handle_end.y - handle_start.y),
        rotation,
        pad_min,
        pad_max,
        IM_COL32(255, 0, 0, 255),
        false
    );

    // display frame
    draw_list->AddRect(pad_min, pad_max, IM_COL32(255, 255, 255, 255));

    // render frame
    if (soft_limits) {
        draw_list->AddRect(
            ImVec2(pad_min.x + offset.x * scale / 2, pad_min.y + offset.y * scale / 2),
            ImVec2(pad_max.x - offset.x * scale / 2, pad_max.y - offset.y * scale / 2),
            IM_COL32(255, 255, 255, 255)
        );
    }

    // control logic
    // ImGui::InvisibleButton("pad", pad_size);
    // bool is_active = ImGui::IsItemActive();

    // if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
    //     ImVec2 delta = ImGui::GetIO().MouseDelta;
    //     state[0] += delta.x / scale;
    //     state[1] += delta.y / scale;
    // }
    return change;
}


void Box2D::draw_image_border(
    ImDrawList* draw_list,
    ImVec2 center,
    ImVec2 size,
    float angle_rad,
    ImVec2 draw_area_min,
    ImVec2 draw_area_max,
    ImU32 color,
    bool filled
) {
    // Half sizes
    float hw = size.x * 0.5f;
    float hh = size.y * 0.5f;

    float cos_a = cosf(angle_rad);
    float sin_a = sinf(angle_rad);

    // Local-space rectangle corners (centered at origin)
    ImVec2 corners[4] = {{-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh}};

    std::vector<ImVec2> poly;
    for (int i = 0; i < 4; ++i) {
        float x = corners[i].x;
        float y = corners[i].y;

        ImVec2 rotated = {center.x + x * cos_a - y * sin_a, center.y + x * sin_a + y * cos_a};
        poly.push_back(rotated);
    }

    // Define clipping edges (clockwise)
    ImVec2 clip_edges[4][2] = {
        {ImVec2(draw_area_min.x, draw_area_min.y), ImVec2(draw_area_max.x, draw_area_min.y)},  // top
        {ImVec2(draw_area_max.x, draw_area_min.y), ImVec2(draw_area_max.x, draw_area_max.y)},  // right
        {ImVec2(draw_area_max.x, draw_area_max.y), ImVec2(draw_area_min.x, draw_area_max.y)},  // bottom
        {ImVec2(draw_area_min.x, draw_area_max.y), ImVec2(draw_area_min.x, draw_area_min.y)}   // left
    };

    // Clip the polygon against each edge
    for (int i = 0; i < 4; ++i) {
        poly = clip_polygon_edge(poly, clip_edges[i][0], clip_edges[i][1]);
    }

    if (poly.empty()) {
        ImVec2 marker_pos(
            ImClamp(center.x, draw_area_min.x, draw_area_max.x),
            ImClamp(center.y, draw_area_min.y, draw_area_max.y)
        );
        draw_list->AddCircleFilled(marker_pos, 4.0f, IM_COL32(255, 0, 0, 255));
    } else if (filled)
        draw_list->AddConvexPolyFilled(poly.data(), poly.size(), color);
    else
        draw_list->AddPolyline(poly.data(), poly.size(), color, true, 1.0f);
}


std::vector<ImVec2> Box2D::clip_polygon_edge(const std::vector<ImVec2>& poly, const ImVec2& p1, const ImVec2& p2) {
    std::vector<ImVec2> result;

    for (size_t i = 0; i < poly.size(); ++i) {
        const ImVec2& current = poly[i];
        const ImVec2& prev = poly[(i + poly.size() - 1) % poly.size()];

        // Edge vector
        ImVec2 edge = ImVec2(p2.x - p1.x, p2.y - p1.y);

        // Calculate inside-ness using cross product
        auto is_inside = [&](const ImVec2& pt) { return (edge.x * (pt.y - p1.y) - edge.y * (pt.x - p1.x)) >= 0; };

        bool current_in = is_inside(current);
        bool prev_in = is_inside(prev);

        if (current_in) {
            if (!prev_in) {
                // Entering – compute intersection
                float dx = current.x - prev.x;
                float dy = current.y - prev.y;
                float t_num = (edge.x * (prev.y - p1.y) - edge.y * (prev.x - p1.x));
                float t_den = (edge.y * dx - edge.x * dy);
                float t = t_num / t_den;

                ImVec2 intersect = {prev.x + dx * t, prev.y + dy * t};
                result.push_back(intersect);
            }
            result.push_back(current);
        } else if (prev_in) {
            // Exiting – compute intersection
            float dx = current.x - prev.x;
            float dy = current.y - prev.y;
            float t_num = (edge.x * (prev.y - p1.y) - edge.y * (prev.x - p1.x));
            float t_den = (edge.y * dx - edge.x * dy);
            float t = t_num / t_den;

            ImVec2 intersect = {prev.x + dx * t, prev.y + dy * t};
            result.push_back(intersect);
        }
    }

    return result;
}
