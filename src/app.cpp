#include <sys/types.h>
#include <cstring>

#include <webgpu/webgpu.h>
#include <webgpu/webgpu-raii.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_wgpu.h>

#include "app.hpp"
#include "shaders_code.hpp"

ImVec2 side_base_size(const ImVec2& window_size) { return ImVec2(window_size.x / 4., window_size.y); };


ImVec2 center_base_size(const ImVec2& window_size) { return ImVec2(window_size.x / 2., window_size.y); };


App::App(const GPUContext& ctx) : ctx(ctx), shader_manager(ctx, shader_render_width, shader_render_height) {
    shader_manager.add_shader("shader test", fullscreen_vertex, shader);
    shader_manager.add_shader("shader test 2", fullscreen_vertex, bbbb);

    shader_manager.reorder_element(0, 1);
};


void App::display() {
    ImGuiID dockspace_id = ImGui::GetID("viewport_dockspace");
    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport());

    if (!initiliazed) {
        ImGui::DockBuilderRemoveNode(dockspace_id);  // reset layout
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.25f, nullptr, &dockspace_id);
        ImGuiID dock_id_right =
            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);

        ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_id_left);
        if (node)
            node->LocalFlags |=
                ImGuiDockNodeFlags_NoUndocking | static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoTabBar);

        node = ImGui::DockBuilderGetNode(dock_id_right);
        if (node)
            node->LocalFlags |=
                ImGuiDockNodeFlags_NoUndocking | static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoTabBar);

        node = ImGui::DockBuilderGetNode(dockspace_id);
        if (node)
            node->LocalFlags |=
                ImGuiDockNodeFlags_NoUndocking | static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoTabBar);

        ImGui::DockBuilderDockWindow("input_manager", dock_id_left);
        ImGui::DockBuilderDockWindow("shader_manager", dock_id_right);
        ImGui::DockBuilderDockWindow("shader_display", dockspace_id);
        ImGui::DockBuilderFinish(dockspace_id);

        initiliazed = true;
    }

    ImGui::Begin("input_manager");
    ImGui::End();

    ImGui::Begin("shader_manager");
    shader_manager.display();
    ImGui::End();

    ImGui::Begin("shader_display");
    shader_manager.render();
    ImGui::End();
}


// // if (!*rescale_sampler) {
//     wgpu::SamplerDescriptor sampler_desc = {};
//     sampler_desc.magFilter = wgpu::FilterMode::Linear;
//     sampler_desc.minFilter = wgpu::FilterMode::Linear;
//     sampler_desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
//     sampler_desc.addressModeU = wgpu::AddressMode::ClampToEdge;
//     sampler_desc.addressModeV = wgpu::AddressMode::ClampToEdge;
//     sampler_desc.maxAnisotropy = 1;


//     rescale_sampler = ctx.get_device().createSampler(sampler_desc);
// // }


// // if (!*rescale_bind_group_layout) {
//     Log::log("creat bgl");
//     wgpu::BindGroupLayoutEntry layout_entries[2] = {};
//     layout_entries[0].binding = 0;
//     layout_entries[0].visibility = WGPUShaderStage_Fragment;
//     layout_entries[0].sampler.type = WGPUSamplerBindingType_Filtering;

//     layout_entries[1].binding = 1;
//     layout_entries[1].visibility = WGPUShaderStage_Fragment;
//     layout_entries[1].texture.sampleType = WGPUTextureSampleType_Float;
//     layout_entries[1].texture.viewDimension = WGPUTextureViewDimension_2D;
//     layout_entries[1].texture.multisampled = false;

//     wgpu::BindGroupLayoutDescriptor layout_desc = {};
//     layout_desc.entryCount = 2;
//     layout_desc.entries =layout_entries;

//     rescale_bind_group_layout = ctx.get_device().createBindGroupLayout(layout_desc);
// // }

// // if (!*rescale_bind_group) {
//     Log::log("creat bg");
//     wgpu::BindGroupEntry entries[2];
//     entries[0].binding = 0;
//     entries[0].sampler = *rescale_sampler;

//     entries[1].binding = 1;
//     entries[1].textureView = *tex_view;

//     wgpu::BindGroupDescriptor bg_desc;
//     bg_desc.entryCount = 2;
//     bg_desc.entries = entries;
//     bg_desc.layout = *rescale_bind_group_layout;

//     rescale_bind_group = ctx.get_device().createBindGroup(bg_desc);
// // }



// ImVec2 drawing_area = ImGui::GetContentRegionAvail();
// ImVec2 render_to_drawing_ratio(drawing_area.x / previous_render_dim.x, drawing_area.y /
// previous_render_dim.y);

// ImVec2 final_render_dim;
// if (render_to_drawing_ratio.x < render_to_drawing_ratio.y) {
//     final_render_dim.x = render_to_drawing_ratio.x * previous_render_dim.x;
//     final_render_dim.y = render_to_drawing_ratio.x * previous_render_dim.y;
// } else {
//     final_render_dim.x = render_to_drawing_ratio.y * previous_render_dim.x;
//     final_render_dim.y = render_to_drawing_ratio.y * previous_render_dim.y;
// }

// // Center drawing
// ImGui::SetCursorPos(ImVec2(
//     -(final_render_dim.x - drawing_area.x) * 0.5 + ImGui::GetCursorPosX(),
//     -(final_render_dim.y - drawing_area.y) * 0.5 + ImGui::GetCursorPosY()
// ));


// Log::info("ok");
// Log::info("ok");
// ImGui::Image(
//     (ImTextureID)(intptr_t)tex_view,
//     ImVec2(shader_render_width, shader_render_height),
//     ImVec2(0, 1),
//     ImVec2(1, 0)
// );
// Log::info("okk");
