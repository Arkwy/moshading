#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>
#include <webgpu/webgpu-raii.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

#include "app.hpp"


void glfw_error_callback(int error, const char* description) {
    printf("GLFW Error %d: %s\n", error, description);
}


void resize_callback(GLFWwindow* window, int new_width, int new_height) {
    App app = *static_cast<App*>(glfwGetWindowUserPointer(window));

    app.surface_config.width = new_width;
    app.surface_config.height = new_height;

    app.surface->configure(app.surface_config);
}


bool App::initialize() {

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return false;

    // Make sure GLFW does not initialize any graphics context.
    // This needs to be done explicitly later.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    this->window = glfwCreateWindow(
        100,
        100,
        "Dear ImGui GLFW+WebGPU example",
        nullptr,
        nullptr
    );
    if (this->window == nullptr) {
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(this->window, this);

    this->instance = wgpu::createInstance();

    wgpu::raii::Adapter adapter = this->instance->requestAdapter(wgpu::RequestAdapterOptions{});
    if (!adapter) {
        glfwDestroyWindow(this->window);
        glfwTerminate();
        return false;
    }

    this->device = adapter->requestDevice(wgpu::DeviceDescriptor{});
    struct wl_display* wayland_display = glfwGetWaylandDisplay();
    struct wl_surface* wayland_surface = glfwGetWaylandWindow(this->window);

    wgpu::SurfaceSourceWaylandSurface fromWaylandSurface;
    fromWaylandSurface.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
    fromWaylandSurface.chain.next = nullptr;
    fromWaylandSurface.display = wayland_display;
    fromWaylandSurface.surface = wayland_surface;

    wgpu::SurfaceDescriptor surfaceDescriptor;
    surfaceDescriptor.nextInChain = &fromWaylandSurface.chain;
    surfaceDescriptor.label = (WGPUStringView){NULL, WGPU_STRLEN};

    this->surface = this->instance->createSurface(surfaceDescriptor);
    if (!*this->surface) {
        glfwDestroyWindow(this->window);
        glfwTerminate();
        return false;
    }

    wgpu::SurfaceCapabilities sc;
    this->surface->getCapabilities(*adapter, &sc);

    bool format_found = false;
    for (size_t i = 0; i < sc.formatCount; i++) {
        if (sc.formats[i] == WGPUTextureFormat_RGBA8Unorm) {
            format_found = true;
        }
    }
    if (!format_found) {
        glfwDestroyWindow(this->window);
        glfwTerminate();
        return false;
    }

    int width, height;
    glfwGetFramebufferSize(this->window, &width, &height);

    this->surface_config.nextInChain = nullptr;
    this->surface_config.device = *this->device;
    this->surface_config.format = WGPUTextureFormat_RGBA8Unorm;
    this->surface_config.usage = WGPUTextureUsage_RenderAttachment;
    this->surface_config.presentMode = WGPUPresentMode_Fifo;
    this->surface_config.width = width;
    this->surface_config.height = height;
    this->surface_config.viewFormatCount = 0;
    this->surface_config.viewFormats = nullptr;

    this->surface->configure(this->surface_config);

    glfwSetFramebufferSizeCallback(this->window, resize_callback);


    glfwShowWindow(this->window);

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
    ImGui_ImplGlfw_InitForOther(this->window, true);

    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = *this->device;
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = this->preferred_fmt;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);

    return true;
}


void App::terminate() {
    
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    this->surface->release();  // released manually as it must

    glfwDestroyWindow(this->window);
    glfwTerminate();
}


void App::main_loop() {

    glfwPollEvents();
    if (glfwGetWindowAttrib(this->window, GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
        return;
    }


    // Start the Dear ImGui frame
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    // Rendering
    ImGui::Render();

#ifdef IMGUI_IMPL_WEBGPU_BACKEND_DAWN
    // Tick needs to be called in Dawn to display validation errors
    device->tick();
#elifdef IMGUI_IMPL_WEBGPU_BACKEND_WGPU
    this->device->poll(false, nullptr);
#endif


    wgpu::SurfaceTexture surface_texture;
    this->surface->getCurrentTexture(&surface_texture);

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
    wgpu::raii::CommandEncoder encoder = this->device->createCommandEncoder(enc_desc);

    wgpu::raii::RenderPassEncoder pass = encoder->beginRenderPass(render_pass_desc);
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), *pass);
    wgpuRenderPassEncoderEnd(*pass);

    wgpu::CommandBufferDescriptor cmd_buffer_desc = {};
    wgpu::raii::CommandBuffer cmd_buffer = encoder->finish(cmd_buffer_desc);
    wgpu::raii::Queue queue = this->device->getQueue();
    queue->submit(1, &(*cmd_buffer));

    this->surface->present();
}


bool App::is_running() {
    return !glfwWindowShouldClose(this->window);
}
