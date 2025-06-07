#include "manager.hpp"

#include <imgui.h>

#include <algorithm>
#include <chrono>
#include <format>
#include <memory>
#include <webgpu/webgpu.hpp>

#include "imgui_internal.h"
#include "shader.hpp"
#include "src/log.hpp"


InnerShader::InnerShader(  // debug constructor, to be removed
    const ShaderManager& manager,
    const std::string& name,
    const char* const vertex_code,
    const char* const frag_code
)
    : shader(std::make_unique<Shader>(name, vertex_code, frag_code)) {
    init_module(manager);
    init_pipeline(manager);
}


InnerShader::InnerShader(const ShaderManager& manager, std::unique_ptr<Shader> shader) : shader(std::move(shader)) {
    init_module(manager);
    init_pipeline(manager);
}


void InnerShader::init_module(const ShaderManager& manager) {
#ifdef __EMSCRIPTEN__
    wgpu::ShaderModuleWGSLDescriptor vertex_source;
    vertex_source.code = shader->vertex_code;
    vertex_source.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
#else
    wgpu::ShaderSourceWGSL vertex_source;
    vertex_source.code.data = shader->vertex_code;
    vertex_source.code.length = WGPU_STRLEN;
    vertex_source.chain.sType = WGPUSType_ShaderSourceWGSL;
#endif
    vertex_source.chain.next = nullptr;

    wgpu::ShaderModuleDescriptor vertex_module_desc;
    vertex_module_desc.nextInChain = &vertex_source.chain;

    vertex_module = manager.ctx.get_device().createShaderModule(vertex_module_desc);


#ifdef __EMSCRIPTEN__
    wgpu::ShaderModuleWGSLDescriptor frag_source;
    frag_source.code = shader->frag_code;
    frag_source.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
#else
    wgpu::ShaderSourceWGSL frag_source;
    frag_source.code.data = shader->frag_code;
    frag_source.code.length = WGPU_STRLEN;
    frag_source.chain.sType = WGPUSType_ShaderSourceWGSL;
#endif
    frag_source.chain.next = nullptr;

    wgpu::ShaderModuleDescriptor frag_module_desc;
    frag_module_desc.nextInChain = &frag_source.chain;

    frag_module = manager.ctx.get_device().createShaderModule(frag_module_desc);
}


void InnerShader::init_pipeline(const ShaderManager& manager) {
    wgpu::VertexState vertex_state;
    vertex_state.module = *vertex_module;
#ifdef __EMSCRIPTEN__
    vertex_state.entryPoint = "vs_main";
#else
    vertex_state.entryPoint.data = "vs_main";
    vertex_state.entryPoint.length = WGPU_STRLEN;
#endif
    vertex_state.bufferCount = 0;
    vertex_state.buffers = nullptr;
    vertex_state.constantCount = 0;
    vertex_state.constants = nullptr;

    wgpu::ColorTargetState color_target;
    color_target.format = wgpu::TextureFormat::RGBA8Unorm;
    color_target.writeMask = wgpu::ColorWriteMask::All;
    color_target.blend = nullptr;

    wgpu::FragmentState frag_state;
    frag_state.module = *frag_module;
#ifdef __EMSCRIPTEN__
    frag_state.entryPoint = "fs_main";
#else
    frag_state.entryPoint.data = "fs_main";
    frag_state.entryPoint.length = WGPU_STRLEN;
#endif
    frag_state.constantCount = 0;
    frag_state.constants = nullptr;
    frag_state.targetCount = 1;
    frag_state.targets = &color_target;

    // // render pipeline setup
    WGPUBindGroupLayout bgls[1] = {*manager.bind_group_layout};
    wgpu::PipelineLayoutDescriptor pipeline_layout_desc;
    pipeline_layout_desc.bindGroupLayoutCount = 1;
    pipeline_layout_desc.bindGroupLayouts = bgls;

    wgpu::raii::PipelineLayout pipeline_layout = manager.ctx.get_device().createPipelineLayout(pipeline_layout_desc);

    wgpu::RenderPipelineDescriptor pipeline_desc;
    pipeline_desc.layout = *pipeline_layout;
    pipeline_desc.vertex = vertex_state;
    pipeline_desc.fragment = &frag_state;
    pipeline_desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipeline_desc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
    pipeline_desc.primitive.frontFace = wgpu::FrontFace::CCW;
    pipeline_desc.primitive.cullMode = wgpu::CullMode::None;
    pipeline_desc.depthStencil = nullptr;
    pipeline_desc.multisample.count = 1;
    pipeline_desc.multisample.mask = ~0u;
    pipeline_desc.multisample.alphaToCoverageEnabled = false;

    render_pipeline = manager.ctx.get_device().createRenderPipeline(pipeline_desc);
}



ShaderManager::ShaderManager(const GPUContext& ctx, unsigned int width, unsigned int height)
    : ctx(ctx), shaders(), width(width), height(height) {
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
    texture_desc.size.width = width;
    texture_desc.size.height = height;
    texture_desc.size.depthOrArrayLayers = 1;
    texture_desc.format = wgpu::TextureFormat::RGBA8Unorm;
    texture_desc.sampleCount = 1;
    texture_desc.mipLevelCount = 1;
    texture_desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

    texture_A = ctx.get_device().createTexture(texture_desc);
    texture_view_A = texture_A->createView();

    texture_B = ctx.get_device().createTexture(texture_desc);
    texture_view_B = texture_B->createView();

    // Sampler
    wgpu::SamplerDescriptor sampler_desc;
    sampler_desc.minFilter = wgpu::FilterMode::Linear;
    sampler_desc.magFilter = wgpu::FilterMode::Linear;
    sampler_desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;  // Optional, no mipmaps used
    sampler_desc.maxAnisotropy = 1;

    sampler = ctx.get_device().createSampler(sampler_desc);

    // Default uniforms
    wgpu::BufferDescriptor du_buffer_desc;
    du_buffer_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    du_buffer_desc.size = sizeof(DefaultUniforms);
    du_buffer_desc.mappedAtCreation = false;

    default_uniforms = ctx.get_device().createBuffer(du_buffer_desc);


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

    bind_group_layout = ctx.get_device().createBindGroupLayout(bgl_desc);

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
    bg_A_desc.layout = *bind_group_layout;
    bg_A_desc.entryCount = 3;
    bg_A_desc.entries = bg_A_entries;

    wgpu::BindGroupDescriptor bg_B_desc;
    bg_B_desc.layout = *bind_group_layout;
    bg_B_desc.entryCount = 3;
    bg_B_desc.entries = bg_B_entries;

    // bind groups
    bind_group_A = ctx.get_device().createBindGroup(bg_A_desc);
    bind_group_B = ctx.get_device().createBindGroup(bg_B_desc);
}


void ShaderManager::resize(unsigned int new_width, unsigned int new_height) {
    width = new_width;
    height = new_height;

    init();

    auto old_shaders = std::move(shaders);
    shaders = std::vector<InnerShader>();
    shaders.reserve(old_shaders.size());
    for (size_t i = 0; i < old_shaders.size(); i++) {
        add_shader(std::move(old_shaders[i].shader));
    }
}


void ShaderManager::add_shader(const std::string& name, const char* const vertex_code, const char* const frag_code) {
    shaders.push_back(InnerShader(*this, name, vertex_code, frag_code));
}


void ShaderManager::add_shader(std::unique_ptr<Shader> shader) {
    shaders.push_back(InnerShader(*this, std::move(shader)));
}


void ShaderManager::reorder_element(size_t index, size_t new_index) {
    if (index < new_index) {
        std::rotate(shaders.begin() + index, shaders.begin() + index + 1, shaders.begin() + new_index + 1);
    } else {
        std::rotate(shaders.begin() + new_index, shaders.begin() + index, shaders.begin() + index + 1);
    }
}


void ShaderManager::render() const {
    assert(*texture_view_A && *texture_view_B);
    assert(shaders.size() > 0);
    wgpu::TextureView tv;
    DefaultUniforms du = {
        width,
        height,
        std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start_time).count()
    };

    wgpu::raii::Queue queue = ctx.get_device().getQueue();

    queue->writeBuffer(*default_uniforms, 0, &du, sizeof(du));

    wgpu::RenderPassColorAttachment color_attachment;
    color_attachment.loadOp = wgpu::LoadOp::Clear;
    color_attachment.storeOp = wgpu::StoreOp::Store;
    color_attachment.clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
    color_attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    wgpu::RenderPassDescriptor render_pass_desc;
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachment;
    render_pass_desc.depthStencilAttachment = nullptr;

    for (size_t i = 0; i < shaders.size(); i++) {  // TODO chache color attachment(s?) and render pass descriptor(s?)
        tv = i % 2 ? *texture_view_A : *texture_view_B;
        wgpu::BindGroup bg = i % 2 ? *bind_group_B : *bind_group_A;

        color_attachment.view = tv;


        wgpu::raii::CommandEncoder cmd_encoder = ctx.get_device().createCommandEncoder();
        wgpu::raii::RenderPassEncoder pass_encoder = cmd_encoder->beginRenderPass(render_pass_desc);

        pass_encoder->setViewport(0.0f, 0.0f, width, height, 0.0f, 1.0f);
        pass_encoder->setScissorRect(0, 0, width, height);
        pass_encoder->setBindGroup(0, bg, 0, nullptr);
        pass_encoder->setPipeline(*shaders[i].render_pipeline);
        pass_encoder->draw(3, 1, 0, 0);
        pass_encoder->end();

        wgpu::raii::CommandBuffer cmd_buffer = cmd_encoder->finish();

        queue->submit(1, &(*cmd_buffer));
    }
    ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<WGPUTextureView>(tv)), ImVec2(width, height));
}


void ShaderManager::display() {
    int to_remode_idx = -1; // store shader idx user decided to remove or -1 if no remove action
    for (size_t i = 0; i < shaders.size(); i++) {
        std::unique_ptr<Shader>& shader = shaders[i].shader;

        ImGui::PushID(i);

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0);

        ImGuiChildFlags child_flags = ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY;
        ImGui::BeginChild(shader->name.c_str(), ImVec2(-1, 0), child_flags, ImGuiWindowFlags_MenuBar);


        // Menu bar
        if (ImGui::BeginMenuBar()) {
            ImGui::Text("%s", shader->name.c_str());

            // Push all the way to the right
            float icon_width = ImGui::CalcTextSize("--").x + ImGui::CalcTextSize("x").x;  // if using icon font
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 3 * icon_width);

            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));                  // Transparent when idle
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));  // Gray when hovered
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));   // Same as hovered (optional)

            ImGui::Button("--");

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("DND_SHADER_CELL", &i, sizeof(size_t));
                ImGui::Text("Move %s", shader->name.c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::SameLine();

            if (ImGui::Button("x")) {
                to_remode_idx = i;
            }

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar(1);

            ImGui::EndMenuBar();
        }

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

    if (to_remode_idx >= 0) {
        shaders.erase(shaders.begin() + to_remode_idx);
    }
}
