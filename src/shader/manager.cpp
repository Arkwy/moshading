#include "manager.hpp"

#include <IconsFontAwesome6.h>
#include <imgui.h>

#include <chrono>
#include <memory>
#include <webgpu/webgpu.hpp>

#include "src/file_loader.hpp"
#include "src/log.hpp"
#include "src/shader/shader.hpp"

ShaderManager::ShaderManager(Context& ctx) : ctx(ctx), shaders() {
    init();
}


void ShaderManager::init() {
    assert(!*texture_A && !*texture_view_A && !*texture_B && !*texture_view_B);
    start_time = std::chrono::high_resolution_clock::now();

    // Textures
    wgpu::TextureDescriptor texture_desc;
#ifdef __EMSCRIPTEN__
    texture_desc.label = "shader_render";
#else
    texture_desc.label.data = "shader_render";
    texture_desc.label.length = WGPU_STRLEN;
#endif
    texture_desc.size.width = ctx.rendering.dim[0];
    texture_desc.size.height = ctx.rendering.dim[1];
    texture_desc.size.depthOrArrayLayers = 1;
    texture_desc.format = wgpu::TextureFormat::RGBA8Unorm;
    texture_desc.sampleCount = 1;
    texture_desc.mipLevelCount = 1;
    texture_desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

    texture_A = ctx.gpu.get_device().createTexture(texture_desc);
    texture_view_A = texture_A->createView();

    texture_B = ctx.gpu.get_device().createTexture(texture_desc);
    texture_view_B = texture_B->createView();

    // Sampler
    wgpu::SamplerDescriptor sampler_desc;
    sampler_desc.minFilter = wgpu::FilterMode::Linear;
    sampler_desc.magFilter = wgpu::FilterMode::Linear;
    sampler_desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
    sampler_desc.maxAnisotropy = 1;

    sampler = ctx.gpu.get_device().createSampler(sampler_desc);

    // Default uniforms
    wgpu::BufferDescriptor du_buffer_desc;
    du_buffer_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    du_buffer_desc.size = sizeof(DefaultUniforms);
    du_buffer_desc.mappedAtCreation = false;

    default_uniforms = ctx.gpu.get_device().createBuffer(du_buffer_desc);


    // Bind group layout
    wgpu::BindGroupLayoutEntry bgl_entries[3];
    // texture entry
    bgl_entries[0].binding = 0;
    bgl_entries[0].visibility = wgpu::ShaderStage::Fragment;
    bgl_entries[0].texture.sampleType = wgpu::TextureSampleType::Float;
    bgl_entries[0].texture.viewDimension = wgpu::TextureViewDimension::_2D;
    bgl_entries[0].texture.multisampled = false;
    // sampler entry
    bgl_entries[1].binding = 1;
    bgl_entries[1].visibility = wgpu::ShaderStage::Fragment;
    bgl_entries[1].sampler.type = wgpu::SamplerBindingType::Filtering;
    // default uniforms entry
    bgl_entries[2].binding = 2;
    bgl_entries[2].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    bgl_entries[2].buffer.type = wgpu::BufferBindingType::Uniform;
    bgl_entries[2].buffer.hasDynamicOffset = false;
    bgl_entries[2].buffer.minBindingSize = sizeof(DefaultUniforms);

    wgpu::BindGroupLayoutDescriptor bgl_desc;
    bgl_desc.entryCount = 3;
    bgl_desc.entries = bgl_entries;

    default_bind_group_layout = ctx.gpu.get_device().createBindGroupLayout(bgl_desc);

    // Bind groups
    // entries
    wgpu::BindGroupEntry bg_texture_A_entry;
    bg_texture_A_entry.binding = 0;
    bg_texture_A_entry.textureView = *texture_view_A;

    wgpu::BindGroupEntry bg_texture_B_entry;
    bg_texture_B_entry.binding = 0;
    bg_texture_B_entry.textureView = *texture_view_B;

    wgpu::BindGroupEntry bg_sampler_entry;
    bg_sampler_entry.binding = 1;
    bg_sampler_entry.sampler = *sampler;

    wgpu::BindGroupEntry bg_uniforms_entry;
    bg_uniforms_entry.binding = 2;
    bg_uniforms_entry.buffer = *default_uniforms;
    bg_uniforms_entry.offset = 0;
    bg_uniforms_entry.size = sizeof(DefaultUniforms);

    wgpu::BindGroupEntry bg_A_entries[3] = {bg_texture_A_entry, bg_sampler_entry, bg_uniforms_entry};
    wgpu::BindGroupEntry bg_B_entries[3] = {bg_texture_B_entry, bg_sampler_entry, bg_uniforms_entry};

    // descriptors
    wgpu::BindGroupDescriptor bg_A_desc;
    bg_A_desc.layout = *default_bind_group_layout;
    bg_A_desc.entryCount = 3;
    bg_A_desc.entries = bg_A_entries;

    wgpu::BindGroupDescriptor bg_B_desc;
    bg_B_desc.layout = *default_bind_group_layout;
    bg_B_desc.entryCount = 3;
    bg_B_desc.entries = bg_B_entries;

    // bind groups
    bind_group_A = ctx.gpu.get_device().createBindGroup(bg_A_desc);
    bind_group_B = ctx.gpu.get_device().createBindGroup(bg_B_desc);

    for (std::unique_ptr<ShaderUnion>& s : shaders) {
        s->apply([&](auto& s) { s.init_pipeline(ctx, *default_bind_group_layout); });
        if (s->is_current<Shader<ShaderKind::Image>>()) {
            s->get<Shader<ShaderKind::Image>>().set_render_dim(ctx.rendering.dim);
        }
    }
}


void ShaderManager::resize(unsigned int new_width, unsigned int new_height) {
    ctx.rendering.dim = std::array<unsigned int, 2>({new_width, new_height});
    init();
}


void ShaderManager::reorder_element(size_t index, size_t new_index) {
    if (index < new_index) {
        std::rotate(shaders.begin() + index, shaders.begin() + index + 1, shaders.begin() + new_index + 1);
    } else {
        std::rotate(shaders.begin() + new_index, shaders.begin() + index, shaders.begin() + index + 1);
    }
}


void ShaderManager::render() const {
    unsigned int& width = ctx.rendering.dim[0];
    unsigned int& height = ctx.rendering.dim[1];

    assert(*texture_view_A && *texture_view_B);
    wgpu::TextureView tv = *texture_view_A;
    DefaultUniforms du = {
        width,
        height,
        std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start_time).count()
    };

    wgpu::raii::Queue queue = ctx.gpu.get_device().getQueue();

    queue->writeBuffer(*default_uniforms, 0, &du, sizeof(du));
    for (size_t i = 0; i < shaders.size(); i++) {
        shaders[i]->apply([&](auto& shader) { shader.write_buffers(*queue); });
    }

    wgpu::RenderPassColorAttachment color_attachment;
    color_attachment.loadOp = wgpu::LoadOp::Clear;
    color_attachment.storeOp = wgpu::StoreOp::Store;
    color_attachment.clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
    color_attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    wgpu::RenderPassDescriptor render_pass_desc;
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachment;
    render_pass_desc.depthStencilAttachment = nullptr;

    wgpu::raii::CommandEncoder cmd_encoder = ctx.gpu.get_device().createCommandEncoder();

    // Clear previous render
    color_attachment.view = *texture_view_A;
    cmd_encoder->beginRenderPass(render_pass_desc).end();

    for (size_t i = 0; i < shaders.size(); i++) {
        tv = i % 2 ? *texture_view_A : *texture_view_B;
        wgpu::BindGroup default_bg = i % 2 ? *bind_group_B : *bind_group_A;

        color_attachment.view = tv;

        wgpu::raii::RenderPassEncoder pass_encoder = cmd_encoder->beginRenderPass(render_pass_desc);

        pass_encoder->setViewport(0.0f, 0.0f, width, height, 0.0f, 1.0f);
        pass_encoder->setScissorRect(0, 0, width, height);
        pass_encoder->setBindGroup(0, default_bg, 0, nullptr);
        shaders[i]->apply([&](auto& s) { s.set_bind_groups(*pass_encoder); });
        pass_encoder->setPipeline(shaders[i]->apply([](auto& s) { return s.get_render_pipeline(); }));
        pass_encoder->draw(3, 1, 0, 0);
        pass_encoder->end();
    }

    wgpu::raii::CommandBuffer cmd_buffer = cmd_encoder->finish();
    queue->submit(1, &(*cmd_buffer));

    // display rendered texture
    ImVec2 display_region = ImGui::GetContentRegionAvail();
    float start_x = ImGui::GetCursorPosX();
    float start_y = ImGui::GetCursorPosY();

    ImGui::InvisibleButton("##render region", display_region, ImGuiButtonFlags_MouseButtonLeft);

    if (ImGui::IsItemHovered()) {
        ImGuiIO& io = ImGui::GetIO();

        // Zoom with mouse wheel
        float zoom_delta = io.MouseWheel * 0.1f;
        if (zoom_delta != 0.0f) {
            display_state.zoom = std::clamp(display_state.zoom + zoom_delta, 0.1f, 100.0f);
        }

        // Pan with left-click drag
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            display_state.offset_x += io.MouseDelta.x / display_state.zoom;
            display_state.offset_y += io.MouseDelta.y / display_state.zoom;
        }
    }


    ImVec2 display_dim;
    ImVec2 tex_to_display_ratio(
        (display_region.x / width) * display_state.zoom,
        (display_region.y / height) * display_state.zoom
    );

    if (tex_to_display_ratio.x < tex_to_display_ratio.y) {
        display_dim.x = tex_to_display_ratio.x * width;
        display_dim.y = tex_to_display_ratio.x * height;
    } else {
        display_dim.x = tex_to_display_ratio.y * width;
        display_dim.y = tex_to_display_ratio.y * height;
    }

    ImGui::SetCursorPos(ImVec2(
        -(display_dim.x - display_region.x) * 0.5 + start_x + display_state.offset_x * display_state.zoom,
        -(display_dim.y - display_region.y) * 0.5 + start_y + display_state.offset_y * display_state.zoom
    ));


    ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<WGPUTextureView>(tv)), display_dim);
}


void ShaderManager::display() {
    int to_remove_idx = -1;  // store shader idx user decided to remove or -1 if no remove action
    for (size_t i = 0; i < shaders.size(); i++) {
        std::unique_ptr<ShaderUnion>& shader = shaders[i];

        const std::string shader_name = shader->apply([](auto& s) { return s.name; });

        ImGui::PushID(i);

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0);

        ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;
        ImGui::BeginChild(shader_name.c_str(), ImVec2(-1, 0), child_flags, ImGuiWindowFlags_MenuBar);

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.7);

        // Menu bar
        if (ImGui::BeginMenuBar()) {
            ImGui::Text("%s", shader_name.c_str());

            // Push all the way to the right
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 90);


            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));                     // Transparent when idle
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));  // Gray when hovered
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));  // Same as hovered (optional)

            ImGui::Button(ICON_FA_GRIP);

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("DND_SHADER_CELL", &i, sizeof(size_t));
                ImGui::Text("Move %s", shader_name.c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::SameLine();

            if (ImGui::Button(ICON_FA_ROTATE_LEFT)) {
                shader->apply([](auto& s) { s.reset(); });
            }

            ImGui::SameLine();

            if (ImGui::Button(ICON_FA_XMARK)) {
                to_remove_idx = i;
            }

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar(1);

            ImGui::EndMenuBar();
        }

        shader->apply([](auto& s) { return s.display(); });
        ImGui::PopItemWidth();
        ImGui::EndChild();

        ImGui::PopStyleVar(1);

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_SHADER_CELL")) {
                IM_ASSERT(payload->DataSize == sizeof(size_t));
                size_t payload_i = *(const size_t*)payload->Data;
                reorder_element(payload_i, i);
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::PopID();
    }
    if (to_remove_idx >= 0) {
        shaders.erase(shaders.begin() + to_remove_idx);
    }


    ImGui::BeginChild("add_shader");
    ImGui::Text("Select shader to add");
#define X(name) #name
    std::vector<std::string> shaders = {SHADER_VARIANTS};
#undef X
    bool change = ImGui::BeginCombo("shader", shaders[selected_shader].c_str());
    if (change) {
        for (unsigned int i = 0; i < shaders.size(); i++) {
            const bool is_selected = (i == selected_shader);

            if (ImGui::Selectable(shaders[i].c_str(), is_selected)) selected_shader = i;

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    creation_dialog(static_cast<ShaderKind>(selected_shader));
    ImGui::EndChild();

}
