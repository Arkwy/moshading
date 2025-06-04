#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>
#include <imgui.h>
#include <webgpu/webgpu.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu-raii.hpp>

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #include <emscripten/html5_webgpu.h>
#else
    #define GLFW_EXPOSE_NATIVE_WAYLAND
    #define GLFW_NATIVE_INCLUDE_NONE
    #include <GLFW/glfw3native.h>
#endif

#include "renderer.hpp"
#include "gpu_context.hpp"

int main() {
    GPUContext ctx;
    if (!ctx.init()) {
        return 1;
    }

    Renderer renderer(ctx);

    if (!renderer.init()) {
        return 1;
    }
    
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg([](void* renderer)->void{static_cast<Renderer*>(renderer)->main_loop();}, &renderer, 0, true);
#else
    while (renderer.is_running()) renderer.main_loop();
    ctx.get_device().poll(true, nullptr); // make sure every command terminates before quitting
#endif


    renderer.terminate();


    return 0;
}
