#pragma once

#include <imgui.h>

#include <webgpu/webgpu-raii.hpp>

#include "imgui_internal.h"
#include "shaders_code.hpp"
#include "src/shader/shader.hpp"
#include "webgpu/webgpu.hpp"


template <>
struct Shader<ShaderKind::Dithering> : public ShaderBase<Shader<ShaderKind::Dithering>> {
    Shader(const std::string& name)
        : ShaderBase<Shader<ShaderKind::Dithering>>(name, fullscreen_vertex, dithering) {}

    enum class Mode : int {
        Threshold, 
        Random, 
        Halftone, 
        Bayer, 
        VoidAndCluster
    };
    const char* modes[5] = {"Threshold", "Random", "Halftone", "Ordered (bayer)", "Ordered (void-and-cluster)"};

    struct alignas(16) Uniforms {
        union {
            struct {
                Mode mode;
                bool colored;
                float threshold;
                float threshold_r;
                float threshold_g;
                float threshold_b;
            };
            struct {
                int mode_id;
                bool _;
                float _;
            };
        };
    };

    Uniforms uniforms = {{{Mode::Threshold, false, 0.0}}};

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
            case Mode::Threshold:
                ImGui::DragFloat("threshold", &uniforms.threshold, 0.001);
                break;
            default:
                break;
        }
    }

    void write_buffers(wgpu::Queue& queue) const { queue.writeBuffer(*buffer, 0, &uniforms, sizeof(uniforms)); }

    void set_bind_groups(wgpu::RenderPassEncoder& pass_encoder) const {
        pass_encoder.setBindGroup(1, *bind_group, 0, nullptr);
    }
};
