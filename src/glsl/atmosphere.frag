#version 300 es
precision highp float;

in vec2 v_uv;
in vec3 v_normal;
in vec3 v_worldPos;

uniform vec3 u_lightDir;
uniform vec3 u_camPos;
uniform mat4 u_viewProj;
uniform float u_earthRadius;
uniform float u_atmosphereRadius;
uniform float u_time;

uniform sampler2D u_noiseTex;

out vec4 out_color;

void main() {
    out_color = vec4(0.0);
}