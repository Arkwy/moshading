#include "app.hpp"

#include <backends/imgui_impl_wgpu.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <sys/types.h>
#include <webgpu/webgpu.h>

#include <cstring>
#include <webgpu/webgpu-raii.hpp>


App::App(Context& ctx) : ctx(ctx), shader_manager(ctx) {}


void App::display() {
    ImGuiID dockspace_id = ImGui::GetID("viewport_dockspace");
    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport());

    if (!initiliazed) {
        ImGui::DockBuilderRemoveNode(dockspace_id);  // reset layout
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_id_right =
            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);

        ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_id_right);
        if (node)
            node->LocalFlags |=
                ImGuiDockNodeFlags_NoUndocking | static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoTabBar);

        node = ImGui::DockBuilderGetNode(dockspace_id);
        if (node)
            node->LocalFlags |=
                ImGuiDockNodeFlags_NoUndocking | static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoTabBar);

        ImGui::DockBuilderDockWindow("shader_manager", dock_id_right);
        ImGui::DockBuilderDockWindow("shader_display", dockspace_id);
        ImGui::DockBuilderFinish(dockspace_id);

        initiliazed = true;
    }

    ImGui::Begin("shader_manager");
    shader_manager.display();
    ImGui::End();

    ImGui::Begin("shader_display", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
    shader_manager.render();
    ImGui::End();
}
