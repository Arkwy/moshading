#pragma once

#include <chrono>
#include <coroutine>
#include <memory>
#include <optional>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>
#include <webgpu/webgpu-raii.hpp>

#include "imgui.h"
#include "shader.hpp"
#include "shaders/chromatic_aberration.hpp"
#include "shaders/dithering.hpp"
#include "shaders/image.hpp"
#include "shaders/noise.hpp"
#include "src/context.hpp"
#include "src/log.hpp"

struct ShaderManager {
    Context& ctx;

    ShaderManager(Context& ctx);

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager(ShaderManager&&) = delete;

    void display();
    void render() const;


    template <ShaderUnionConcept S, typename... Args>
    void add_shader(Args&&... args) {  // TODO move to private when ui is here
        shaders.push_back(std::make_unique<ShaderUnion>());
        auto& shader = shaders[shaders.size() - 1];
        shader->set<S>(args...);
        shader->apply([&](auto& s) { s.init(); });
        shader->apply([&](auto& s) { s.init_pipeline(*default_bind_group_layout); });
    }

    void add_shader(std::unique_ptr<ShaderUnion>&& shader_ptr) {  // TODO move to private when ui is here
        assert(shader_ptr.get()->tag != ShaderUnion::Tag::None);
        shaders.push_back(std::forward<std::unique_ptr<ShaderUnion>>(shader_ptr));
        auto& shader = shaders[shaders.size() - 1];
        shader->apply([&](auto& s) { s.init(); });
        shader->apply([&](auto& s) { s.init_pipeline(*default_bind_group_layout); });
    }

    void reorder_element(size_t index, size_t new_index);  // TODO move to private when ui is here

  private:
    struct alignas(16) DefaultUniforms {
        uint32_t viewport_width;
        uint32_t viewport_height;
        float time;
    };


    struct DisplayState {
        float zoom;
        float offset_x;
        float offset_y;
    };

    mutable DisplayState display_state{1.0, 0.0, 0.0};

    size_t selected_shader = 0;
    bool adding_shader = false;

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

    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;

    void init();

    void resize(unsigned int new_width, unsigned int new_height);

    void creation_dialog(ShaderKind k) {
#define X(name)                                               \
    [&]() {                                                   \
        if (try_creation_dialog<ShaderKind::name>(k)) return; \
    }()
        (SHADER_KINDS);
#undef X
    }


    template <ShaderKind K>
    requires(sizeof(Shader<K>::RESOURCES) != 0)
    bool try_creation_dialog(ShaderKind k) {
        if (k == K) {
            static std::optional<std::string> selection = std::nullopt;

            if (selection.has_value()) {
                ImGui::Text("%s", selection.value().c_str());
            }

            // TODO selections logic
            ImGui::Text("it needs some ressources");

            // if (!selection.has_value()) {
            //     ImGui::BeginDisabled();
            // }
            // if (ImGui::Button("Add")) {
            //     assert(selection.has_value());
            //     add_shader<Shader<K>>(
            //         std::filesystem::path(selection.value()).filename().stem().string(),
            //         selection.value(),
            //         ctx
            //     );
            //     adding_shader = false;
            //     selection = std::nullopt;
            //     return true;
            // }
            // if (!selection.has_value()) {
            //     ImGui::EndDisabled();
            // }
            return true;
        }
        return false;
    }


    template <ShaderKind K>
        requires(sizeof(Shader<K>::RESOURCES) == 0)
    bool try_creation_dialog(ShaderKind k) {
        if (k == K) {
            if (ImGui::Button("Add")) {
                add_shader<Shader<K>>("default name", ctx);
                adding_shader = false;
            }
            return true;
        }
        return false;
    }
};
