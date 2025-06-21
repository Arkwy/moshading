#pragma once

#include <vector>
#include <chrono>

#include <webgpu/webgpu-raii.hpp>

#include "src/gpu_context.hpp"
#include "shader.hpp"


struct ShaderManager {
    const GPUContext& ctx;

    ShaderManager(const GPUContext& ctx, unsigned int width, unsigned int height);

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager(ShaderManager&&) = delete;

    void display();
    void render() const;
    // void add_shader(
    //     const std::string& name, const char* const vertex_code, const char* const frag_code
    // );  // TODO refacto to limit module recompilation + move to private when ui is here
    void add_shader(ShaderVariant&& shader);  // TODO move to private when ui is here
    // void reorder_element(size_t index, size_t new_index);  // TODO move to private when ui is here

  private:
    struct alignas(16) DefaultUniforms {
        uint32_t viewport_width;
        uint32_t viewport_height;
        float time;
    };

    wgpu::raii::BindGroupLayout default_bind_group_layout;
    wgpu::raii::Sampler sampler;
    wgpu::raii::Buffer default_uniforms;

    wgpu::raii::Texture texture_A;
    wgpu::raii::TextureView texture_view_A;
    wgpu::raii::BindGroup bind_group_A;

    wgpu::raii::Texture texture_B;
    wgpu::raii::TextureView texture_view_B;
    wgpu::raii::BindGroup bind_group_B;

    std::vector<ShaderVariant> shaders;

    unsigned int width;
    unsigned int height;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;


    void init();

    void resize(unsigned int new_width, unsigned int new_height);
};
