#pragma once

#include <imgui.h>

#include <bit>
#include <webgpu/webgpu-raii.hpp>

#include "shaders_code.hpp"
#include "src/shader/shader.hpp"
#include "webgpu/webgpu.hpp"


template <>
struct Shader<ShaderKind::Dithering> : public ShaderBase<Shader<ShaderKind::Dithering>> {
    constexpr static const char* const default_name = "dithering";
    Shader(const std::string& name, const Context& ctx)
        : ShaderBase<Shader<ShaderKind::Dithering>>(
              name, ctx.shader_source_cache.get(fullscreen_vertex), ctx.shader_source_cache.get(dithering), ctx
          ) {}

    enum class Mode : int { Threshold, Random, Halftone, Bayer, VoidAndCluster };
    const char* modes[5] = {"Threshold", "Random", "Halftone", "Ordered (bayer)", "Ordered (void-and-cluster)"};

    struct alignas(16) Uniforms {
        Mode mode = Mode::Threshold;
        unsigned int control = 0;
        float threshold = 0.5;
        float _;
        float threshold_rgb[3] = {0.5, 0.5, 0.5};
        float _;
        float random_min_rgb[3] = {0, 0, 0};
        float _;
        float random_max_rgb[3] = {1, 1, 1};
        float random_min = 0;
        float random_max = 1;
        float halftone_scale = 10;
        float halftone_angle = 0;
        unsigned int bayer_steps = 3;
    };


    Uniforms uniforms{};

    wgpu::raii::BindGroupLayout bind_group_layout;
    wgpu::raii::Buffer buffer;
    wgpu::raii::BindGroup bind_group;

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


    void display() {
        ImGui::Combo("Mode", std::bit_cast<int*>(&uniforms.mode), modes, 4);

        bool color_mode = static_cast<bool>(uniforms.control & 1u);
        bool time_based = static_cast<bool>(uniforms.control & 2u);
        if (ImGui::Checkbox("color mode", &color_mode)) {
            uniforms.control ^= 1u;
        }
        int bayer_steps = uniforms.bayer_steps;

        switch (uniforms.mode) {
            case Mode::Threshold:
                if (uniforms.control & 1u) {
                    ImGui::DragFloat3("threshold", uniforms.threshold_rgb, 0.001);
                } else {
                    ImGui::DragFloat("threshold", &uniforms.threshold, 0.001);
                }
                break;
            case Mode::Random:
                if (ImGui::Checkbox("dynamic", &time_based)) {
                    uniforms.control ^= 2u;
                }
                if (uniforms.control & 1u) {
                    ImGui::DragFloat3("min threshold", uniforms.random_min_rgb, 0.001);
                    ImGui::DragFloat3("max threshold", uniforms.random_max_rgb, 0.001);
                } else {
                    ImGui::DragFloat("min threshold", &uniforms.random_min, 0.001);
                    ImGui::DragFloat("max threshold", &uniforms.random_max, 0.001);
                }
                break;
            case Mode::Halftone:
                ImGui::DragFloat("size", &uniforms.halftone_scale, 0.1);
                ImGui::DragFloat("orientation", &uniforms.halftone_angle, 0.01);
                break;
            case Mode::Bayer:
                if (ImGui::SliderInt("steps", &bayer_steps, 0, 10)) uniforms.bayer_steps = bayer_steps;
                break;
            default:
                break;
        }
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
