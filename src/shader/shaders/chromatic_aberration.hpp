#pragma once

#include <imgui.h>

#include <webgpu/webgpu-raii.hpp>
#include <webgpu/webgpu.hpp>

#include "shaders_code.hpp"
#include "src/shader/parameter.hpp"
#include "src/shader/shader.hpp"

template <>
struct Shader<ShaderKind::ChromaticAbberation> : public ShaderBase<Shader<ShaderKind::ChromaticAbberation>> {
    enum class Mode: unsigned int {
        Uniform,
        LinearScaling,
        QuadraticScaling,
    };
    const char* modes[3] = {"Uniform", "LinearScaling", "QuadraticScaling"};

    struct alignas(16) Uniforms {
        union {
            struct {
                float uni_red_shift_x;
                float uni_red_shift_y;
                float uni_green_shift_x;
                float uni_green_shift_y;
                float uni_blue_shift_x;
                float uni_blue_shift_y;
                float scale_center_x;
                float scale_center_y;
                float scale_red_intensity;
                float scale_green_intensity;
                float scale_blue_intensity;
                Mode mode;
            };
            struct {
                float uni_red_shift[2];
                float uni_green_shift[2];
                float uni_blue_shift[2];
                float scale_center[2];
                float scale_intensity[3];
                unsigned int mode_id;
            };
        };
    };

    Uniforms uniforms = {{{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, Mode::Uniform}}};

    template <size_t N>
    using FField = Float<N, WidgetKind::DragField>;
    template <size_t N>
    using FieldNames = std::optional<std::array<std::string, N>>;
    using UniformModeFields = WidgetGroup<FField<2>, FField<2>, FField<2>>;
    using ScalingModeFields = WidgetGroup<FField<2>, FField<3>>;
    using Parameters = Choice<WidgetKind::Dropdown, UniformModeFields, ScalingModeFields>;

    Parameters parameters;

    static Parameters init_parameters(Uniforms& uniforms) {
        return Parameters(
            "mode",
            uniforms.mode_id,
            {"uniform", "linear scaling"},
            UniformModeFields(
                FField<2>(
                    "red",
                    {0.001, 0.001},
                    {0, 0},
                    {0, 0},
                    std::span<float, 2>(uniforms.uni_red_shift, 2),
                    FieldNames<2>({"x", "y"})
                ),
                FField<2>(
                    "green",
                    {0.001, 0.001},
                    {0, 0},
                    {0, 0},
                    std::span<float, 2>(uniforms.uni_green_shift, 2),
                    FieldNames<2>({"x", "y"})
                ),
                FField<2>(
                    "blue",
                    {0.001, 0.001},
                    {0, 0},
                    {0, 0},
                    std::span<float, 2>(uniforms.uni_blue_shift, 2),
                    FieldNames<2>({"x", "y"})
                )
            ),
            ScalingModeFields(
                FField<2>(
                    "center",
                    {0.001, 0.001},
                    {0, 0},
                    {0, 0},
                    std::span<float, 2>(uniforms.scale_center, 2),
                    FieldNames<2>({"x", "y"})
                ),
                FField<3>(
                    "intensity",
                    {0.001, 0.001, 0.001},
                    {0, 0, 0},
                    {0, 0, 0},
                    std::span<float, 3>(uniforms.scale_intensity, 3),
                    FieldNames<3>({"r", "g", "b"})
                )
            )
        );
    }

    wgpu::raii::BindGroupLayout bind_group_layout;
    wgpu::raii::Buffer buffer;
    wgpu::raii::BindGroup bind_group;


    Shader(const std::string& name)
        : ShaderBase<Shader<ShaderKind::ChromaticAbberation>>(name, fullscreen_vertex, chromatic_aberration),
          parameters(init_parameters(uniforms)) {}


    void init(const Context& ctx) {
        this->init_module(ctx);

        wgpu::BindGroupLayoutEntry bgl_entry;
        bgl_entry.binding = 0;
        bgl_entry.visibility = wgpu::ShaderStage::Fragment;
        bgl_entry.buffer.type = wgpu::BufferBindingType::Uniform;
        bgl_entry.buffer.hasDynamicOffset = false;
        bgl_entry.buffer.minBindingSize = sizeof(Uniforms);

        wgpu::BindGroupLayoutDescriptor bgl_desc;
        bgl_desc.entryCount = 1;
        bgl_desc.entries = &bgl_entry;
        bind_group_layout = ctx.get_device().createBindGroupLayout(bgl_desc);

        wgpu::BufferDescriptor buffer_desc;
        buffer_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        buffer_desc.size = sizeof(Uniforms);
        buffer_desc.mappedAtCreation = false;

        buffer = ctx.get_device().createBuffer(buffer_desc);

        wgpu::BindGroupEntry bg_uniforms_entry;
        bg_uniforms_entry.binding = 0;
        bg_uniforms_entry.buffer = *buffer;
        bg_uniforms_entry.offset = 0;
        bg_uniforms_entry.size = sizeof(Uniforms);

        wgpu::BindGroupDescriptor bg_desc;
        bg_desc.layout = *bind_group_layout;
        bg_desc.entryCount = 1;
        bg_desc.entries = &bg_uniforms_entry;

        bind_group = ctx.get_device().createBindGroup(bg_desc);
    }


    wgpu::raii::PipelineLayout make_pipeline_layout(
        const Context& ctx, const wgpu::BindGroupLayout& default_bind_group_layout
    ) {
        wgpu::raii::PipelineLayout pipeline_layout;

        WGPUBindGroupLayout bgls[2] = {default_bind_group_layout, *bind_group_layout};

        wgpu::PipelineLayoutDescriptor pipeline_layout_desc;
        pipeline_layout_desc.bindGroupLayoutCount = 2;
        pipeline_layout_desc.bindGroupLayouts = bgls;
        pipeline_layout = ctx.get_device().createPipelineLayout(pipeline_layout_desc);

        return pipeline_layout;
    }


    void display() const {
        parameters.display();
    }

    void reset() {
        uniforms = {{{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, Mode::Uniform}}};
    }

    void write_buffers(wgpu::Queue& queue) const {
        queue.writeBuffer(*buffer, 0, &uniforms, sizeof(uniforms));
    }

    void set_bind_groups(wgpu::RenderPassEncoder& pass_encoder) const {
        pass_encoder.setBindGroup(1, *bind_group, 0, nullptr);
    }
};
