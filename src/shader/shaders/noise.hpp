#pragma once

#include <imgui.h>

#include <webgpu/webgpu-raii.hpp>

#include "shaders_code.hpp"
#include "src/context.hpp"
#include "src/shader/shader.hpp"
#include "webgpu/webgpu.hpp"


template <>
struct Shader<ShaderKind::Noise> : public ShaderBase<Shader<ShaderKind::Noise>> {
    Shader(const std::string& name, const Context& ctx)
        : ShaderBase<Shader<ShaderKind::Noise>>(name, ctx.shader_source_cache.get(fullscreen_vertex), ctx.shader_source_cache.get(noise), ctx) {}


    struct alignas(16) Uniforms {
        float colored_min[3] = {-0.1, -0.1, -0.1};
        float min = -0.1;
        float colored_max[3] = {0.1, 0.1, 0.1};
        float max = 0.1;
        unsigned int control = 3;
        unsigned int seed = 0;
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
        bool color_mode = uniforms.control & 1u;
        bool dynamic_mode = uniforms.control & 2u;
        if (ImGui::Checkbox("color mode", &color_mode)) {
            uniforms.control ^= 1u;
        }
        if (ImGui::Checkbox("dynamic", &dynamic_mode)) {
            uniforms.control ^= 2u;
        }
        int seed = uniforms.seed;
        if (ImGui::InputInt("seed", &seed) && seed >= 0) uniforms.seed = seed;  
        if (color_mode) {
            if (ImGui::DragFloat3("min", uniforms.colored_min, 0.01, -1, 1, "%.2f"))
                #pragma unroll
                for (size_t i = 0; i < 3; i++)
                    if (uniforms.colored_min[i] > uniforms.colored_max[i])
                        uniforms.colored_min[i] = uniforms.colored_max[i];
            if (ImGui::DragFloat3("max", uniforms.colored_max, 0.01, -1, 1, "%.2f"))
                #pragma unroll
                for (size_t i = 0; i < 3; i++)
                    if (uniforms.colored_max[i] < uniforms.colored_min[i])
                        uniforms.colored_max[i] = uniforms.colored_min[i];
        } else {
            if (ImGui::DragFloat("min", &uniforms.min, 0.01, -1, 1, "%.2f"))
                if (uniforms.min > uniforms.max)
                    uniforms.min = uniforms.max;
            if (ImGui::DragFloat("max", &uniforms.max, 0.01, -1, 1, "%.2f"))
                if (uniforms.max < uniforms.min)
                    uniforms.max = uniforms.min;
        }
    }

    void reset() {
        uniforms = {};
    }

    void write_buffers(wgpu::Queue& queue) const { queue.writeBuffer(*buffer, 0, &uniforms, sizeof(uniforms)); }

    void set_bind_groups(wgpu::RenderPassEncoder& pass_encoder) const {
        pass_encoder.setBindGroup(1, *bind_group, 0, nullptr);
    }
};
