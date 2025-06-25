struct DefaultUniforms {
    viewport_size: vec2<u32>,
    time: f32,
};

@group(0) @binding(2) var<uniform> default_uniforms: DefaultUniforms;

struct QuadUniforms {
    size: vec2<f32>,
    pos: vec2<f32>,
    rot: f32,
};

@group(1) @binding(2) var<uniform> quad_uniforms: QuadUniforms;

fn rotate_2d(a : f32, vec : vec2<f32>) -> vec2<f32> {
    return mat2x2<f32>(cos(a), sin(a), -sin(a), cos(a)) * vec;
} 

@vertex
fn vs_main(@builtin(vertex_index) vertex_index: u32) -> @builtin(position) vec4<f32> {
    var local_positions = array<vec2<f32>, 6>(
        vec2<f32>(0.0, 0.0),
        vec2<f32>(quad_uniforms.size.x, 0.0),
        vec2<f32>(0.0, quad_uniforms.size.y),
        vec2<f32>(quad_uniforms.size.x, 0.0),
        vec2<f32>(quad_uniforms.size.x, quad_uniforms.size.y),
        vec2<f32>(0.0, quad_uniforms.size.y),
    );

    var local = rotate_2d(quad_uniforms.rot, local_positions[vertex_index] - vec2<f32>(0.5)) + vec2<f32>(0.5);
    var world = quad_uniforms.pos + local;
    var pos = world / default_uniforms.viewport_size;
    // pos = pos * 2.0 - 1.0; // convert to clip space [-1, 1]
    // pos.y = -pos.y; // flip Y if needed (depending on your clip space convention)
    return vec4<f32>(pos, 0.0, 1.0);
}
