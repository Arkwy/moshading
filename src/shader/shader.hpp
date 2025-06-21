#pragma once

#include <optional>
#include <string>
#include <webgpu/webgpu-raii.hpp>

#include "src/gpu_context.hpp"

// #include "parameter.hpp"

#define SHADER_VARIANTS X(NoParam), X(Circle)

#define X(name) name
enum class ShaderKind { SHADER_VARIANTS };
#undef X

// Shader::Shader(ShaderVariant&& shader) : shader_desc(std::move(shader)) {}


// void Shader::init_module(const GPUContext& ctx)


// void Shader::init_pipeline(const GPUContext& ctx, const wgpu::BindGroupLayout& default_bind_group_layout)


template <typename Derived>
struct ShaderBase {
    const std::string name;
    const char* const vertex_code;
    const char* const frag_code;

    const wgpu::ShaderSourceWGSL vertex_source;
    const wgpu::ShaderSourceWGSL frag_source;

    const wgpu::ShaderModuleDescriptor vertex_module_desc;
    const wgpu::ShaderModuleDescriptor frag_module_desc;

    const std::optional<wgpu::BindGroupLayoutDescriptor> bind_group_layout_desc;


    ShaderBase(const std::string& name, const char* const vertex_code, const char* const frag_code)
        : name(name),
          vertex_code(vertex_code),
          frag_code(frag_code),
          vertex_source(make_source(vertex_code)),
          frag_source(make_source(frag_code)),
          vertex_module_desc(make_module_desc(vertex_source)),
          frag_module_desc(make_module_desc(frag_source)),
          bind_group_layout_desc(make_bind_group_layout_desc()) {}

    ~ShaderBase() = default;

    void display() { static_cast<Derived*>(this)->display(); }

    void write_buffers(wgpu::Queue& queue) { static_cast<Derived*>(this)->write_buffers(queue); }

    void init_module(const GPUContext& ctx) {
        vertex_module =
            ctx.get_device().createShaderModule(vertex_module_desc);
        frag_module =
            ctx.get_device().createShaderModule(frag_module_desc);
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
        wgpu::raii::PipelineLayout pipeline_layout;
        if (bind_group_layout_desc.has_value()) {
            shader_specific_bind_group_layout =
                ctx.get_device().createBindGroupLayout(bind_group_layout_desc.value());

            WGPUBindGroupLayout bgls[2] = {default_bind_group_layout, *shader_specific_bind_group_layout.value()};

            wgpu::PipelineLayoutDescriptor pipeline_layout_desc;
            pipeline_layout_desc.bindGroupLayoutCount = 2;
            pipeline_layout_desc.bindGroupLayouts = bgls;
            pipeline_layout = ctx.get_device().createPipelineLayout(pipeline_layout_desc);
        } else {
            WGPUBindGroupLayout bgls[1] = {default_bind_group_layout};

            wgpu::PipelineLayoutDescriptor pipeline_layout_desc;
            pipeline_layout_desc.bindGroupLayoutCount = 1;
            pipeline_layout_desc.bindGroupLayouts = bgls;
            pipeline_layout = ctx.get_device().createPipelineLayout(pipeline_layout_desc);
        }

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

    const wgpu::RenderPipeline get_render_pipeline() const {
        return *render_pipeline;
    }

  private:
    wgpu::raii::ShaderModule vertex_module;
    wgpu::raii::ShaderModule frag_module;
    std::optional<wgpu::raii::BindGroupLayout> shader_specific_bind_group_layout = std::nullopt;
    std::optional<wgpu::raii::BindGroup> shader_specific_bind_group = std::nullopt;
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


    static const std::optional<wgpu::BindGroupLayoutDescriptor> make_bind_group_layout_desc() {
        return Derived::make_bind_group_layout_desc();
    }
};


template <ShaderKind K>
struct Shader : public ShaderBase<Shader<K>> {

    Shader(const std::string& name, const char* const vertex_code, const char* const frag_code)
        : ShaderBase<Shader<K>>(name, vertex_code, frag_code) {}

    // functions to template specialize
    void display();
    void write_buffers(wgpu::Queue& queue);
    static const std::optional<wgpu::BindGroupLayoutDescriptor> make_bind_group_layout_desc();
    const std::optional<wgpu::BindGroupDescriptor> make_bind_group_desc() const;
};


template <ShaderKind K>
void Shader<K>::display() {}


template <ShaderKind K>
void Shader<K>::write_buffers(wgpu::Queue& queue) {}


template <ShaderKind K>
const std::optional<wgpu::BindGroupLayoutDescriptor> Shader<K>::make_bind_group_layout_desc() {
    return std::nullopt;
}


#define X(name) Shader<ShaderKind::name>
using ShaderVariant = std::variant<SHADER_VARIANTS>;
#undef X


// struct Shader {

//     wgpu::raii::ShaderModule vertex_module;
//     wgpu::raii::ShaderModule frag_module;
//     std::optional<wgpu::raii::BindGroupLayout> shader_specific_bind_group_layout = std::nullopt;
//     std::optional<wgpu::raii::BindGroup> shader_specific_bind_group = std::nullopt;
//     wgpu::raii::RenderPipeline render_pipeline;

//     Shader(Shader&& other) noexcept
//         : shader_desc(std::move(other.shader_desc)),
//           vertex_module(std::move(other.vertex_module)),
//           frag_module(std::move(other.frag_module)),
//           render_pipeline(std::move(other.render_pipeline)) {}

//     // Shader& operator=(Shader&& other) noexcept {
//     //     if (this != &other) {
//     //         shader_desc = std::move(other.shader_desc);
//     //         vertex_module = std::move(other.vertex_module);
//     //         frag_module = std::move(other.frag_module);
//     //         render_pipeline = std::move(other.render_pipeline);
//     //     }
//     //     return *this;
//     // }

//     // Deleted copy constructor and copy assignment operator
//     Shader(const Shader&) = delete;
//     Shader& operator=(const Shader&) = delete;

//     Shader(ShaderVariant&& shader);

//     void init_module(const GPUContext& ctx);
//     void init_pipeline(const GPUContext& ctx, const wgpu::BindGroupLayout& default_bind_group_layout);
//     // void init_bind_group(const ShaderManager& manager);
// };
