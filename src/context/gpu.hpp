#pragma once

#include <webgpu/webgpu-raii.hpp>

struct GPU {
    GPU() {
        init();
    }

    bool is_initialized() const;

    const wgpu::Instance& get_instance() const;
#ifndef __EMSCRIPEN__
    const wgpu::Adapter& get_adapter() const;
#endif
    const wgpu::Device& get_device() const;

  private:
    bool initialized = false;

    wgpu::raii::Instance instance;
#ifndef __EMSCRIPTEN__  // TODO manage adapter if needed, currently letting emscripten do the work
    wgpu::raii::Adapter adapter;
#endif
    wgpu::raii::Device device;

    bool init();
};
