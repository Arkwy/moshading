// #include "src/shader/shader.hpp"
// #include "webgpu/webgpu-raii.hpp"

// template <>
// struct ShaderDescriptor<ShaderKind::Circle> : public ShaderDescriptorBase<ShaderDescriptor<ShaderKind::Circle>> {
//     struct alignas(16) Uniforms {
//         float center_x;
//         float center_y;
//         float radius;
//         float border_width;
//     };

//     Uniforms uniforms = { 0.0, 0.0, 1.0, 0.1 };

//     void display() {}

//     void write_buffers(wgpu::Queue& queue) {
//         // TODO
//     }

//     static const std::optional<wgpu::BindGroupLayoutDescriptor> make_bind_group_layout_desc() {
//         // Bind group layout
//         wgpu::BindGroupLayoutEntry bgl_entries[1];
//         bgl_entries[0].binding = 0;
//         bgl_entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
//         bgl_entries[0].buffer.type = wgpu::BufferBindingType::Uniform;
//         bgl_entries[0].buffer.hasDynamicOffset = false;
//         bgl_entries[0].buffer.minBindingSize = sizeof(float);

//         wgpu::BindGroupLayoutDescriptor bgl_desc;
//         bgl_desc.entryCount = 1;
//         bgl_desc.entries = bgl_entries;

//         return bgl_desc;
//     }
// };
