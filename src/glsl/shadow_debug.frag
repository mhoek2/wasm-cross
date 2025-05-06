#version 300 es
precision highp float;

in vec2 v_uv;
uniform sampler2D u_shadowMap;

out vec4 fragColor;

void main() {
    float depth = texture(u_shadowMap, v_uv).r;
    fragColor = vec4(vec3(depth), 1.0); // grayscale depth
}
