#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <chrono>

#include "src/gpu_context.hpp"
#include "src/shader/shader.hpp"
#include "webgpu/webgpu-raii.hpp"

struct ShaderManager;

struct InnerShader {
    std::unique_ptr<Shader> shader;  // futur proof for when `Shader` will be derived
    wgpu::raii::ShaderModule vertex_module;
    wgpu::raii::ShaderModule frag_module;
    wgpu::raii::RenderPipeline render_pipeline;

    InnerShader(InnerShader&& other) noexcept
        : shader(std::move(other.shader)),
          vertex_module(std::move(other.vertex_module)),
          frag_module(std::move(other.frag_module)),
          render_pipeline(std::move(other.render_pipeline)) {}

    InnerShader& operator=(InnerShader&& other) noexcept {
        if (this != &other) {
            shader = std::move(other.shader);
            vertex_module = std::move(other.vertex_module);
            frag_module = std::move(other.frag_module);
            render_pipeline = std::move(other.render_pipeline);
        }
        return *this;
    }

    // Deleted copy constructor and copy assignment operator
    InnerShader(const InnerShader&) = delete;
    InnerShader& operator=(const InnerShader&) = delete;

    // debug constructor, to be removed
    InnerShader(
        const ShaderManager& manager,
        const std::string& name,
        const char* const vertex_code,
        const char* const frag_code
    );
    InnerShader(const ShaderManager& manager, std::unique_ptr<Shader> shader);

  private:
    void init_module(const ShaderManager& manager);
    void init_pipeline(const ShaderManager& manager);
};


struct ShaderManager {
    const GPUContext& ctx;

    ShaderManager(const GPUContext& ctx, unsigned int width, unsigned int height);

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager(ShaderManager&&) = delete;

    void display();
    void render() const;
    void add_shader(
        const std::string& name, const char* const vertex_code, const char* const frag_code
    );  // TODO refacto to limit module recompilation + move to private when ui is here
    void add_shader(std::unique_ptr<Shader> shader);  // TODO move to private when ui is here
    void reorder_element(size_t index, size_t new_index);  // TODO move to private when ui is here

  private:
    friend struct InnerShader;

    struct alignas(16) DefaultUniforms {
        uint32_t viewport_width;
        uint32_t viewport_height;
        float time;
    };

    wgpu::raii::BindGroupLayout bind_group_layout;
    wgpu::raii::Sampler sampler;
    wgpu::raii::Buffer default_uniforms;

    wgpu::raii::Texture texture_A;
    wgpu::raii::TextureView texture_view_A;
    wgpu::raii::BindGroup bind_group_A;

    wgpu::raii::Texture texture_B;
    wgpu::raii::TextureView texture_view_B;
    wgpu::raii::BindGroup bind_group_B;

    std::vector<InnerShader> shaders;

    unsigned int width;
    unsigned int height;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;


    void init();

    void resize(unsigned int new_width, unsigned int new_height);
};
