#pragma once

#include <imgui.h>

#include <webgpu/webgpu-raii.hpp>

#include "shaders_code.hpp"
#include "src/shader/shader.hpp"
#include "webgpu/webgpu.hpp"


template <>
struct Shader<ShaderKind::Noise> : public ShaderBase<Shader<ShaderKind::Noise>> {
    Shader(const std::string& name)
        : ShaderBase<Shader<ShaderKind::Noise>>(name, fullscreen_vertex, noise) {}

    enum class Mode : int {
        Grey,
        Colored,
    };
    const char* modes[2] = {"Grey", "Colored"};

    struct alignas(16) Uniforms {
        union {
            struct {
                float mean_red;
                float mean_green;
                float mean_blue;
                float mean;
                float variance_red;
                float variance_green;
                float variance_blue;
                float variance;
                Mode mode;
            };
            struct {
                float colored_mean[3];
                float _;
                float colored_variance[3];
                float _;
                int mode_id;
            };
        };
    };

    Uniforms uniforms = {{{0.0, 0.0, 0.0, 0.0, 0.05, 0.05, 0.05, 0.05, Mode::Grey}}};

    wgpu::raii::BindGroupLayout bind_group_layout;
    wgpu::raii::Buffer buffer;
    wgpu::raii::BindGroup bind_group;

    void init(const GPUContext& ctx) {
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
        const GPUContext& ctx, const wgpu::BindGroupLayout& default_bind_group_layout
    ) {
        wgpu::raii::PipelineLayout pipeline_layout;

        WGPUBindGroupLayout bgls[2] = {default_bind_group_layout, *bind_group_layout};

        wgpu::PipelineLayoutDescriptor pipeline_layout_desc;
        pipeline_layout_desc.bindGroupLayoutCount = 2;
        pipeline_layout_desc.bindGroupLayouts = bgls;
        pipeline_layout = ctx.get_device().createPipelineLayout(pipeline_layout_desc);

        return pipeline_layout;
    }


    void display() {
        ImGui::Combo("Mode", &uniforms.mode_id, modes, 2);
        switch (uniforms.mode) {
            case Mode::Grey:
                ImGui::DragFloat("mean", &uniforms.mean, 0.001);
                ImGui::DragFloat("variance", &uniforms.variance, 0.001, 0, 100);
                break;
            default:
                ImGui::DragFloat3("mean", uniforms.colored_mean, 0.001);
                ImGui::DragFloat3("variance", uniforms.colored_variance, 0.001);
                break;
        }
    }

    void reset() {
        uniforms = {{{0.0, 0.0, 0.0, 0.0, 0.05, 0.05, 0.05, 0.05, Mode::Grey}}};
    }

    void write_buffers(wgpu::Queue& queue) const { queue.writeBuffer(*buffer, 0, &uniforms, sizeof(uniforms)); }

    void set_bind_groups(wgpu::RenderPassEncoder& pass_encoder) const {
        pass_encoder.setBindGroup(1, *bind_group, 0, nullptr);
    }
};
