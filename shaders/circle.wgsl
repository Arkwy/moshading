struct DefaultUniforms {
    viewport_size : vec2<u32>,
    time : f32,
};

@group(0) @binding(0) var input_tex : texture_2d<f32>;
@group(0) @binding(1) var input_sampler : sampler;
@group(0) @binding(2) var<uniform> default_uniforms : DefaultUniforms;

struct CircleUniforms {
    center : vec2<f32>,
    radius : f32,
    border_width : f32,
};

@group(1) @binding(0) var<uniform> circle_uniforms: CircleUniforms;
// const circle_uniforms = CircleUniforms( vec2<f32>(0, 0), 0.5, 0.05 );

fn fullscreen_uv(coord : vec2<f32>) -> vec2<f32> {
    return coord / vec2<f32>(default_uniforms.viewport_size);
}

fn centered_uv(coord : vec2<f32>) -> vec2<f32> {
    var uv = coord;
    let vs = vec2<f32>(default_uniforms.viewport_size);
    let m = min(vs.x, vs.y);
    uv *= 2.0 / m;
    uv -= vec2<f32>(vs.x / m, vs.y / m);
    return uv;
}


@fragment fn fs_main(@builtin(position) coord : vec4<f32>) -> @location(0) vec4<f32> {
    let uv = centered_uv(coord.xy);
    let color = textureSample(input_tex, input_sampler, fullscreen_uv(coord.xy));
    return color - vec4<f32>(step(abs(length(uv - circle_uniforms.center) - circle_uniforms.radius), circle_uniforms.border_width));
    // return vec4<f32>(vec3<f32>(length(uv)), 1.0);
}
