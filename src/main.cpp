#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>
#include <imgui.h>
#include <stdio.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu-raii.hpp>

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #include <emscripten/html5_webgpu.h>
#endif

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
    #include "../libs/emscripten/emscripten_mainloop_stub.h"  // TODO
#endif

#include "shader_parameter.hpp"

// Global WebGPU required states
static wgpu::raii::Instance wgpu_instance;
static wgpu::raii::Device wgpu_device;
static wgpu::raii::Surface wgpu_surface;
static wgpu::SurfaceConfiguration wgpu_surface_config;
static wgpu::TextureFormat wgpu_preferred_fmt = WGPUTextureFormat_RGBA8Unorm;
static int wgpu_swap_chain_width = 1280;
static int wgpu_swap_chain_height = 720;

// Forward declarations
static bool InitWGPU(GLFWwindow* window);
// static void CreateSwapChain(int width, int height);

static void glfw_error_callback(int error, const char* description) {
    printf("GLFW Error %d: %s\n", error, description);
}
void framebufferResizeCallback(GLFWwindow* window, int newWidth, int newHeight) {
    wgpu_surface_config.width = newWidth;
    wgpu_surface_config.height = newHeight;

    wgpu_surface->configure(wgpu_surface_config);
}

// Main code
int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    // Make sure GLFW does not initialize any graphics context.
    // This needs to be done explicitly later.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        wgpu_swap_chain_width,
        wgpu_swap_chain_height,
        "Dear ImGui GLFW+WebGPU example",
        nullptr,
        nullptr
    );
    if (window == nullptr) return 1;

    // Initialize the WebGPU environment
    if (!InitWGPU(window)) {
        if (window) glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    glfwShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOther(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = *wgpu_device;
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = wgpu_preferred_fmt;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use
    // ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your
    // application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double
    // backslash \\ !
    // - Emscripten allows preloading a file or folder to be accessible at runtime. See Makefile for details.
    // io.Fonts->AddFontDefault();
#ifndef IMGUI_DISABLE_FILE_FUNCTIONS
    // io.Fonts->AddFontFromFileTTF("fonts/segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("fonts/ArialUni.ttf", 18.0f, nullptr,
    // io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);
#endif

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    UserParam::Integer<UserParam::WidgetKind::Slider> int_slider("int slider", 0, 10, 0);
    UserParam::Integer<UserParam::WidgetKind::Field> int_field("int field", 0, 10, 0);

    UserParam::Float<UserParam::WidgetKind::Slider> float_slider("float slider", 0, 1200, 0);
    UserParam::Float<UserParam::WidgetKind::Field> float_field("float field", 0, 1200, 0);


    // UserParam::WidgetGroup params(int_slider, int_field, float_slider, float_field);
    // UserParam::Feature<UserParam::WidgetKind::Checkbox, decltype(params)> enable("enable", false, params);
    // UserParam::Feature<UserParam::WidgetKind::Checkbox, void> enable("enable", false);

    UserParam::WidgetGroup int_params(int_slider, int_field);
    UserParam::WidgetGroup float_params(float_slider, float_field);

    UserParam::Choice<UserParam::WidgetKind::Dropdown, void, decltype(int_params), decltype(float_params)>
        enable("number type", 0, {"none", "int", "float"}, std::monostate{}, int_params, float_params);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the
    // imgui.ini file. You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your
        // inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or
        // clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or
        // clear/overwrite your copy of the keyboard data. Generally you may always pass all inputs to dear imgui, and
        // hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // React to changes in screen size
        int width, height;
        glfwGetFramebufferSize((GLFWwindow*)window, &width, &height);
        if (width != wgpu_swap_chain_width || height != wgpu_swap_chain_height) {
            ImGui_ImplWGPU_InvalidateDeviceObjects();
            // CreateSwapChain(width, height);
            ImGui_ImplWGPU_CreateDeviceObjects();
        }

        // Start the Dear ImGui frame
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        UserParam::window(enable);


        // Rendering
        ImGui::Render();

#ifdef IMGUI_IMPL_WEBGPU_BACKEND_DAWN
        // Tick needs to be called in Dawn to display validation errors
        wgpu_device->tick();
#elifdef IMGUI_IMPL_WEBGPU_BACKEND_WGPU
        wgpu_device->poll(false, nullptr);
#endif

        wgpu::SurfaceTexture surface_texture;
        wgpu_surface->getCurrentTexture(&surface_texture);

        wgpu::raii::TextureView textureView;
        *textureView = wgpuTextureCreateView(surface_texture.texture, NULL);

        wgpu::RenderPassColorAttachment color_attachments = {};
        color_attachments.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
        color_attachments.loadOp = WGPULoadOp_Clear;
        color_attachments.storeOp = WGPUStoreOp_Store;
        color_attachments.clearValue = {
            clear_color.x * clear_color.w,
            clear_color.y * clear_color.w,
            clear_color.z * clear_color.w,
            clear_color.w
        };
        color_attachments.view = *textureView;

        wgpu::RenderPassDescriptor render_pass_desc = {};
        render_pass_desc.colorAttachmentCount = 1;
        render_pass_desc.colorAttachments = &color_attachments;
        render_pass_desc.depthStencilAttachment = nullptr;

        wgpu::CommandEncoderDescriptor enc_desc = {};
        wgpu::raii::CommandEncoder encoder = wgpu_device->createCommandEncoder(enc_desc);

        wgpu::raii::RenderPassEncoder pass = encoder->beginRenderPass(render_pass_desc);
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), *pass);
        wgpuRenderPassEncoderEnd(*pass);

        wgpu::CommandBufferDescriptor cmd_buffer_desc = {};
        wgpu::raii::CommandBuffer cmd_buffer = encoder->finish(cmd_buffer_desc);
        wgpu::raii::Queue queue = wgpu_device->getQueue();
        queue->submit(1, &(*cmd_buffer));
        // queue->release();
        // cmd_buffer->release();
        // pass->release();
        // encoder->release();
        // textureView->release();

#ifndef __EMSCRIPTEN__
        // wgpuSwapChainPresent(wgpu_swap_chain); // TODO
#endif
        wgpu_surface->present();
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    wgpu_surface->release(); // released manually as it must

    glfwDestroyWindow(window);
    glfwTerminate();


    return 0;
}

#ifndef __EMSCRIPTEN__
static wgpu::Adapter request_adapter(wgpu::Instance instance) {
    return wgpu_instance->requestAdapter(wgpu::RequestAdapterOptions{});
}

static wgpu::Device request_device(wgpu::Adapter& adapter) { return adapter.requestDevice(wgpu::DeviceDescriptor{}); }
#endif

static bool InitWGPU(GLFWwindow* window) {
    wgpu_instance = wgpu::createInstance();

#ifdef __EMSCRIPTEN__
    // wgpu_device = emscripten_webgpu_get_device();
    // if (!wgpu_device) return false;
#else
    wgpu::raii::Adapter adapter = request_adapter(*wgpu_instance);
    if (!adapter) return false;
    wgpu_device = request_device(*adapter);
#endif

#ifdef __EMSCRIPTEN__
    // wgpu::SurfaceDescriptorFromCanvasHTMLSelector html_surface_desc = {};
    // html_surface_desc.selector = "#canvas";
    // wgpu::SurfaceDescriptor surface_desc = {};
    // surface_desc.nextInChain = &html_surface_desc;
    // wgpu::Surface surface = instance.CreateSurface(&surface_desc);

    // wgpu::Adapter adapter = {};
    // wgpu_preferred_fmt = (WGPUTextureFormat)surface.GetPreferredFormat(adapter);
#else
    struct wl_display* wayland_display = glfwGetWaylandDisplay();
    struct wl_surface* wayland_surface = glfwGetWaylandWindow(window);

    wgpu::SurfaceSourceWaylandSurface fromWaylandSurface;
    fromWaylandSurface.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
    fromWaylandSurface.chain.next = nullptr;
    fromWaylandSurface.display = wayland_display;
    fromWaylandSurface.surface = wayland_surface;

    wgpu::SurfaceDescriptor surfaceDescriptor;
    surfaceDescriptor.nextInChain = &fromWaylandSurface.chain;
    surfaceDescriptor.label = (WGPUStringView){NULL, WGPU_STRLEN};

    wgpu_surface = wgpu_instance->createSurface(surfaceDescriptor);
    if (!*wgpu_surface) return false;

    wgpu::SurfaceCapabilities sc;
    wgpu_surface->getCapabilities(*adapter, &sc);
    // std::cout << std::hex;
    // for (int i = 0; i < sc.presentModeCount; i++) {
    //     std::cout << "0x" << sc.presentModes[i] << std::endl;
    // }
    // std::cout << std::dec;
    bool format_found = false;
    for (size_t i = 0; i < sc.formatCount; i++) {
        if (sc.formats[i] == WGPUTextureFormat_RGBA8Unorm) {
            format_found = true;
        }
    }
    if (!format_found) return false;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    wgpu_surface_config.nextInChain = NULL;
    wgpu_surface_config.device = *wgpu_device;
    wgpu_surface_config.format = WGPUTextureFormat_RGBA8Unorm;
    wgpu_surface_config.usage = WGPUTextureUsage_RenderAttachment;
    wgpu_surface_config.presentMode = WGPUPresentMode_Fifo;
    wgpu_surface_config.width = width;
    wgpu_surface_config.height = height;
    wgpu_surface_config.viewFormatCount = 0;
    wgpu_surface_config.viewFormats = NULL;

    wgpu_surface->configure(wgpu_surface_config);

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

#endif

    return true;
}

// static void CreateSwapChain(int width, int height) {
//     if (wgpu_swap_chain) wgpuSwapChainRelease(wgpu_swap_chain);
//     wgpu_swap_chain_width = width;
//     wgpu_swap_chain_height = height;
//     WGPUSwapChainDescriptor swap_chain_desc = {};
//     swap_chain_desc.usage = WGPUTextureUsage_RenderAttachment;
//     swap_chain_desc.format = wgpu_preferred_fmt;
//     swap_chain_desc.width = width;
//     swap_chain_desc.height = height;
//     swap_chain_desc.presentMode = WGPUPresentMode_Fifo;
//     wgpu_swap_chain = wgpuDeviceCreateSwapChain(wgpu_device, wgpu_surface, &swap_chain_desc);
// }
