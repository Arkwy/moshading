#pragma once

#include <memory>
#include <vector>
#include <chrono>

#include <webgpu/webgpu-raii.hpp>

#include "src/gpu_context.hpp"
#include "src/file_loader.hpp"
#include "shader.hpp"
#include "shaders/circle.hpp"
#include "shaders/chromatic_aberration.hpp"
#include "shaders/image.hpp"
#include "shaders/noise.hpp"
#include "shaders/dithering.hpp"


struct ShaderManager {
    const GPUContext& ctx;

    ShaderManager(const GPUContext& ctx, unsigned int width, unsigned int height);

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager(ShaderManager&&) = delete;

    void display();
    void render() const;

    template <ShaderUnionConcept S, typename... Args>
    void add_shader(Args&&... args){  // TODO move to private when ui is here
        shaders.push_back(std::make_unique<ShaderUnion>());
        shaders[shaders.size()-1]->set<S>(args...);
        shaders[shaders.size()-1]->apply([&](auto& s) { s.init(ctx); });
        shaders[shaders.size()-1]->apply([&](auto& s) { s.init_pipeline(ctx, *default_bind_group_layout); });
    }

    void reorder_element(size_t index, size_t new_index);  // TODO move to private when ui is here

  private:
    struct alignas(16) DefaultUniforms {
        uint32_t viewport_width;
        uint32_t viewport_height;
        float time;
    };

    FileLoader file_loader;

    wgpu::raii::BindGroupLayout default_bind_group_layout;
    wgpu::raii::Sampler sampler;
    wgpu::raii::Buffer default_uniforms;

    wgpu::raii::Texture texture_A;
    wgpu::raii::TextureView texture_view_A;
    wgpu::raii::BindGroup bind_group_A;

    wgpu::raii::Texture texture_B;
    wgpu::raii::TextureView texture_view_B;
    wgpu::raii::BindGroup bind_group_B;

    std::vector<std::unique_ptr<ShaderUnion>> shaders;

    unsigned int width;
    unsigned int height;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;

    void init();

    void resize(unsigned int new_width, unsigned int new_height);
};
