
#include <algorithm>
#include "imgui.h"

void ShowSidePanel() {
    static float panelWidth = 250.0f;
    const float panelMin = 100.0f;
    const float panelMax = 400.0f;

    // Clamp the panel width
    panelWidth = std::clamp(panelWidth, panelMin, panelMax);

    // Set window position and size before Begin()
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, ImGui::GetMainViewport()->Size.y));

    // Disable resizing outside desired axis
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

    // Begin your panel window
    ImGui::Begin("SidePanel", nullptr, flags);

    // Begin scrollable content
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0);
    ImGui::BeginChild("ScrollRegion", ImVec2(0, 0), true);

    ImGui::Text("Folder Tree");

    ImGui::EndChild();  // End scrollable content
    ImGui::PopStyleVar();

    // Right edge resize handle
    ImVec2 win_pos = ImGui::GetWindowPos();
    ImVec2 win_size = ImGui::GetWindowSize();
    ImVec2 handle_pos = ImVec2(win_size.x - 20, 0);
    ImVec2 handle_size = ImVec2(20, ImGui::GetWindowHeight());

    ImGui::SetCursorScreenPos(handle_pos);
    // Define larger hitbox
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

    ImGui::Button("##bigbutton", handle_size);

    // Optional: visual feedback
    // bool hovered = ImGui::IsItemHovered();
    // bool pressed = ImGui::IsItemActive();

    // ImDrawList* draw_list = ImGui::GetWindowDrawList();
    // ImVec2 center = ImVec2(cursor_pos.x + hitbox_size.x * 0.5f, cursor_pos.y + hitbox_size.y * 0.5f);

    // // Draw actual visible button (smaller)
    // ImVec2 button_size = ImVec2(60, 20);
    // ImVec2 button_pos = ImVec2(center.x - button_size.x / 2, center.y - button_size.y / 2);
    // draw_list->AddRectFilled(
    //     button_pos,
    //     ImVec2(button_pos.x + button_size.x, button_pos.y + button_size.y),
    //     hovered ? IM_COL32(200, 200, 255, 255) : IM_COL32(150, 150, 200, 255),
    //     4.0f
    // );
    // draw_list->AddText(ImVec2(button_pos.x + 10, button_pos.y + 2), IM_COL32(0, 0, 0, 255), "Click");

    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    // ImGui::Button("ResizePanel", ImVec2(5, ImGui::GetWindowHeight()));
    if (ImGui::IsItemActive()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        panelWidth += ImGui::GetIO().MouseDelta.x;
    }

    ImGui::End();
}
