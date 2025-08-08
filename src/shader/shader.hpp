#pragma once

#include <imgui.h>

#include <memory>
#include <string>
#include <webgpu/webgpu-raii.hpp>

#include "src/context.hpp"
#include "src/tagged_union.hpp"

#define SHADER_KINDS X(ChromaticAbberation), X(Image), X(Noise), X(Dithering)

#define X(name) name
enum class ShaderKind { SHADER_KINDS };
#undef X


template <typename Derived>
struct ShaderBase {
    constexpr static const ResourceKind RESOURCES[0] = {};
    constexpr static const char* const default_name = "unamed shader";
    const std::shared_ptr<void> lifetime_token; // lifetime tracker used for auto unsubscription to resources updates

    const Context& ctx;
    std::string name;
    const ShaderSource& vertex_source;
    const ShaderSource& frag_source;

    ShaderBase(const ShaderBase<Derived>& sb) = delete;
    ShaderBase(ShaderBase<Derived>&& sb) = delete;

    void init_pipeline(const wgpu::BindGroupLayout& default_bind_group_layout) {
        wgpu::VertexState vertex_state;
        vertex_state.module = *vertex_source.compiled_module;
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
        frag_state.module = *frag_source.compiled_module;
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

        render_pipeline = ctx.gpu.get_device().createRenderPipeline(pipeline_desc);
    }

    const wgpu::RenderPipeline get_render_pipeline() const {
        return *render_pipeline;
    }

  protected:
    wgpu::raii::RenderPipeline render_pipeline;

    ShaderBase(
        const std::string& name, const ShaderSource& vertex_source, const ShaderSource& frag_source, const Context& ctx
    )
        :  lifetime_token(), ctx(ctx), name(name), vertex_source(vertex_source), frag_source(frag_source) {}

    wgpu::raii::PipelineLayout make_pipeline_layout(
        const Context& ctx, const wgpu::BindGroupLayout& default_bind_group_layout
    ) {
        return static_cast<Derived*>(this)->make_pipeline_layout(ctx, default_bind_group_layout);
    }
};


template <ShaderKind K>
struct Shader : public ShaderBase<Shader<K>> {
    Shader(
        const std::string& name, const ShaderSource& vertex_source, const ShaderSource& frag_source, const Context& ctx
    )
        : ShaderBase<Shader<K>>(name, vertex_source, frag_source, ctx) {}

    // functions to template specialize
    void init();
    void display();
    void write_buffers(wgpu::Queue& queue) const;
    wgpu::raii::PipelineLayout make_pipeline_layout(
        const Context& ctx, const wgpu::BindGroupLayout& default_bind_group_layout
    );
    void set_bind_groups(wgpu::RenderPassEncoder& pass_encoder) const;

    void reset();
};

template <ShaderKind K>
void Shader<K>::init() {}

template <ShaderKind K>
void Shader<K>::reset() {}

template <ShaderKind K>
void Shader<K>::display() {}

template <ShaderKind K>
void Shader<K>::write_buffers(wgpu::Queue& _) const {}

template <ShaderKind K>
wgpu::raii::PipelineLayout Shader<K>::make_pipeline_layout(
    const Context& ctx, const wgpu::BindGroupLayout& default_bind_group_layout
) {
    wgpu::raii::PipelineLayout pipeline_layout;

    WGPUBindGroupLayout bgls[1] = {default_bind_group_layout};

    wgpu::PipelineLayoutDescriptor pipeline_layout_desc;
    pipeline_layout_desc.bindGroupLayoutCount = 1;
    pipeline_layout_desc.bindGroupLayouts = bgls;

    pipeline_layout = ctx.gpu.get_device().createPipelineLayout(pipeline_layout_desc);

    return pipeline_layout;
}


template <ShaderKind K>
void Shader<K>::set_bind_groups(wgpu::RenderPassEncoder& _) const {}


#define X(name) Shader<ShaderKind::name>
using ShaderUnion = TaggedUnion<SHADER_KINDS>;
#undef X


template <typename T, typename Variant>
struct is_in_tagged_union;

template <typename T, typename... Types>
struct is_in_tagged_union<T, TaggedUnion<Types...>> : std::bool_constant<(std::is_same_v<T, Types> || ...)> {};

template <typename T>
concept ShaderUnionConcept = is_in_tagged_union<T, ShaderUnion>::value;
