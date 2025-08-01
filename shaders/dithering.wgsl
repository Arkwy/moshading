const PI: f32 = 3.14159265;

struct Uniforms {
    viewport_size: vec2<u32>,
    time: f32,
};

@group(0) @binding(0) var input_tex: texture_2d<f32>;
@group(0) @binding(1) var input_sampler: sampler;
@group(0) @binding(2) var<uniform> uniforms: Uniforms;


struct DitherUniforms {
    mode: i32,  // 0 = threshold, 1 = random, 2 = halftone, 3 = ordered (bayer), 4 = ordered (void-and-cluster)
    control: u32, // each bit represents a bool value, 1=color mode, 2=random use time, 4 = ??
    threshold: f32,
    threshold_rgb: vec3<f32>,
    random_min_rgb: vec3<f32>,
    random_max_rgb: vec3<f32>,
    random_min: f32,
    random_max: f32,
    halftone_scale: f32,
    halftone_angle: f32,
    bayer_steps: u32,
};

@group(1) @binding(0) var<uniform> parameters: DitherUniforms;

fn color_mode() -> bool {
    return bool(parameters.control & 1u);
}

fn fullscreen_uv(coord: vec2<f32>) -> vec2<f32>{
    return coord / vec2<f32>(uniforms.viewport_size);
}


fn threshold_dithering(color: vec4<f32>, coord: vec2<f32>) -> vec4<f32> {
    if (color_mode()) {
        return vec4<f32>(vec3<f32>(color.rgb > parameters.threshold_rgb), color.a);
    }
    return vec4<f32>(vec3<f32>(f32((color.r + color.g + color.b) / 3.0 > parameters.threshold)), color.a);
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
        let v = vec3<u32>(vec2<u32>(coord), seed + time);
        return ldexp(vec3<f32>(pcg3d(v)), vec3<i32>(-32));
    } else {
        let v = vec3<u32>(vec2<u32>(coord), seed);
        return ldexp(vec3<f32>(pcg3d(v)), vec3<i32>(-32));
    }
}

fn random_dithering(color: vec4<f32>, coord: vec2<f32>) -> vec4<f32> {
    if (color_mode()) {
        let offset = parameters.random_min_rgb;
        let scale = parameters.random_max_rgb - parameters.random_min_rgb;
        let noise = offset + rand(coord, 0u, bool(parameters.control & 2u)) * scale;
        return vec4<f32>(vec3<f32>(color.rgb > noise), color.a);
    }

    let offset = parameters.random_min;
    let scale = parameters.random_max - parameters.random_min;
    let noise = offset + rand(coord, 0u, bool(parameters.control & 2u)).x * scale;
    return vec4<f32>(vec3<f32>(f32((color.r + color.g + color.b) / 3.0 > noise)), color.a);
}



fn rotate_2d(a : f32, vec : vec2<f32>) -> vec2<f32> {
    return mat2x2<f32>(cos(a), sin(a), -sin(a), cos(a)) * vec;
}
fn halftone_dithering(color: vec4<f32>, coord: vec2<f32>) -> vec4<f32> {
    let grid = rotate_2d(parameters.halftone_angle, coord - (vec2<f32>(uniforms.viewport_size) / 2.)) / parameters.halftone_scale * PI; 
    let mask = ((sin(grid.x) + cos(grid.y)) / 4.0) + 0.5;
    if (color_mode()) {
        return vec4<f32>(vec3<f32>(color.rgb > vec3<f32>(mask)), color.a);
    }
    return vec4<f32>(vec3<f32>(f32((color.r + color.g + color.b) / 3.0 > mask)), color.a);
}


fn ordered_dithering(color: vec4<f32>, coord: vec2<f32>) -> vec4<f32> {
    var mask: f32 = 0;
    var grid = vec2<u32>(coord);
    let steps = parameters.bayer_steps;
    for (var i: u32 = 0; i < steps; i++) {
        var id_2d = grid % 2;
        let id = f32(id_2d.x + 2 * (id_2d.x ^ id_2d.y)) / f32(4u << (2u * i));
        mask += id;
        grid.x >>= 1;
        grid.y >>= 1;
    }
    mask += 1.0 / f32(1u << ((2u * steps) + 1u));
    if (color_mode()) {
        return vec4<f32>(vec3<f32>(color.rgb > vec3<f32>(mask)), color.a);
    }
    return vec4<f32>(vec3<f32>(f32((color.r + color.g + color.b) / 3.0 > mask)), color.a);
}



@fragment fn fs_main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
    let uv = fullscreen_uv(coord.xy);

    let color = textureSample(input_tex, input_sampler, uv);

    switch (parameters.mode) {
        case 0 {
            return threshold_dithering(color, coord.xy);
        }
        case 1 {
            return random_dithering(color, coord.xy);
        }
        case 2 {
            return halftone_dithering(color, coord.xy);
        }
        case 3 {
            return ordered_dithering(color, coord.xy);
        }
        default {
            return color;
        }
    }

}
