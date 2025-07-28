#include <stdexcept>
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #include <emscripten/html5_webgpu.h>
#endif

#include "gpu.hpp"
#include "src/log.hpp"

bool GPU::init() {
    if (initialized) {
        Log::warn("GPU context already initialized");
        return false;
    }

    this->instance = wgpu::createInstance();

#ifdef __EMSCRIPTEN__
    this->device = wgpu::raii::Device(emscripten_webgpu_get_device());
#else
    this->adapter = this->instance->requestAdapter(wgpu::RequestAdapterOptions{});
    if (!adapter) {
        Log::error("Adapter request failed.");
        return false;
    }

    this->device = adapter->requestDevice(wgpu::DeviceDescriptor{});
#endif

#ifndef __EMSCRIPTEN__
#ifdef DEBUG
    wgpuSetLogLevel(WGPULogLevel_Debug);
#endif
    wgpuSetLogCallback(
        [](WGPULogLevel level, WGPUStringView message, void* _) {
            log(level, "WGPU: " + std::string(message.data, message.length));
        },
        nullptr
    );
#endif

    initialized = true;
    return true;
}

bool GPU::is_initialized() const {
    return initialized;
}

const wgpu::Instance& GPU::get_instance() const {
    if (!initialized) {
        Log::error("GPU context not initialized.");
        throw std::runtime_error("GPU context not initialized.");
    }
    return *instance;
}


#ifndef __EMSCRIPTEN__
const wgpu::Adapter& GPU::get_adapter() const {
    if (!initialized) {
        Log::error("GPU context not initialized.");
        throw std::runtime_error("GPU context not initialized.");
    }
    return *adapter;
}
#endif


const wgpu::Device& GPU::get_device() const {
    if (!initialized) {
        Log::error("GPU context not initialized.");
        throw std::runtime_error("GPU context not initialized.");
    }
    return *device;
}
