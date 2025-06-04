#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>
#include <webgpu/webgpu-raii.hpp>
#include <GLFW/glfw3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>

#include "renderer.hpp"


void glfw_error_callback(int error, const char* description) {
    printf("GLFW Error %d: %s\n", error, description);
}


EM_BOOL resize_callback(int eventType, const EmscriptenUiEvent *uiEvent, void *userData) {
    int width = uiEvent->windowInnerWidth;
    int height = uiEvent->windowInnerHeight;
    printf("Window resized to: %d x %d\n", width, height);
    return EM_TRUE;
}


bool Renderer::init() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return false;

    // Make sure GLFW does not initialize any graphics context.
    // This needs to be done explicitly later.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(
        100,
        100,
        "Dear ImGui GLFW+WebGPU example",
        nullptr,
        nullptr
    );
    if (window == nullptr) {
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(window, this);


    // Setup the surface descriptor for HTML canvas
    WGPUSurfaceDescriptorFromCanvasHTMLSelector html_surface_desc = {};
    html_surface_desc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
    html_surface_desc.chain.next = nullptr;
    html_surface_desc.selector = "#canvas";

    WGPUSurfaceDescriptor surface_desc = {};
    surface_desc.nextInChain = &html_surface_desc.chain;
    surface_desc.label = nullptr;

    // Get surface via emscripten API
    // wgpu_surface = wgpu_instance->createSurface(surface_desc);
    surface = wgpu::raii::Surface(wgpuInstanceCreateSurface(ctx.get_instance(), &surface_desc));

    // Set preferred format (no adapter needed in emscripten)
    preferred_fmt = wgpuSurfaceGetPreferredFormat(*surface, nullptr);

    // Get canvas size from GLFW
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // Configure surface
    surface_config.nextInChain = nullptr;
    surface_config.device = ctx.get_device();
    surface_config.format = preferred_fmt;
    surface_config.usage = WGPUTextureUsage_RenderAttachment;
    surface_config.presentMode = WGPUPresentMode_Fifo;
    surface_config.width = width;
    surface_config.height = height;
    surface_config.viewFormatCount = 0;
    surface_config.viewFormats = nullptr;

    surface->configure(surface_config);

    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, resize_callback);

    glfwShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");

    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = ctx.get_device();
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = preferred_fmt;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);

    io.IniFilename = nullptr;

    return true;
}


void Renderer::main_loop() {
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // std::cout << &this->app << std::endl;
    display_app();
    // ImGui::Begin("some window");
    // ImGui::End();

    // Rendering
    ImGui::Render();

    wgpu::SurfaceTexture surface_texture;
    surface->getCurrentTexture(&surface_texture);

    wgpu::raii::TextureView textureView;
    *textureView = wgpuTextureCreateView(surface_texture.texture, NULL);

    wgpu::RenderPassColorAttachment color_attachments = {};
    color_attachments.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    color_attachments.loadOp = WGPULoadOp_Clear;
    color_attachments.storeOp = WGPUStoreOp_Store;
    color_attachments.clearValue = {0, 100, 200};
    color_attachments.view = *textureView;

    wgpu::RenderPassDescriptor render_pass_desc = {};
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachments;
    render_pass_desc.depthStencilAttachment = nullptr;

    wgpu::CommandEncoderDescriptor enc_desc = {};
    wgpu::raii::CommandEncoder encoder = ctx.get_device().createCommandEncoder(enc_desc);

    wgpu::raii::RenderPassEncoder pass = encoder->beginRenderPass(render_pass_desc);
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), *pass);
    wgpuRenderPassEncoderEnd(*pass);

    wgpu::CommandBufferDescriptor cmd_buffer_desc = {};
    wgpu::raii::CommandBuffer cmd_buffer = encoder->finish(cmd_buffer_desc);
    wgpu::raii::Queue queue = ctx.get_device().getQueue();
    queue->submit(1, &(*cmd_buffer));
}

