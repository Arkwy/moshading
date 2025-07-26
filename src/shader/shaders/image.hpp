#pragma once

#include <imgui.h>
#include <stb/stb_image.h>

#include <cmath>
#include <cstddef>
#include <optional>
#include <webgpu/webgpu-raii.hpp>

#include "shaders_code.hpp"
#include "src/context.hpp"
#include "src/shader/parameter.hpp"
#include "src/shader/shader.hpp"
#include "webgpu/webgpu.hpp"


template <>
struct Shader<ShaderKind::Image> : public ShaderBase<Shader<ShaderKind::Image>> {
    constexpr static const ResourceKind RESOURCES[1] = {ResourceKind::Image};
    struct alignas(16) Uniforms {
        union {
            struct {
                float size_x;
                float size_y;
                float pos_x;
                float pos_y;
                float rotation;
                float opacity;
            };
            struct {
                float size[2];
                float pos[2];
                float _;
                float _;
            };
            float raw[6];
        };
    };

    const size_t image_index;

    Uniforms uniforms = {{{0.0, 0.0, 0.0, 0.0, 0.0, 1.0}}};

    wgpu::raii::BindGroupLayout bind_group_layout;
    wgpu::raii::Buffer buffer;
    wgpu::raii::BindGroup bind_group;

    int base_height = -1;
    int base_width = -1;

    std::array<unsigned int, 2> render_dim = {0, 0};

    void set_render_dim(const std::array<unsigned int, 2>& dim) {
        render_dim = dim;
        std::get<2>(parameters.widgets).max = {static_cast<float>(render_dim[0]), static_cast<float>(render_dim[1])};
    }

    template <size_t N>
    using FField = Float<N, WidgetKind::DragField>;
    template <size_t N>
    using FieldNames = std::optional<std::array<std::string, N>>;
    using Parameters = WidgetGroup<FField<2>, FField<2>, Box2D, FField<1>, FField<1>>;

    Parameters parameters;

    Shader(const std::string& name, const size_t& image_index, const Context& ctx)
        : ShaderBase<Shader<ShaderKind::Image>>(
              name, ctx.shader_source_cache.get(fullscreen_vertex), ctx.shader_source_cache.get(image), ctx
          ),
          image_index(image_index),
          parameters(init_parameters(uniforms, render_dim)) {
        ctx.resource_manager.get_image(image_index).subscribe([&](){update_bind_group();});
    }


    static Parameters init_parameters(Uniforms& uniforms, const std::array<unsigned int, 2>& render_dim) {
        return Parameters(
            FField<2>("size", {1, 1}, {0, 0}, {0, 0}, std::span<float, 2>(uniforms.size, 2), FieldNames<2>({"x", "y"})),
            FField<
                2>("position", {1, 1}, {0, 0}, {0, 0}, std::span<float, 2>(uniforms.pos, 2), FieldNames<2>({"x", "y"})),
            Box2D(
                "##position",
                {0, 0},
                {static_cast<float>(render_dim[0]), static_cast<float>(render_dim[1])},
                true,
                uniforms.pos,
                uniforms.size,
                uniforms.rotation
            ),
            FField<1>("rotation", {0.01}, {0}, {0}, std::span<float, 1>(&uniforms.rotation, 1)),
            FField<1>("opacity", {0.01}, {0}, {1}, std::span<float, 1>(&uniforms.opacity, 1))
        );
    }


    void init() {
        set_render_dim(ctx.render_target.dim);

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
        // uniforms entry
        bgl_entries[2].binding = 2;
        bgl_entries[2].visibility = wgpu::ShaderStage::Fragment;
        bgl_entries[2].buffer.type = wgpu::BufferBindingType::Uniform;
        bgl_entries[2].buffer.hasDynamicOffset = false;
        bgl_entries[2].buffer.minBindingSize = sizeof(Uniforms);

        wgpu::BindGroupLayoutDescriptor bgl_desc;
        bgl_desc.entryCount = 3;
        bgl_desc.entries = bgl_entries;

        bind_group_layout = ctx.gpu.get_device().createBindGroupLayout(bgl_desc);

        wgpu::BufferDescriptor buffer_desc;
        buffer_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        buffer_desc.size = sizeof(Uniforms);
        buffer_desc.mappedAtCreation = false;

        buffer = ctx.gpu.get_device().createBuffer(buffer_desc);

        update_bind_group();
    }


    void update_bind_group() {
        wgpu::BindGroupEntry bg_entries[3];
        // texture entry
        bg_entries[0].binding = 0;
        bg_entries[0].textureView = *ctx.resource_manager.get_image(image_index).texture_view;
        // sampler entry
        bg_entries[1].binding = 1;
        bg_entries[1].sampler = *ctx.resource_manager.default_texture_sampler;
        // uniforms entry
        bg_entries[2].binding = 2;
        bg_entries[2].buffer = *buffer;
        bg_entries[2].offset = 0;
        bg_entries[2].size = sizeof(Uniforms);

        wgpu::BindGroupDescriptor bg_desc;
        bg_desc.layout = *bind_group_layout;
        bg_desc.entryCount = 3;
        bg_desc.entries = bg_entries;

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


    void display() {
        parameters.display();
    }

    void write_buffers(wgpu::Queue& queue) const {
        queue.writeBuffer(*buffer, 0, &uniforms, sizeof(uniforms));
    }

    void reset() {
        uniforms = {{{0.0, 0.0, 0.0, 0.0, 0.0, 1.0}}};
        uniforms.size_x = static_cast<float>(base_width);
        uniforms.size_y = static_cast<float>(base_height);
    }

    void set_bind_groups(wgpu::RenderPassEncoder& pass_encoder) const {
        pass_encoder.setBindGroup(1, *bind_group, 0, nullptr);
    }
};
