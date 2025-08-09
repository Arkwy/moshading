#pragma once

#include <imgui.h>

#include <limits>
#include <webgpu/webgpu-raii.hpp>
#include <webgpu/webgpu.hpp>

#include "shaders_code.hpp"
#include "src/context.hpp"
#include "src/shader/parameter.hpp"
#include "src/shader/shader.hpp"

template <>
struct Shader<ShaderKind::Scanlines> : public ShaderBase<Shader<ShaderKind::Scanlines>> {
    constexpr static const char* const default_name = "scanlines";
    enum class Mode : unsigned int {
        Horizontal,
        Vertical,
    };
    const char* modes[2] = {"Horizontal", "Vertical"};


    struct alignas(16) Uniforms {
        float speed = 1000;
        float freq = 1;
        float shift[2] = {5, 0};
        float thikness = 10;
        Mode mode = Mode::Horizontal;
    };


    Uniforms uniforms{};

    template <size_t N>
    using FField = Float<N, WidgetKind::DragField>;
    template <size_t N>
    using FieldNames = std::optional<std::array<std::string, N>>;
    using ModeP = Choice<WidgetKind::Dropdown, void, void>;
    using Parameters = WidgetGroup<ModeP, FField<1>, FField<1>, FField<2>, FField<1>>;

    Parameters parameters;

    static Parameters init_parameters(Uniforms& uniforms) {
        return Parameters(
            ModeP("direction", reinterpret_cast<unsigned int&>(uniforms.mode), {"Horizontal", "Vertical"}),
            FField<1>("speed", {0.1}, {0}, {std::numeric_limits<float>::infinity()}, std::span<float, 1>(&uniforms.speed, 1)),
            FField<1>("frequency", {0.1}, {1}, {std::numeric_limits<float>::infinity()}, std::span<float, 1>(&uniforms.freq, 1)),
            FField<2>("shift", {0.01, 0.01}, {0, 0}, {0, 0}, std::span<float, 2>(uniforms.shift, 2), FieldNames<2>({"x", "y"})),
            FField<1>("thickness", {0.01}, {0}, {std::numeric_limits<float>::infinity()}, std::span<float, 1>(&uniforms.thikness, 1))
        );
    }


    wgpu::raii::BindGroupLayout bind_group_layout;
    wgpu::raii::Buffer buffer;
    wgpu::raii::BindGroup bind_group;


    Shader(const std::string& name, const Context& ctx)
        : ShaderBase<Shader<ShaderKind::Scanlines>>(name, ctx.shader_source_cache.get(fullscreen_vertex), ctx.shader_source_cache.get(scanlines), ctx),
          parameters(init_parameters(uniforms)) {}


    void init() {
        wgpu::BindGroupLayoutEntry bgl_entry;
        bgl_entry.binding = 0;
        bgl_entry.visibility = wgpu::ShaderStage::Fragment;
        bgl_entry.buffer.type = wgpu::BufferBindingType::Uniform;
        bgl_entry.buffer.hasDynamicOffset = false;
        bgl_entry.buffer.minBindingSize = sizeof(Uniforms);

        wgpu::BindGroupLayoutDescriptor bgl_desc;
        bgl_desc.entryCount = 1;
        bgl_desc.entries = &bgl_entry;
        bind_group_layout = ctx.gpu.get_device().createBindGroupLayout(bgl_desc);

        wgpu::BufferDescriptor buffer_desc;
        buffer_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        buffer_desc.size = sizeof(Uniforms);
        buffer_desc.mappedAtCreation = false;

        buffer = ctx.gpu.get_device().createBuffer(buffer_desc);

        wgpu::BindGroupEntry bg_uniforms_entry;
        bg_uniforms_entry.binding = 0;
        bg_uniforms_entry.buffer = *buffer;
        bg_uniforms_entry.offset = 0;
        bg_uniforms_entry.size = sizeof(Uniforms);

        wgpu::BindGroupDescriptor bg_desc;
        bg_desc.layout = *bind_group_layout;
        bg_desc.entryCount = 1;
        bg_desc.entries = &bg_uniforms_entry;

        bind_group = ctx.gpu.get_device().createBindGroup(bg_desc);
    }


    wgpu::raii::PipelineLayout make_pipeline_layout(
        const Context& ctx, const wgpu::BindGroupLayout& default_bind_group_layout
    ) {
        wgpu::raii::PipelineLayout pipeline_layout;

        WGPUBindGroupLayout bgls[2] = {default_bind_group_layout, *bind_group_layout};

        wgpu::PipelineLayoutDescriptor pipeline_layout_desc;
        pipeline_layout_desc.bindGroupLayoutCount = 2;
        pipeline_layout_desc.bindGroupLayouts = bgls;
        pipeline_layout = ctx.gpu.get_device().createPipelineLayout(pipeline_layout_desc);

        return pipeline_layout;
    }


    void display() const {
        parameters.display();
    }


    void reset() {
        uniforms = {};
    }


    void write_buffers(wgpu::Queue& queue) const {
        queue.writeBuffer(*buffer, 0, &uniforms, sizeof(uniforms));
    }


    void set_bind_groups(wgpu::RenderPassEncoder& pass_encoder) const {
        pass_encoder.setBindGroup(1, *bind_group, 0, nullptr);
    }
};
