#pragma once

#include <stb/stb_image.h>

#include <cstddef>
#include <filesystem>
#include <functional>
#include <type_traits>
#include <vector>
#include <webgpu/webgpu-raii.hpp>

#include "imgui.h"
#include "imgui_internal.h"
#include "src/context/gpu.hpp"
#include "src/log.hpp"


enum class ResourceKind {
    Image,
    // Video, TODO
};


struct SafeCallback {
    std::weak_ptr<void> subscriber_lifetime;
    std::function<void()> callback;
};


template <ResourceKind T>
struct Resource;


template <>
struct Resource<ResourceKind::Image> {
#ifdef __EMSCRIPTEN__
    using Handle = uint8_t*;
#else
    using Handle = std::filesystem::path;
#endif

    struct Data {
        uint8_t* ptr;
        int width;
        int height;
    };

    Data data{};

    bool uploaded = false;
    wgpu::raii::Texture texture;
    wgpu::raii::TextureView texture_view;

    std::string name;

    mutable std::vector<SafeCallback> update_callbacks;

    ~Resource() {
        if (data.ptr) stbi_image_free(data.ptr);
    }


    Resource(const std::string& name, const Handle& handle, const GPU& gpu, bool upload = true) : name(name) {
        update(handle, gpu, upload);
    }


    Resource(Resource&& other)
        : data(other.data),
          uploaded(other.uploaded),
          texture(std::move(other.texture)),
          texture_view(std::move(other.texture_view)),
          name(std::move(other.name)) {
        other.data.ptr = nullptr;
    }

    void update(const Handle& handle, const GPU& gpu, bool upload = true) {
        data = load(handle);

        if (!data.ptr) {
            Log::error("Image loading failed.");
            return;
        }

        wgpu::TextureDescriptor tex_desc;
        tex_desc.size = {static_cast<uint32_t>(data.width), static_cast<uint32_t>(data.height), 1};
        tex_desc.format = wgpu::TextureFormat::RGBA8Unorm;
        tex_desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
        tex_desc.dimension = wgpu::TextureDimension::_2D;
        tex_desc.mipLevelCount = 1;
        tex_desc.sampleCount = 1;
        tex_desc.viewFormatCount = 0;
        tex_desc.viewFormats = nullptr;

        texture = gpu.get_device().createTexture(tex_desc);

        wgpu::TextureViewDescriptor tex_view_desc = {};
        tex_view_desc.format = wgpu::TextureFormat::RGBA8Unorm;
        tex_view_desc.dimension = wgpu::TextureViewDimension::_2D;
        tex_view_desc.mipLevelCount = 1;
        tex_view_desc.baseMipLevel = 0;
        tex_view_desc.arrayLayerCount = 1;
        tex_view_desc.baseArrayLayer = 0;
        tex_view_desc.aspect = wgpu::TextureAspect::All;

        texture_view = texture->createView(tex_view_desc);

        if (upload) upload_to_gpu(gpu);

        notify_update();
    }



    static Data load(const Handle& handle) {
        Data data;
#ifdef __EMSCRIPTEN__
        data.ptr = stbi_load_from_memory(handle, &data.width, &data.height, nullptr, 4);
#else
        data.ptr = stbi_load(handle.c_str(), &data.width, &data.height, nullptr, 4);
#endif
        return data;
    }


    void upload_to_gpu(const GPU& gpu) {
        if (uploaded) return;

        if (!texture) {
            Log::error("Tried to upload to a texture that has not been created.");
            return;
        }

        wgpu::raii::Queue queue = gpu.get_device().getQueue();
#ifdef __EMSCRIPTEN__
        wgpu::ImageCopyTexture tcti;
#else
        wgpu::TexelCopyTextureInfo tcti;
#endif
        tcti.texture = *texture;
        tcti.mipLevel = 0;
        tcti.origin = {0, 0, 0};
        tcti.aspect = wgpu::TextureAspect::All;

#ifdef __EMSCRIPTEN__
        wgpu::TextureDataLayout tcbl;
#else
        wgpu::TexelCopyBufferLayout tcbl;
#endif
        tcbl.bytesPerRow = data.width * 4;
        tcbl.rowsPerImage = data.height;
        tcbl.offset = 0;

        wgpu::Extent3D e3d;
        e3d.width = data.width;
        e3d.height = data.height;
        e3d.depthOrArrayLayers = 1;

        queue->writeTexture(tcti, data.ptr, data.height * data.width * 4, tcbl, e3d);

        uploaded = true;
    }


    template <typename T>
        requires std::is_same_v<std::shared_ptr<void>, decltype(std::declval<T>().lifetime_token)>
    void subscribe(const std::function<void()>& callback, T& subscriber) const {
        update_callbacks.push_back(SafeCallback{
            .subscriber_lifetime = subscriber.lifetime_token,
            .callback = callback,
        });
    }


    void notify_update() {  // TODO implement thread safe alternative ?
        std::erase_if(update_callbacks, [](auto& scb) { return scb.subscriber_lifetime.expired(); });
        for (auto& scb : update_callbacks) {
            scb.callback();
        }
    }


    void display() {
        ImVec2 display_region = ImGui::GetContentRegionAvail();
        ImVec2 text_size = ImGui::CalcTextSize(name.c_str());

        float start_x = ImGui::GetCursorPosX();
        float start_y = ImGui::GetCursorPosY();

        display_region.y -= 1.5 * text_size.y;

        ImVec2 display_dim;
        ImVec2 tex_to_display_ratio(
            display_region.x / data.width,
            display_region.y / data.height
        );

        if (tex_to_display_ratio.x < tex_to_display_ratio.y) {
            display_dim.x = tex_to_display_ratio.x * data.width;
            display_dim.y = tex_to_display_ratio.x * data.height;
        } else {
            display_dim.x = tex_to_display_ratio.y * data.width;
            display_dim.y = tex_to_display_ratio.y * data.height;
        }

        ImGui::SetCursorPos(ImVec2(
            -(display_dim.x - display_region.x) * 0.5 + start_x,
            -(display_dim.y - display_region.y) * 0.5 + start_y
        ));

        ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<WGPUTextureView>(*texture_view)), display_dim);
        
        ImGui::SetCursorPosX((display_region.x - text_size.x) / 2);
        ImGui::Text("%s", name.c_str());
    }
};



struct ResourceManager {
    const GPU& gpu;
    wgpu::raii::Sampler default_texture_sampler;

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


    // private:
    std::unordered_map<size_t, size_t> images_index_map;
    std::vector<Resource<ResourceKind::Image>> images;

    static size_t next_id() {
        static size_t id = 0;
        return id++;
    }
};
