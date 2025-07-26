#include <stb/stb_image.h>

#include <cstddef>
#include <filesystem>
#include <functional>
#include <webgpu/webgpu-raii.hpp>
#include "src/context/gpu.hpp"
#include "src/log.hpp"

using ResourceUpdateCallback = std::function<void()>;

enum class ResourceKind {
    Image,
    // Video, TODO
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

    Data data;

    bool uploaded = false;
    wgpu::raii::Texture texture;
    wgpu::raii::TextureView texture_view;
    
    mutable std::vector<ResourceUpdateCallback> update_callbacks;
    
    ~Resource() {
        if (data.ptr)
            stbi_image_free(data.ptr);
    }


    Resource(const Handle& handle, const GPU& gpu, bool upload = true) {
        update(handle, gpu, upload);
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

        if (upload)
            upload_to_gpu(gpu);

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


    void subscribe(ResourceUpdateCallback callback) const {
        update_callbacks.push_back(std::move(callback));
    }


    void notify_update() {
        for (auto& cb : update_callbacks) {
            cb();
        }
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

    void add_image(const Resource<ResourceKind::Image>::Handle& handle) {
        images.push_back(Resource<ResourceKind::Image>(handle, gpu));
        resource_index_map[next_id()] = images.size() - 1;
    }

    const Resource<ResourceKind::Image>& get_image(size_t id) const {
        return images[resource_index_map.at(id)];
    }
    

  private:
    std::unordered_map<size_t, size_t> resource_index_map;
    std::vector<Resource<ResourceKind::Image>> images;

    static size_t next_id() {
        static size_t id = 0;
        return id++;
    }
};
