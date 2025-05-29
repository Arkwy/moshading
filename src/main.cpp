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


#include "app.hpp"

int main() {
    App app;
    if (!app.initialize()) {
        return 1;
    }
    
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg([](void* app)->void{static_cast<App*>(app)->main_loop();}, &app, 0, true);
#else
    while (app.is_running()) app.main_loop();
#endif

    app.terminate();

    return 0;
}
