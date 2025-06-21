struct Uniforms {
    viewport_size: vec2<u32>,
    time: f32,
};

@group(0) @binding(0) var input_tex: texture_2d<f32>;
@group(0) @binding(1) var input_sampler: sampler;
@group(0) @binding(2) var<uniform> uniforms: Uniforms;

fn fullscreen_uv(coord : vec2<f32>) -> vec2<f32> {
    return coord / vec2<f32>(uniforms.viewport_size);
}

fn centered_uv(coord : vec2<f32>) -> vec2<f32> {
    var uv = coord;
    let vs = vec2<f32>(uniforms.viewport_size);
    let m = min(vs.x, vs.y);
    uv *= 2.0 / m;
    uv -= vec2<f32>(vs.x / m, vs.y / m);
    return uv;
}

@fragment fn fs_main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
    let uv = (centered_uv(coord.xy) + 1.0) / 2.0;
    let color = textureSample(input_tex, input_sampler, fullscreen_uv(coord.xy));
    // return vec4<f32>(sin(uv * 10.0 + uniforms.time), 1.0, 1.0);
    return vec4<f32>(uv, 1.0, 1.0);
}
