#version 300 es
precision highp float;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec3 a_normal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec2 v_uv;
out vec3 v_tangent;
out vec3 v_bitangent;
out vec3 v_normal;
out vec3 v_worldPos;

#define textureSize 256.0
#define texelSize 1.0 / 256.0
#define MIN_HEIGHT_DIFF 0.01  // Minimum height difference to consider for normal calculation

void main() 
{
    float deltaU = 1.0 / 256.0;
    float deltaV = 1.0 / 256.0;
    vec3 tangent = normalize(cross(a_normal, vec3(deltaU, deltaV, 0.0)));
    vec3 bitangent = normalize(cross(a_normal, tangent));

    v_uv = a_uv;
    v_tangent = tangent;
    v_bitangent = bitangent;
    v_normal = mat3(u_model) * a_normal;
    v_worldPos = vec3(u_model * vec4(a_position, 1.0));

    gl_Position = u_proj * u_view * vec4(v_worldPos, 1.0);
}