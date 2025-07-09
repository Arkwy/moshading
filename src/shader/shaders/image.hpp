#pragma once

#include <imgui.h>

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <webgpu/webgpu-raii.hpp>

#include "imgui_internal.h"
#include "webgpu/webgpu.hpp"

#include <stb/stb_image.h>

#include "shaders_code.hpp"
#include "src/shader/shader.hpp"


template <>
struct Shader<ShaderKind::Image> : public ShaderBase<Shader<ShaderKind::Image>> {
    const std::string image_path;

    struct StbiDeleter {
        void operator()(uint8_t* data) const {
            stbi_image_free(data);
        }
    };
    using StbiPtr = std::unique_ptr<uint8_t, StbiDeleter>;
    StbiPtr image_data;

    Shader(const std::string& name, const std::string& image_path)
        : ShaderBase<Shader<ShaderKind::Image>>(name, fullscreen_vertex, image), image_path(image_path) {}


    struct alignas(16) Uniforms {
        union {
            struct {
                float size_x;
                float size_y;
                float pos_x;
                float pos_y;
                float rot;
                float opacity;
            };
            struct {
                float size[2];
                float pos[2];
                float rotation;
                float transparency;
            };
            float raw[6];
        };
    };

    Uniforms uniforms = {{{0.0, 0.0, 0.0, 0.0, 0.0, 1.0}}};

    wgpu::raii::Sampler sampler;
    wgpu::raii::Texture texture;
    wgpu::raii::TextureView texture_view;

    wgpu::raii::BindGroupLayout bind_group_layout;
    wgpu::raii::Buffer buffer;
    wgpu::raii::BindGroup bind_group;

    int base_height = -1;
    int base_width = -1;

    void init(const GPUContext& ctx) {
        this->init_module(ctx);

        // loading image
        uint8_t* _image_data = stbi_load(image_path.c_str(), &base_width, &base_height, nullptr, 4);

        if (!_image_data) {
            throw std::runtime_error("Could not load image.");
            // TODO implement better error handling
        }
        image_data = StbiPtr(_image_data);

        uniforms.size_x = static_cast<float>(base_width);
        uniforms.size_y = static_cast<float>(base_height);

        // create texture
        wgpu::TextureDescriptor tex_desc;
        tex_desc.size = {static_cast<uint32_t>(base_width), static_cast<uint32_t>(base_height), 1};
        tex_desc.format = wgpu::TextureFormat::RGBA8Unorm;
        tex_desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
        tex_desc.dimension = wgpu::TextureDimension::_2D;
        tex_desc.mipLevelCount = 1;
        tex_desc.sampleCount = 1;
        tex_desc.viewFormatCount = 0;
        tex_desc.viewFormats = nullptr;

        texture = ctx.get_device().createTexture(tex_desc);

        // upload to GPU
        wgpu::raii::Queue queue = ctx.get_device().getQueue();

        wgpu::TexelCopyTextureInfo tcti;
        tcti.texture = *texture;
        tcti.mipLevel = 0;
        tcti.origin = {0, 0, 0};
        tcti.aspect = wgpu::TextureAspect::All;

        wgpu::TexelCopyBufferLayout tcbl;
        tcbl.bytesPerRow = base_width * 4;
        tcbl.rowsPerImage = base_height;
        tcbl.offset = 0;

        wgpu::Extent3D e3d;
        e3d.width = base_width;
        e3d.height = base_height;
        e3d.depthOrArrayLayers = 1;

        queue->writeTexture(tcti, image_data.get(), base_height * base_width * 4, tcbl, e3d);

        // create texture and sampler binding
        wgpu::TextureViewDescriptor tex_view_desc = {};
        tex_view_desc.format = wgpu::TextureFormat::RGBA8Unorm;
        tex_view_desc.dimension = wgpu::TextureViewDimension::_2D;
        tex_view_desc.mipLevelCount = 1;
        tex_view_desc.baseMipLevel = 0;
        tex_view_desc.arrayLayerCount = 1;
        tex_view_desc.baseArrayLayer = 0;
        tex_view_desc.aspect = wgpu::TextureAspect::All;

        texture_view = texture->createView(tex_view_desc);


        wgpu::SamplerDescriptor sampler_desc = {};
        sampler_desc.addressModeU = wgpu::AddressMode::ClampToEdge;
        sampler_desc.addressModeV = wgpu::AddressMode::ClampToEdge;
        sampler_desc.addressModeW = wgpu::AddressMode::ClampToEdge;
        sampler_desc.magFilter = wgpu::FilterMode::Linear;
        sampler_desc.minFilter = wgpu::FilterMode::Linear;
        sampler_desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
        sampler_desc.maxAnisotropy = 1;

        sampler = ctx.get_device().createSampler(sampler_desc);


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
        bind_group_layout = ctx.get_device().createBindGroupLayout(bgl_desc);

        wgpu::BufferDescriptor buffer_desc;
        buffer_desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        buffer_desc.size = sizeof(Uniforms);
        buffer_desc.mappedAtCreation = false;

        buffer = ctx.get_device().createBuffer(buffer_desc);

        wgpu::BindGroupEntry bg_entries[3];
        // texture entry
        bg_entries[0].binding = 0;
        bg_entries[0].textureView = *texture_view;
        // sampler entry
        bg_entries[1].binding = 1;
        bg_entries[1].sampler = *sampler;
        // uniforms entry
        bg_entries[2].binding = 2;
        bg_entries[2].buffer = *buffer;
        bg_entries[2].offset = 0;
        bg_entries[2].size = sizeof(Uniforms);

        wgpu::BindGroupDescriptor bg_desc;
        bg_desc.layout = *bind_group_layout;
        bg_desc.entryCount = 3;
        bg_desc.entries = bg_entries;

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
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.35);
        ImGui::DragFloat("##pos_x", &uniforms.pos_x, 1, 0, 0, "x: %.3f");
        ImGui::SameLine();
        ImGui::DragFloat("##pos_y", &uniforms.pos_y, 1, 0, 0, "y: %.3f");
        ImGui::DragFloat("##size_x", &uniforms.size_x, 1, 0, 0, "x: %.3f");
        ImGui::SameLine();
        ImGui::DragFloat("##size_y", &uniforms.size_y, 1, 0, 0, "y: %.3f");
        ImGui::PopItemWidth();
        ImGui::DragFloat("rotation", &uniforms.rot, 0.01);
        ImGui::SliderFloat("opacity", &uniforms.opacity, 0, 1);

        ImVec2 pad_size = ImVec2(200, 200);
        ImVec2 pad_min = ImGui::GetCursorScreenPos();
        ImVec2 pad_max = ImVec2(pad_min.x + pad_size.x, pad_min.y + pad_size.y);

        // Draw the background
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(pad_min, pad_max, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(pad_min, pad_max, IM_COL32(255, 255, 255, 255));

        // Normalize and draw the handle
        ImVec2& pos = *reinterpret_cast<ImVec2*>(uniforms.pos);
        ImVec2 handle_pos = ImVec2(pad_min.x + pos.x, pad_min.y + pos.y);
        handle_pos = ImClamp(handle_pos, pad_min, pad_max);

        draw_list->AddCircleFilled(handle_pos, 5.0f, IM_COL32(255, 0, 0, 255));

        // Dragging logic
        ImGui::InvisibleButton("pad", pad_size);
        bool is_active = ImGui::IsItemActive();

        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            ImVec2 delta = ImGui::GetIO().MouseDelta;
            pos.x += delta.x;
            pos.y += delta.y;
        }
    }

    void write_buffers(wgpu::Queue& queue) const { queue.writeBuffer(*buffer, 0, &uniforms, sizeof(uniforms)); }

    void reset() {
        uniforms = {{{0.0, 0.0, 0.0, 0.0, 0.0, 1.0}}};
        uniforms.size_x = static_cast<float>(base_width);
        uniforms.size_y = static_cast<float>(base_height);
    }

    void set_bind_groups(wgpu::RenderPassEncoder& pass_encoder) const {
        pass_encoder.setBindGroup(1, *bind_group, 0, nullptr);
    }
};
