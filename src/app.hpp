#pragma once

// #include <memory>
// #include <vector>

// #include "image.hpp"
// #include "shader.hpp"

#include <webgpu/webgpu.h>
#include "gpu_context.hpp"
#include "webgpu/webgpu-raii.hpp"

struct App {
    const GPUContext& ctx;
    unsigned int shader_render_width = 1920;
    unsigned int shader_render_height = 1080;
    float time = 0;

    
    // std::vector<std::unique_ptr<Img>> images;
    // std::vector<std::unique_ptr<Shader>> shaders;

    App(const GPUContext& ctx) : ctx(ctx) {
    };

    void display();


  private:
    wgpu::raii::Texture tex;
    wgpu::raii::TextureView tex_view;
    wgpu::raii::ShaderModule shader_module;
    wgpu::raii::RenderPipeline render_pipeline; 
    // wgpu::raii::Sampler rescale_sampler;
    // wgpu::raii::BindGroupLayout rescale_bind_group_layout;
    // wgpu::raii::BindGroup rescale_bind_group;

    bool initiliazed = false;
    bool render_shader();
};

