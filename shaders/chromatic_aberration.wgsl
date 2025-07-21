struct DefaultUniforms {
    viewport_size: vec2<u32>,
    time: f32,
};

@group(0) @binding(0) var input_tex : texture_2d<f32>;
@group(0) @binding(1) var input_sampler : sampler;
@group(0) @binding(2) var<uniform> default_uniforms : DefaultUniforms;

struct Uniforms {
    red_shift: vec2<f32>,
    green_shift: vec2<f32>,
    blue_shift: vec2<f32>,
    scale_center: vec2<f32>,
    scale_intensity: vec3<f32>,
    mode: i32,
};

@group(1) @binding(0) var<uniform> uniforms: Uniforms;

fn fullscreen_uv(coord: vec2<f32>) -> vec2<f32> {
    return coord / vec2<f32>(default_uniforms.viewport_size);
}


fn scale_linear(uv: vec2<f32>, center: vec2<f32>, intensity: f32) -> vec2<f32> {
    return ((uv - center) * (vec2<f32>(1.0) + intensity)) + center;
}


@fragment fn fs_main(@builtin(position) coord: vec4<f32>) -> @location(0) vec4<f32> {
    let uv = fullscreen_uv(coord.xy);

    var red = vec2<f32>(0.0);
    var green = vec2<f32>(0.0);
    var blue = vec2<f32>(0.0);

    let vs = vec2<f32>(default_uniforms.viewport_size);
    switch (uniforms.mode) {
        case 0 {
            red = textureSample(input_tex, input_sampler, uv + uniforms.red_shift / vs).ra;
            green = textureSample(input_tex, input_sampler, uv + uniforms.green_shift / vs).ga;
            blue = textureSample(input_tex, input_sampler, uv + uniforms.blue_shift / vs).ba;
        }
        case 1 {
            red = textureSample(
                input_tex,
                input_sampler,
                scale_linear(uv, uniforms.scale_center / vs, uniforms.scale_intensity.r)
            ).ra;
            green = textureSample(
                input_tex,
                input_sampler,
                scale_linear(uv, uniforms.scale_center / vs, uniforms.scale_intensity.g)
            ).ga;
            blue = textureSample(
                input_tex,
                input_sampler,
                scale_linear(uv, uniforms.scale_center / vs, uniforms.scale_intensity.b)
            ).ba;
        }
        case 2 {
            return vec4<f32>(1.0);
        }
        default {
            return vec4<f32>(0.0);
        }
    }
    return vec4<f32>(red.x, green.x, blue.x, (red.y + green.y + blue.y) / 3.0);
}
