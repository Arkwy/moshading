#pragma once

#include <memory>
#include <webgpu/webgpu-raii.hpp>

#include "gpu.hpp"
#include "webgpu/webgpu.hpp"

struct ShaderSource {
    const char* const code;
#ifdef __EMSCRIPTEN__
    const wgpu::ShaderModuleWGSLDescriptor source;
#else
    const wgpu::ShaderSourceWGSL source;
#endif
    const wgpu::ShaderModuleDescriptor module_desc;

    const wgpu::raii::ShaderModule compiled_module;

    ShaderSource(const GPU& gpu, const char* const code)
        : code(code),
          source(make_source(code)),
          module_desc(make_module_desc(source)),
          compiled_module(make_module(gpu, module_desc)) {}


  private:
#ifdef __EMSCRIPTEN__
    static const wgpu::ShaderModuleWGSLDescriptor make_source(const char* const code) {
        wgpu::ShaderModuleWGSLDescriptor source;
        source.code = code;
        source.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
#else
    static const wgpu::ShaderSourceWGSL make_source(const char* const code) {
        wgpu::ShaderSourceWGSL source;
        source.code.data = code;
        source.code.length = WGPU_STRLEN;
        source.chain.sType = WGPUSType_ShaderSourceWGSL;
#endif
        source.chain.next = nullptr;

        return source;
    }

#ifdef __EMSCRIPTEN__
    static const wgpu::ShaderModuleDescriptor make_module_desc(const wgpu::ShaderModuleWGSLDescriptor& source) {
#else
    static const wgpu::ShaderModuleDescriptor make_module_desc(const wgpu::ShaderSourceWGSL& source) {
#endif
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
    const GPU& gpu;

    ShaderSourceCache(const GPU& gpu) : sources(), gpu(gpu) {}

    const ShaderSource& get(const char* const source_code) const {
        if (!sources.contains(source_code)) {
            sources[source_code] = std::make_unique<ShaderSource>(gpu, source_code);
        }
        return *sources[source_code].get();
    }
};
