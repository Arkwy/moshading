#include <backends/imgui_impl_wgpu.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <sys/types.h>
#include <webgpu/webgpu.h>

#include <cstring>
#include <string>
#include <webgpu/webgpu-raii.hpp>

#include "app.hpp"
#include "log.hpp"

#include "shaders_code.hpp"


ImVec2 side_base_size(const ImVec2& window_size) { return ImVec2(window_size.x / 4., window_size.y); };

ImVec2 center_base_size(const ImVec2& window_size) { return ImVec2(window_size.x / 2., window_size.y); };

extern const char shader[];

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

        // ImGui::DockBuilderAddNode(dock_id_left, ImGuiDockNodeFlags_NoUndocking);

        ImGui::DockBuilderDockWindow("input_manager", dock_id_left);
        ImGui::DockBuilderDockWindow("shader_manager", dock_id_right);
        ImGui::DockBuilderDockWindow("shader_display", dockspace_id);
        ImGui::DockBuilderFinish(dockspace_id);

        initiliazed = true;
    }

    ImGui::Begin("input_manager");
    ImGui::End();

    ImGui::Begin("shader_manager");
    ImGui::End();

    ImGui::Begin("shader_display");
    render_shader();
    ImGui::End();
}



bool App::render_shader() {


    if (shader_render_height < 1 || shader_render_width < 1) {
        Log::error("Render widtdh and height should be greater than 0.");
        return false;
    }

    // (re)create texture
    static ImVec2 previous_render_dim(0, 0);
    if (previous_render_dim.x != shader_render_width || previous_render_dim.y != shader_render_height) {
        Log::log("create tex");
        previous_render_dim.x = shader_render_width;
        previous_render_dim.y = shader_render_height;
        wgpu::TextureDescriptor tex_desc = {};
#ifdef __EMSCRIPTEN__
        tex_desc.label = "shader_render";
#else
        tex_desc.label.data = "shader_render";
        tex_desc.label.length = WGPU_STRLEN;
#endif
        tex_desc.size.width = shader_render_width;
        tex_desc.size.height = shader_render_height;
        tex_desc.size.depthOrArrayLayers = 1;
        tex_desc.format = wgpu::TextureFormat::RGBA8Unorm;
        tex_desc.sampleCount = 1;
        tex_desc.mipLevelCount = 1;
        tex_desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

        tex = ctx.get_device().createTexture(tex_desc);
        if (*tex_view) {
            Log::log("destroy tex view");
            tex_view->release();
            *tex_view = nullptr;
        }
    }

    // offscreen rendering
    if (!*tex_view) {
        Log::log("create tex view");
        tex_view = tex->createView();
    }

    wgpu::RenderPassColorAttachment color_attachment = {};
    color_attachment.view = *tex_view;
    color_attachment.loadOp = wgpu::LoadOp::Clear;
    color_attachment.storeOp = wgpu::StoreOp::Store;
    color_attachment.clearValue = {0.4f, 0.2f, time/1000.f, 1.0f};
    color_attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    wgpu::RenderPassDescriptor render_pass_desc = {};
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachment;
    render_pass_desc.depthStencilAttachment = nullptr;

    wgpu::raii::CommandEncoder cmd_encoder = ctx.get_device().createCommandEncoder();
    wgpu::raii::RenderPassEncoder pass_encoder = cmd_encoder->beginRenderPass(render_pass_desc);

    // // // shader setup
    if (!*shader_module) {

#ifdef __EMSCRIPTEN__
        wgpu::ShaderModuleWGSLDescriptor shader_source;
        shader_source.code = shader;
        shader_source.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
#else
        wgpu::ShaderSourceWGSL shader_source;
        shader_source.code.data = shader;
        shader_source.code.length = WGPU_STRLEN;
        shader_source.chain.sType = WGPUSType_ShaderSourceWGSL;
#endif
        shader_source.chain.next = nullptr;

        wgpu::ShaderModuleDescriptor shader_module_desc;
        shader_module_desc.nextInChain = &shader_source.chain;

        shader_module = ctx.get_device().createShaderModule(shader_module_desc);
    }

    if (!*render_pipeline) {
        wgpu::VertexState vertex_state;
        vertex_state.module = *shader_module;
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
        frag_state.module = *shader_module;
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
        wgpu::PipelineLayoutDescriptor pipeline_layout_desc;
        pipeline_layout_desc.bindGroupLayoutCount = 0;
        pipeline_layout_desc.bindGroupLayouts = nullptr;

        wgpu::raii::PipelineLayout pipeline_layout = ctx.get_device().createPipelineLayout(pipeline_layout_desc);

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

        render_pipeline = ctx.get_device().createRenderPipeline(pipeline_desc);
    }

    // // start rendering
    pass_encoder->setViewport(0.0f, 0.0f, shader_render_width, shader_render_height, 0.0f, 1.0f);
    pass_encoder->setScissorRect(0, 0, shader_render_width, shader_render_height);
    pass_encoder->setPipeline(*render_pipeline);
    pass_encoder->draw(3, 1, 0, 0);  // draws a triangle
    pass_encoder->end();

    wgpu::raii::CommandBuffer cmd_buffer = cmd_encoder->finish();

    wgpu::raii::Queue queue = ctx.get_device().getQueue();
    queue->submit(1, &(*cmd_buffer));


    ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<WGPUTextureView>(*tex_view)), ImVec2(shader_render_width, shader_render_height));
    time+=1;

    return true;
}

#ifdef __EMSCRIPTEN__
#else
#endif

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
