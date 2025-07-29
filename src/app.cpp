#include "app.hpp"

#include <backends/imgui_impl_wgpu.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <sys/types.h>
#include <webgpu/webgpu.h>

#include <cassert>
#include <cstring>
#include <filesystem>
#include <webgpu/webgpu-raii.hpp>

#include "src/context/resource.hpp"
#include "src/file_loader.hpp"


App::App(Context& ctx) : ctx(ctx), shader_manager(ctx) {}


void ressource_manager_display(ResourceManager& ressource_manager) {
    static FileLoader file_loader;

    bool can_open_dialog = file_loader.check();

    ImGui::BeginDisabled(!can_open_dialog);
    if (ImGui::Button("Import")) {
        file_loader.open_dialog<ResourceKind::Image>(
#ifdef __EMSCRIPTEN__
            [&](const char* name, uint8_t* data, size_t len) {
                ressource_manager.add_image(name, Resource<ResourceKind::Image>::Handle{.data = data, .len = len});
            }
#else
            [&](const std::string& file) {
                std::filesystem::path path = file;
                ressource_manager.add_image(path.stem(), path);
            }
#endif
        );
    }
    ImGui::EndDisabled();

    float vignette_size = 200;
    float max_cursor_x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - vignette_size;
    for (auto& [resource_id, index] : ressource_manager.images_index_map) {
        auto& resource = ressource_manager.images[index];
        ImGui::BeginChild(
            std::format("{}{}", resource.name, resource_id).c_str(),
            ImVec2(vignette_size, vignette_size)
        );
        resource.display();
        ImGui::EndChild();
        ImGui::SameLine();
        if (ImGui::GetCursorPosX() > max_cursor_x) ImGui::NewLine();
    }
}


void App::display() {
    ImGuiID dockspace_id = ImGui::GetID("viewport_dockspace");
    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport());

    if (!initiliazed) {
        ImGui::DockBuilderRemoveNode(dockspace_id);  // reset layout
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
        if (node)
            node->LocalFlags |=
                ImGuiDockNodeFlags_NoUndocking | static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoTabBar);

        ImGuiID dock_id_right =
            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);
        node = ImGui::DockBuilderGetNode(dock_id_right);
        if (node)
            node->LocalFlags |=
                ImGuiDockNodeFlags_NoUndocking | static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoTabBar);

        ImGuiID dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_id);
        node = ImGui::DockBuilderGetNode(dock_id_down);
        if (node)
            node->LocalFlags |=
                ImGuiDockNodeFlags_NoUndocking | static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoTabBar);

        ImGui::DockBuilderDockWindow("shader_display", dockspace_id);
        ImGui::DockBuilderDockWindow("shader_manager", dock_id_right);
        ImGui::DockBuilderDockWindow("resource_manager", dock_id_down);
        ImGui::DockBuilderFinish(dockspace_id);

        initiliazed = true;
    }

    ImGui::Begin("shader_manager");
    shader_manager.display();
    ImGui::End();

    ImGui::Begin("resource_manager");
    ressource_manager_display(ctx.resource_manager);
    ImGui::End();

    ImGui::Begin("shader_display", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
    shader_manager.render();
    ImGui::End();
}
