#pragma once

#include <imgui.h>

#include <format>
#include <string>
#include <webgpu/webgpu-raii.hpp>

#include "src/gpu_context.hpp"
#include "src/log.hpp"
#include "webgpu/webgpu.hpp"

#define SHADER_VARIANTS X(NoParam), X(Circle)

#define X(name) name
enum class ShaderKind { SHADER_VARIANTS };
#undef X


template <typename Derived>
struct ShaderBase {
    const std::string name;
    const char* const vertex_code;
    const char* const frag_code;

    const wgpu::ShaderSourceWGSL vertex_source;
    const wgpu::ShaderSourceWGSL frag_source;

    const wgpu::ShaderModuleDescriptor vertex_module_desc;
    const wgpu::ShaderModuleDescriptor frag_module_desc;

    void release() { static_cast<Derived*>(this)->release(); }

    ShaderBase(ShaderBase& sb) = delete;
    ShaderBase(ShaderBase&& sb) = default;



    void display() { static_cast<Derived*>(this)->display(); }

    void write_buffers(wgpu::Queue& queue) const { static_cast<Derived*>(this)->write_buffers(queue); }

    void init(const GPUContext& ctx) { static_cast<Derived*>(this)->init(); }

    void init_module(const GPUContext& ctx) {
        vertex_module = ctx.get_device().createShaderModule(vertex_module_desc);
        frag_module = ctx.get_device().createShaderModule(frag_module_desc);
    }

    void init_pipeline(const GPUContext& ctx, const wgpu::BindGroupLayout& default_bind_group_layout) {
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
        wgpu::raii::PipelineLayout pipeline_layout = make_pipeline_layout(ctx, default_bind_group_layout);

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

    const wgpu::RenderPipeline get_render_pipeline() const { return *render_pipeline; }

  protected:
    ShaderBase(const std::string& name, const char* const vertex_code, const char* const frag_code)
        : name(name),
          vertex_code(vertex_code),
          frag_code(frag_code),
          vertex_source(make_source(vertex_code)),
          frag_source(make_source(frag_code)),
          vertex_module_desc(make_module_desc(vertex_source)),
          frag_module_desc(make_module_desc(frag_source)) {}

    wgpu::raii::ShaderModule vertex_module;
    wgpu::raii::ShaderModule frag_module;
    wgpu::raii::RenderPipeline render_pipeline;

    static const wgpu::ShaderSourceWGSL make_source(const char* const code) {
        wgpu::ShaderSourceWGSL source;
#ifdef __EMSCRIPTEN__
        source.code = code;
        source.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
#else
        source.code.data = code;
        source.code.length = WGPU_STRLEN;
        source.chain.sType = WGPUSType_ShaderSourceWGSL;
#endif
        source.chain.next = nullptr;

        return source;
    }

    static const wgpu::ShaderModuleDescriptor make_module_desc(const wgpu::ShaderSourceWGSL& source) {
        wgpu::ShaderModuleDescriptor module_desc;
        module_desc.nextInChain = &source.chain;
        return module_desc;
    }

    wgpu::raii::PipelineLayout make_pipeline_layout(
        const GPUContext& ctx, const wgpu::BindGroupLayout& default_bind_group_layout
    ) {
        return static_cast<Derived*>(this)->make_pipeline_layout(ctx, default_bind_group_layout);
    }
};


template <ShaderKind K>
struct Shader : public ShaderBase<Shader<K>> {
    Shader(const std::string& name, const char* const vertex_code, const char* const frag_code)
        : ShaderBase<Shader<K>>(name, vertex_code, frag_code) {}

    // functions to template specialize
    void init(const GPUContext& ctx);
    void display();
    void write_buffers(wgpu::Queue& queue) const;
    wgpu::raii::PipelineLayout make_pipeline_layout(
        const GPUContext& ctx, const wgpu::BindGroupLayout& default_bind_group_layout
    );
    void set_bind_groups(wgpu::RenderPassEncoder& pass_encoder) const;

    void release();
};


template <ShaderKind K>
void Shader<K>::init(const GPUContext& ctx) {
    this->init_module(ctx);
}

template <ShaderKind K>
void Shader<K>::release() {
    this->vertex_module->release();
    this->frag_module->release();
    this->render_pipeline->release();
}

template <ShaderKind K>
void Shader<K>::display() {}

template <ShaderKind K>
void Shader<K>::write_buffers(wgpu::Queue& queue) const {}

template <ShaderKind K>
wgpu::raii::PipelineLayout Shader<K>::make_pipeline_layout(
    const GPUContext& ctx, const wgpu::BindGroupLayout& default_bind_group_layout
) {
    wgpu::raii::PipelineLayout pipeline_layout;

    WGPUBindGroupLayout bgls[1] = {default_bind_group_layout};

    wgpu::PipelineLayoutDescriptor pipeline_layout_desc;
    pipeline_layout_desc.bindGroupLayoutCount = 1;
    pipeline_layout_desc.bindGroupLayouts = bgls;

    pipeline_layout = ctx.get_device().createPipelineLayout(pipeline_layout_desc);

    return pipeline_layout;
}


template <ShaderKind K>
void Shader<K>::set_bind_groups(wgpu::RenderPassEncoder& pass_encoder) const {}


#define X(name) Shader<ShaderKind::name>
using ShaderVariant = std::variant<SHADER_VARIANTS>;
#undef X
