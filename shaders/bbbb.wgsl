@group(0) @binding(0) var input_tex: texture_2d<f32>;
@group(0) @binding(1) var input_sampler: sampler;

@fragment
fn fs_main(@builtin(position) coord: vec4<f32>) -> @location(0) vec4<f32> {
    let uv = coord.xy / vec2<f32>(textureDimensions(input_tex, 0));
    let color = textureSample(input_tex, input_sampler, uv);
    return vec4<f32>(uv.x, 0.0, 0.0, 1.0); // tint red
}
