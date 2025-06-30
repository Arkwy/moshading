struct Uniforms {
    viewport_size: vec2<u32>,
    time: f32,
};

@group(0) @binding(0) var input_tex: texture_2d<f32>;
@group(0) @binding(1) var input_sampler: sampler;
@group(0) @binding(2) var<uniform> uniforms: Uniforms;


struct DitherUniforms {
    mode: i32,  // 0 = threshold, 1 = random, 2 = halftone, 3 = ordered (bayer), 4 = ordered (void-and-cluster)
    control: i32, // each bit represent a bool value, 2^0=color mode, 2^1=random use time
    threshold: f32,
    threshold_rgb: vec3<f32>,
    random_threshod: f32,
    random_min: f32,
    random_max: f32,
    random_threshod_rgb: vec3<f32>,
    random_min_rgb: vec3<f32>,
    random_max_rgb: vec3<f32>,
};

@group(1) @binding(0) var<uniform> dither_uniforms: DitherUniforms;


fn fullscreen_uv(coord : vec2<f32>) -> vec2<f32> {
    return coord / vec2<f32>(uniforms.viewport_size);
}


fn threshold_dithering(color : vec4<f32>, coord : vec2<f32>, parameters : DitherUniforms) -> vec4<f32> {
    if (bool(parameters.contol & 1)) {
        return vec4<f32>(vec3<f32>(color.rgb > parameters.threshold_rgb), color.a);
    }
    return vec4<f32>(vec3<f32>((color.r + color.g + color.b) / 3.0 > parameters.threshold), color.a);
}


fn random_dithering(color : vec4<f32>, coord : vec2<f32>, parameters : DitherUniforms) -> vec4<f32> {
    if (bool(parameters.contol & 1)) {
        return vec4<f32>(vec3<f32>(color.rgb > parameters.random_threshold_rgb), color.a);
    }
    return vec4<f32>(vec3<f32>((color.r + color.g + color.b) / 3.0 > parameters.threshold), color.a);
}


@fragment fn fs_main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
    let uv = fullscreen_uv(coord.xy);

    let color = textureSample(input_tex, input_sampler, uv);

    switch (dither_uniforms.mode) {
        case 0 {
            return threshold_dithering(color, coord.xy, dither_uniforms);
        }
        default {
            return color;
        }
    }

}
