struct DefaultUniforms {
    viewport_size: vec2<u32>,
    time: f32,
};

@group(0) @binding(0) var input_tex : texture_2d<f32>;
@group(0) @binding(1) var input_sampler : sampler;
@group(0) @binding(2) var<uniform> default_uniforms : DefaultUniforms;

struct Uniforms {
    speed : f32,
    freq: f32,
    shift: vec2<f32>,
    thickness: f32,
    mode: u32,
};

@group(1) @binding(0) var<uniform> uniforms: Uniforms;

fn fullscreen_uv(coord: vec2<f32>) -> vec2<f32> {
    return coord / vec2<f32>(default_uniforms.viewport_size);
}

@fragment fn fs_main(@builtin(position) coord: vec4<f32>) -> @location(0) vec4<f32> {
    let vs = vec2<f32>(default_uniforms.viewport_size);
    let time = default_uniforms.time;
    let freq = uniforms.freq;
    let shift = uniforms.shift / vs;
    var uv = fullscreen_uv(coord.xy);
    switch (uniforms.mode) {
        case 0u {
            let speed = uniforms.speed / vs.y;
            let thickness = uniforms.thickness / vs.y;
            uv += vec2<f32>(f32((abs((uv.y - (time * speed))) % (1.0/freq)) < thickness)) * shift;
        }
        case 1u {
            let speed = uniforms.speed / vs.x;
            let thickness = uniforms.thickness / vs.x;
            uv += vec2<f32>(f32((abs((uv.x - (time * speed))) % (1.0/freq)) < thickness)) * shift;
        }
        default {}
    }
        
    let color = textureSample(input_tex, input_sampler, uv);
    return color;
}
