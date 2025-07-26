#pragma once

#include <webgpu/webgpu-raii.hpp>

#include "context/gpu.hpp"
#include "context/render_target.hpp"
#include "context/resource.hpp"
#include "context/shader_source.hpp"

struct Context {
    GPU gpu;
    RenderTarget render_target;
    ShaderSourceCache shader_source_cache;
    ResourceManager resource_manager;

    Context() : gpu(), render_target(), shader_source_cache(gpu), resource_manager(gpu) {}
};
