#pragma once
#include "resource.hpp"
#include "src/file_loader.hpp"

struct ResourceManager {
    const GPU& gpu;
    wgpu::raii::Sampler default_texture_sampler;
    FileLoader file_loader{};

    ResourceManager(const GPU& gpu) : gpu(gpu) {
        wgpu::SamplerDescriptor default_texture_sampler_desc = {};
        default_texture_sampler_desc.addressModeU = wgpu::AddressMode::ClampToEdge;
        default_texture_sampler_desc.addressModeV = wgpu::AddressMode::ClampToEdge;
        default_texture_sampler_desc.addressModeW = wgpu::AddressMode::ClampToEdge;
        default_texture_sampler_desc.magFilter = wgpu::FilterMode::Linear;
        default_texture_sampler_desc.minFilter = wgpu::FilterMode::Linear;
        default_texture_sampler_desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
        default_texture_sampler_desc.maxAnisotropy = 1;

        default_texture_sampler = gpu.get_device().createSampler(default_texture_sampler_desc);
    }

    size_t add_image(const std::string& name, const Resource<ResourceKind::Image>::Handle& handle) {
        images.push_back(Resource<ResourceKind::Image>(name, handle, gpu));
        size_t id = next_id();
        images_index_map[id] = images.size() - 1;
        return id;
    }

    const Resource<ResourceKind::Image>& get_image(size_t id) const {
        return images[images_index_map.at(id)];
    }

    std::unordered_map<size_t, size_t> images_index_map;
    std::vector<Resource<ResourceKind::Image>> images;

    static size_t next_id() {
        static size_t id = 0;
        return id++;
    }

    void display() {
        bool can_open_dialog = file_loader.check();

        ImGui::BeginDisabled(!can_open_dialog);
        if (ImGui::Button("Import")) {
            file_loader.open_dialog<ResourceKind::Image>(
#ifdef __EMSCRIPTEN__
                [&](const char* name, uint8_t* data, size_t len) {
                    ressource_manager.add_image(name, Resource<ResourceKind::Image>::Handle{.data = data, .len = len});
                }
#else
                [&](const std::string& file) {
                    std::filesystem::path path = file;
                    add_image(path.stem(), path);
                }
#endif
            );
        }
        ImGui::EndDisabled();

        float vignette_size = 200;
        float max_cursor_x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - vignette_size;
        for (auto& [resource_id, index] : images_index_map) {
            auto& resource = images[index];
            ImGui::BeginChild(
                std::format("{}{}", resource.name, resource_id).c_str(),
                ImVec2(vignette_size, vignette_size)
            );
            display_resource(resource);
            ImGui::EndChild();
            ImGui::SameLine();
            if (ImGui::GetCursorPosX() > max_cursor_x) ImGui::NewLine();
        }
    }


    template <ResourceKind K>
        requires(K == ResourceKind::Image)
    void display_resource(Resource<K>& resource) {
        ImVec2 display_region = ImGui::GetContentRegionAvail();
        ImVec2 text_size = ImGui::CalcTextSize(resource.name.c_str());

        float start_x = ImGui::GetCursorPosX();
        float start_y = ImGui::GetCursorPosY();

        display_region.y -= 1.5 * text_size.y;

        ImVec2 display_dim;
        ImVec2 tex_to_display_ratio(display_region.x / resource.data.width, display_region.y / resource.data.height);

        if (tex_to_display_ratio.x < tex_to_display_ratio.y) {
            display_dim.x = tex_to_display_ratio.x * resource.data.width;
            display_dim.y = tex_to_display_ratio.x * resource.data.height;
        } else {
            display_dim.x = tex_to_display_ratio.y * resource.data.width;
            display_dim.y = tex_to_display_ratio.y * resource.data.height;
        }

        ImGui::SetCursorPos(ImVec2(
            -(display_dim.x - display_region.x) * 0.5 + start_x,
            -(display_dim.y - display_region.y) * 0.5 + start_y
        ));

        ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<WGPUTextureView>(*resource.texture_view)), display_dim);

        ImGui::SetCursorPos(ImVec2(
            -(display_dim.x - display_region.x) * 0.5 + start_x,
            -(display_dim.y - display_region.y) * 0.5 + start_y
        ));
        if (ImGui::InvisibleButton("## change image", display_dim, ImGuiButtonFlags_PressedOnDoubleClick)) {
            file_loader.open_dialog<ResourceKind::Image>(
#ifdef __EMSCRIPTEN__
                [&](const char* name, uint8_t* data, size_t len) {
                    resource.name = name;
                    resource.update(Resource<ResourceKind::Image>::Handle{.data = data, .len = len}, gpu);
                }
#else
                [&](const std::string& file) {
                    std::filesystem::path path = file;
                    resource.name = path.stem();
                    resource.update(path, gpu);
                }
#endif
            );
        }

        ImGui::SetCursorPosX((display_region.x - text_size.x) / 2);
        ImGui::Text("%s", resource.name.c_str());
    }
};
