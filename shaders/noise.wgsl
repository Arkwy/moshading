const PI: f32 = 3.14159265;

struct Uniforms {
    viewport_size: vec2<u32>,
    time: f32,
};

@group(0) @binding(0) var input_tex: texture_2d<f32>;
@group(0) @binding(1) var input_sampler: sampler;
@group(0) @binding(2) var<uniform> uniforms: Uniforms;


struct NoiseUniforms {
    min_rgb: vec3<f32>,
    min: f32,
    max_rgb: vec3<f32>,
    max: f32,
    control: u32, // 1 = color mode, 2 = dynamic mode
    seed: u32,
};

@group(1) @binding(0) var<uniform> parameters: NoiseUniforms;

fn color_mode() -> bool {
    return bool(parameters.control & 1u);
}

fn dynamic_mode() -> bool {
    return bool(parameters.control & 2u);
}

fn fullscreen_uv(coord : vec2<f32>) -> vec2<f32> {
    return coord / vec2<f32>(uniforms.viewport_size);
}

fn pcg3d(vec: vec3<u32>) -> vec3<u32> {
    // from: http://www.jcgt.org/published/0009/03/02/
    var v = vec * vec3<u32>(1664525u) + vec3<u32>(1013904223u);
    v.x += v.y*v.z; v.y += v.z*v.x; v.z += v.x*v.y;
    v.x ^= v.x >> 16u; v.y ^= v.y >> 16u; v.z ^= v.z >> 16u;
    v.x += v.y*v.z; v.y += v.z*v.x; v.z += v.x*v.y;
    return v;
}
fn rand(coord : vec2<f32>, seed: u32, dynamic: bool) -> vec3<f32> {
    if (dynamic) {
        let time = u32(uniforms.time * 200);
        let v = vec3<u32>(vec2<u32>(coord), seed + u32(uniforms.time*1000));
        return ldexp(vec3<f32>(pcg3d(v)), vec3<i32>(-32));
    } else {
        let v = vec3<u32>(vec2<u32>(coord), seed);
        return ldexp(vec3<f32>(pcg3d(v)), vec3<i32>(-32));
    }
}


@fragment fn fs_main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
    let uv = fullscreen_uv(coord.xy);
    let time = uniforms.time;

    var color = textureSample(input_tex, input_sampler, uv);

    var noise = rand(coord.xy, parameters.seed, dynamic_mode());
    if (color_mode()) {
        noise = parameters.min_rgb + noise * (parameters.max_rgb - parameters.min_rgb);
    } else {
        noise = parameters.min + vec3<f32>(noise.x) * (parameters.max - parameters.min);
    }

    color += vec4<f32>(noise, 0.);

    return color;
}
