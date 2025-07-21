#pragma once

#include <webgpu/webgpu-raii.hpp>

#include "context/gpu.hpp"
#include "context/rendering.hpp"
#include "context/cache.hpp"
#include "src/file_loader.hpp"

struct Context {
    GPU gpu;
    Rendering rendering;
    ShaderSourceCache cache;

    Context(): gpu(), rendering(), cache(gpu) {}
};
