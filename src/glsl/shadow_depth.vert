#version 300 es
precision highp float;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec3 a_normal;

uniform mat4 u_lightSpaceMatrix;
uniform mat4 u_model;

uniform sampler2D u_heightmap;
uniform float u_heightScale;

out vec2 v_uv;

void main() {
    v_uv = a_uv;

    float height = texture(u_heightmap, a_uv).r;
    vec3 displaced = a_position + a_normal * height * u_heightScale;

    gl_Position = u_lightSpaceMatrix * u_model * vec4(displaced, 1.0);
}