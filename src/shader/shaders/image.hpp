#pragma once

#include <imgui.h>
#include <stb/stb_image.h>

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <valarray>
#include <webgpu/webgpu-raii.hpp>

#include "shaders_code.hpp"
#include "src/context.hpp"
#include "src/file_loader.hpp"
#include "src/log.hpp"
#include "src/shader/manager.hpp"
#include "src/shader/parameter.hpp"
#include "src/shader/shader.hpp"
#include "webgpu/webgpu.hpp"


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

    Uniforms uniforms = {{{0.0, 0.0, 0.0, 0.0, 0.0, 1.0}}};

    wgpu::raii::Sampler sampler;
    wgpu::raii::Texture texture;
    wgpu::raii::TextureView texture_view;

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

    Shader(const std::string& name, const std::string& image_path, const Context& ctx)
        : ShaderBase<Shader<ShaderKind::Image>>(name, ctx.cache.get(fullscreen_vertex), ctx.cache.get(image), ctx),
          image_path(image_path),
          parameters(init_parameters(uniforms, render_dim)) {}


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


    void init(const Context& ctx) {
        set_render_dim(ctx.rendering.dim);
        std::get<2>(parameters.widgets).max = {static_cast<float>(render_dim[0]), static_cast<float>(render_dim[1])};

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

        texture = ctx.gpu.get_device().createTexture(tex_desc);

        // upload to GPU
        wgpu::raii::Queue queue = ctx.gpu.get_device().getQueue();

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

        sampler = ctx.gpu.get_device().createSampler(sampler_desc);


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
