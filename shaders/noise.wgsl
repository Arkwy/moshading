const PI: f32 = 3.14159265;

struct Uniforms {
    viewport_size: vec2<u32>,
    time: f32,
};

@group(0) @binding(0) var input_tex: texture_2d<f32>;
@group(0) @binding(1) var input_sampler: sampler;
@group(0) @binding(2) var<uniform> uniforms: Uniforms;


struct NoiseUniforms {
    mean3d: vec3<f32>,
    mean: f32,
    variance3d: vec3<f32>,
    variance: f32,
    mode: i32, // 0 = grey, 1 = color
};

@group(1) @binding(0) var<uniform> noise_uniforms: NoiseUniforms;


fn fullscreen_uv(coord : vec2<f32>) -> vec2<f32> {
    return coord / vec2<f32>(uniforms.viewport_size);
}

fn rand(uv: vec2<f32>, t: f32) -> f32 {
    return fract(sin(dot(uv, vec2<f32>(312.45314, -977.2434))) * 453.98894 + t);
}

fn rand2(uv: vec2<f32>, t: f32) -> f32 {
    return mix(rand(uv, t), rand(vec2<f32>(8.7601 * uv.y + 123.322, 12.431 * uv.x - 4.4533), - t), 0.5);
}

fn gaussian_noise(r1: f32, r2: f32, mean: f32, variance: f32) -> f32 {
    // from https://en.wikipedia.org/wiki/Box-Muller_transform
    let two_pi = 2.0 * PI;

    let mag = variance * sqrt(-2.0 * log(r1));
    return mag * cos(two_pi * r2) + mean;

    // let z0  = mag * cos(two_pi * r2) + mu;
    // let z1  = mag * sin(two_pi * r2) + mu;
    // return vec2<f32>(z0, z1);

}

@fragment fn fs_main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
    let uv = fullscreen_uv(coord.xy);
    let time = uniforms.time;

    var color = textureSample(input_tex, input_sampler, uv);

    // let r2 = vec2<f32>();
    var gauss_noise = vec4<f32>(0.0);
    switch (noise_uniforms.mode) {
        case 0 {
            gauss_noise = vec4<f32>(gaussian_noise(rand2(uv, time), rand2(-uv, -time), noise_uniforms.mean, noise_uniforms.variance));
        }
        case 1 {
            gauss_noise.r = gaussian_noise(rand(uv, time), rand2(-uv, -time), noise_uniforms.mean3d.r, noise_uniforms.variance3d.r);
            gauss_noise.g = gaussian_noise(rand(uv, time + 124.356), rand2(-uv, -time + 124.356), noise_uniforms.mean3d.g, noise_uniforms.variance3d.g);
            gauss_noise.b = gaussian_noise(rand(uv, time + 214.396), rand2(-uv, -time + 214.396), noise_uniforms.mean3d.b, noise_uniforms.variance3d.b);
        }
        default {}
    }

    // color = vec4<f32>(clamp(color.rgb, vec3<f32>(0.025), vec3<f32>(.975)), color.a);

    color = color + gauss_noise; // global
    if (color.r > 1. || color.r < 0.) { color.r -= 2. * gauss_noise.r; }
    if (color.g > 1. || color.g < 0.) { color.g -= 2. * gauss_noise.g; }
    if (color.b > 1. || color.b < 0.) { color.b -= 2. * gauss_noise.b; }

    return color;
}
