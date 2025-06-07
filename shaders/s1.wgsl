struct Uniforms {
    viewport_size: vec2<u32>,
    time: f32,
};

@group(0) @binding(0) var input_tex: texture_2d<f32>;
@group(0) @binding(1) var input_sampler: sampler;
@group(0) @binding(2) var<uniform> uniforms: Uniforms;

@fragment
fn fs_main(@builtin(position) coord: vec4<f32>) -> @location(0) vec4<f32> {
    let uv = coord.xy / vec2<f32>(uniforms.viewport_size);
    let color = textureSample(input_tex, input_sampler, uv);
    return vec4<f32>(1.0);
}
