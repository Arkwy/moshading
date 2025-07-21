#pragma once

#include <memory>
#include <webgpu/webgpu-raii.hpp>

#include "gpu.hpp"
#include "src/file_loader.hpp"

struct ShaderSource {
    const char* const code;
    const wgpu::ShaderSourceWGSL source;
    const wgpu::ShaderModuleDescriptor module_desc;

    const wgpu::raii::ShaderModule compiled_module;

    ShaderSource(const GPU& gpu, const char* const code)
        : code(code),
          source(make_source(code)),
          module_desc(make_module_desc(source)),
          compiled_module(make_module(gpu, module_desc)) {}


  private:
    static const wgpu::ShaderSourceWGSL make_source(const char* const code) {
        wgpu::ShaderSourceWGSL source;
#ifdef __EMSCRIPTEN__
        source.code = code;
        source.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
#else
        source.code.data = code;
        source.code.length = WGPU_STRLEN;
        source.chain.sType = WGPUSType_ShaderSourceWGSL;
#endif
        source.chain.next = nullptr;

        return source;
    }

    static const wgpu::ShaderModuleDescriptor make_module_desc(const wgpu::ShaderSourceWGSL& source) {
        wgpu::ShaderModuleDescriptor module_desc;
        module_desc.nextInChain = &source.chain;
        return module_desc;
    }

    static const wgpu::raii::ShaderModule make_module(const GPU& gpu, const wgpu::ShaderModuleDescriptor& module_desc) {
        return gpu.get_device().createShaderModule(module_desc);
    }
};


struct ShaderSourceCache {
    mutable std::unordered_map<const char*, std::unique_ptr<ShaderSource>> sources;
    FileLoader file_loader;
    const GPU& gpu;

    ShaderSourceCache(const GPU& gpu) : sources(), file_loader(), gpu(gpu) {}

    const ShaderSource& get(const char* const source_code) const {
        if (!sources.contains(source_code)) {
            sources[source_code] = std::make_unique<ShaderSource>(gpu, source_code);
        }
        return *sources[source_code].get();
    }
};
