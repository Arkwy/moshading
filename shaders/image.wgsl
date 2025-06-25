struct DefaultUniforms {
    viewport_size: vec2<u32>,
    time: f32,
};

@group(0) @binding(0) var input_tex: texture_2d<f32>;
@group(0) @binding(1) var input_sampler: sampler;
@group(0) @binding(2) var<uniform> default_uniforms: DefaultUniforms;


struct ImageUniforms {
    size: vec2<f32>,
    pos: vec2<f32>,
    rot: f32,
    opacity: f32,
};

@group(1) @binding(0) var image_tex: texture_2d<f32>;
@group(1) @binding(1) var image_sampler: sampler;
@group(1) @binding(2) var<uniform> image_uniforms: ImageUniforms;

fn fullscreen_uv(coord : vec2<f32>) -> vec2<f32> {
    return coord / vec2<f32>(default_uniforms.viewport_size);
}

fn rotate_2d(a : f32, vec : vec2<f32>) -> vec2<f32> {
    return mat2x2<f32>(cos(a), sin(a), -sin(a), cos(a)) * vec;
} 

@fragment fn fs_main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
    var uv = coord.xy - image_uniforms.pos;
    uv /= image_uniforms.size;
    uv -= 0.5;
    uv = rotate_2d(image_uniforms.rot, uv);
    uv += 0.5;
    var color = textureSample(input_tex, input_sampler, fullscreen_uv(coord.xy));

    if (all(uv >= vec2<f32>(0.0)) && all(uv <= vec2<f32>(1.0))) {
        var image = textureSample(image_tex, image_sampler, uv);
        image.a *= image_uniforms.opacity;
        color = image * image.a + color * (1.0 - image.a);
    }


    return color;
}
