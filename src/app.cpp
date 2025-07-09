#include <sys/types.h>
#include <cstring>

#include <webgpu/webgpu.h>
#include <webgpu/webgpu-raii.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_wgpu.h>

#include "app.hpp"
#include "shaders_code.hpp"
#include "src/shader/shader.hpp"

ImVec2 side_base_size(const ImVec2& window_size) { return ImVec2(window_size.x / 4., window_size.y); };


ImVec2 center_base_size(const ImVec2& window_size) { return ImVec2(window_size.x / 2., window_size.y); };


App::App(const GPUContext& ctx) : ctx(ctx), shader_manager(ctx, shader_render_width, shader_render_height) {
    shader_manager.add_shader(Shader<ShaderKind::NoParam>("s1", fullscreen_vertex, s1));
    shader_manager.add_shader(Shader<ShaderKind::Circle>("circle", fullscreen_vertex, circle));
    shader_manager.add_shader(Shader<ShaderKind::ChromaticAbberation>("ca"));
    shader_manager.add_shader(Shader<ShaderKind::Noise>("noise"));
    // shader_manager.add_shader(Shader<ShaderKind::Dithering>("dithering"));
};


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

    ImGui::Begin("shader_display");
    shader_manager.render();
    ImGui::End();
}
