#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_wgpu.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <wayland-client-protocol.h>
#include <wayland-version.h>

#include <webgpu/webgpu-raii.hpp>

#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>
// #include <wayland-client.h>

#include "renderer.hpp"
#include "src/log.hpp"




// static void handle_surface_enter(void* opaque_handle, struct wl_surface* surface, struct wl_output* output) {
//     Log::warn("Surface enter");
//     Renderer* renderer = reinterpret_cast<Renderer*>(opaque_handle);
//     renderer->resume_rendering();
// }

// static void handle_surface_leave(void* opaque_handle, struct wl_surface* surface, struct wl_output* output) {
//     Log::warn("Surface leave");
//     Renderer* renderer = reinterpret_cast<Renderer*>(opaque_handle);
//     renderer->pause_rendering();
// }

// static const wl_surface_listener listener = {
//     .enter = handle_surface_enter,
//     .leave = handle_surface_leave,
//     .preferred_buffer_scale = nullptr,
//     .preferred_buffer_transform = nullptr,
// };

static void glfw_error_callback(int error, const char* description) {
    Log::error("GLFW Error code {}: {}", error, description);
}


static void resize_callback(GLFWwindow* window, int new_width, int new_height) {
    Renderer& app = *static_cast<Renderer*>(glfwGetWindowUserPointer(window));

    app.surface_config.width = new_width;
    app.surface_config.height = new_height;

    app.surface->configure(app.surface_config);
}

bool Renderer::init() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return false;

    // Make sure GLFW does not initialize any graphics context.
    // This needs to be done explicitly later.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(100, 100, "Moshading", nullptr, nullptr);

    if (window == nullptr) {
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(window, this);

    wl_display* wayland_display = glfwGetWaylandDisplay();
    wl_surface* wayland_surface = glfwGetWaylandWindow(window);

    // wl_surface_add_listener(wayland_surface, &listener, this);

    wgpu::SurfaceSourceWaylandSurface from_wayland_surface;
    from_wayland_surface.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
    from_wayland_surface.chain.next = nullptr;
    from_wayland_surface.display = wayland_display;
    from_wayland_surface.surface = wayland_surface;

    wgpu::SurfaceDescriptor surface_descriptor;
    surface_descriptor.nextInChain = &from_wayland_surface.chain;
    surface_descriptor.label = WGPUStringView{nullptr, WGPU_STRLEN};

    surface = ctx.gpu.get_instance().createSurface(surface_descriptor);
    if (!*surface) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    wgpu::SurfaceCapabilities sc;
    surface->getCapabilities(ctx.gpu.get_adapter(), &sc);

    bool format_found = false;
    for (size_t i = 0; i < sc.formatCount; i++) {
        if (sc.formats[i] == WGPUTextureFormat_RGBA8Unorm) {
            format_found = true;
        }
    }
    if (!format_found) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    surface_config.nextInChain = nullptr;
    surface_config.device = ctx.gpu.get_device();
    surface_config.format = WGPUTextureFormat_RGBA8Unorm;
    surface_config.usage = WGPUTextureUsage_RenderAttachment;
    surface_config.presentMode =
        WGPUPresentMode_Immediate;  // TODO switch back to FIFO when out of screen detection works
    surface_config.width = width;
    surface_config.height = height;
    surface_config.viewFormatCount = 0;
    surface_config.viewFormats = nullptr;

    surface->configure(surface_config);

    glfwSetFramebufferSizeCallback(window, resize_callback);

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

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOther(window, true);

    ImGui_ImplWGPU_InitInfo init_info;
    init_info.Device = ctx.gpu.get_device();
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = preferred_fmt;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);

    return true;
}


void Renderer::main_loop() {
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) || !glfwGetWindowAttrib(window, GLFW_VISIBLE)) {
        // TODO find out why this never triggers
        ImGui_ImplGlfw_Sleep(10);
        return;
    }

    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    if (fb_width == 0 || fb_height == 0) {
        ImGui_ImplGlfw_Sleep(10);
        return;
    }

    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    display_app();

    // Rendering
    ImGui::Render();

#ifdef IMGUI_IMPL_WEBGPU_BACKEND_DAWN
    ctx.gpu.get_device().tick();
#elifdef IMGUI_IMPL_WEBGPU_BACKEND_WGPU
    ctx.gpu.get_device().poll(false, nullptr);
#endif


    wgpu::SurfaceTexture surface_texture;
    surface->getCurrentTexture(&surface_texture);

    if (!surface_texture.texture) {
        Log::warn("Surface texture is null — skipping frame");
        return;
    }

    if (surface_texture.status == WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
        Log::error("Suboptimal frame, skipping. (should implement better solution ?)");  // TODO ?
        return;
    }

    wgpu::raii::TextureView texture_view(wgpuTextureCreateView(surface_texture.texture, nullptr));

    wgpu::RenderPassColorAttachment color_attachments = {};
    color_attachments.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    color_attachments.loadOp = wgpu::LoadOp::Clear;
    color_attachments.storeOp = wgpu::StoreOp::Store;
    color_attachments.clearValue = {0, 100, 200, 255};
    color_attachments.view = *texture_view;

    wgpu::RenderPassDescriptor render_pass_desc = {};
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachments;
    render_pass_desc.depthStencilAttachment = nullptr;

    wgpu::CommandEncoderDescriptor enc_desc = {};
    wgpu::raii::CommandEncoder encoder = ctx.gpu.get_device().createCommandEncoder(enc_desc);

    wgpu::raii::RenderPassEncoder pass = encoder->beginRenderPass(render_pass_desc);

    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), *pass);
    pass->end();

    wgpu::CommandBufferDescriptor cmd_buffer_desc = {};
    wgpu::raii::CommandBuffer cmd_buffer = encoder->finish(cmd_buffer_desc);
    wgpu::raii::Queue queue = ctx.gpu.get_device().getQueue();
    queue->submit(1, &(*cmd_buffer));

    surface->present();

    fps_limiter(100);
}
