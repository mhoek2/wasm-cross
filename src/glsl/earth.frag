#version 300 es
precision highp float;

in vec2 v_uv;
in vec3 v_tangent;
in vec3 v_bitangent;
in vec3 v_normal;
in vec3 v_worldPos;
in vec4 v_lightSpacePos;

uniform vec3 u_lightDir;     // Directional light (e.g. sun)
uniform vec3 u_camPos;
uniform float u_normalScale;

uniform sampler2D u_colormap; // Earth diffuse texture
uniform sampler2D u_shadowMap;
uniform sampler2D u_heightmap;
uniform sampler2D u_normalMap;
uniform sampler2D u_aoMap;    // Ambient Occlusion map
uniform sampler2D u_metallicMap;
uniform sampler2D u_roughnessMap;
uniform sampler2D u_nightMap;
uniform sampler2D u_specularMap;

out vec4 out_color;

// Constants
const float PI = 3.14159265359;

vec3 Diffuse_Lambert(in vec3 DiffuseColor)
{
	return DiffuseColor * (1.0 / PI);
}

vec3 CalcDiffuse( in vec3 diffuse, in float NE, in float NL,
	in float LH, in float roughness )
{
	return Diffuse_Lambert(diffuse);
}

vec3 F_Schlick( in vec3 SpecularColor, in float VH )
{
	float Fc = pow(1.0 - VH, 5.0);
	return clamp(50.0 * SpecularColor.g, 0.0, 1.0) * Fc + (1.0 - Fc) * SpecularColor; //hacky way to decide if reflectivity is too low (< 2%)
}

float D_GGX( in float NH, in float a )
{
	float a2 = a * a;
	float d = (NH * a2 - NH) * NH + 1.0;
	return a2 / (PI * d * d);
}

// Appoximation of joint Smith term for GGX
// [Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
float V_SmithJointApprox( in float a, in float NV, in float NL )
{
	float Vis_SmithV = NL * (NV * (1.0 - a) + a);
	float Vis_SmithL = NV * (NL * (1.0 - a) + a);
	return 0.5 * (1.0 / (Vis_SmithV + Vis_SmithL));
}

vec3 CalcSpecular( in vec3 specular, in float NH, in float NL,
	in float NE, float LH, in float VH, in float roughness )
{
	vec3  F = F_Schlick(specular, VH);
	float D = D_GGX(NH, roughness);
	float V = V_SmithJointApprox(roughness, NE, NL);

	return D * F * V;
}

vec3 CalcNormal( in vec3 vertexNormal, in vec2 frag_tex_coord )
{
		vec3 n = texture(u_normalMap, frag_tex_coord).rgb - vec3(0.5);
		n.xy *= u_normalScale;
		n.z = sqrt(clamp((0.25 - n.x * n.x) - n.y * n.y, 0.0, 1.0));
		n = n.x * v_tangent + n.y * v_bitangent + n.z * vertexNormal;
		return normalize(n);

#if 0
		vec3 normalMap = texture(u_normalMap, frag_tex_coord).rgb;
		normalMap = normalize(normalMap * 2.0 - 1.0);  // Convert to range [-1, 1]

		// Build the TBN (Tangent, Bitangent, Normal) matrix
		mat3 TBN = mat3(v_tangent, v_bitangent, vertexNormal);

		// Convert the normal from tangent space to world space using the TBN matrix
		return normalize(TBN * normalMap);
#endif
}

float get_shadow(vec4 lightSpacePos, vec3 normal, vec3 lightDir)
{
    // Perspective divide
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    // [0,1] texture space
    projCoords = projCoords * 0.5 + 0.5;

    // Outside shadow map check
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 1.0;

    float currentDepth = projCoords.z;

    // Bias (angle-dependent helps reduce acne)
    float bias = max(0.0015 * (1.0 - dot(normal, lightDir)), 0.0005);

    // PCF shadow filtering
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(u_shadowMap, 0));
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(u_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 0.4 : 1.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

// Main fragment shader function
void main() {
    vec4 diffuse;
    float attenuation;
    vec3 viewDir, lightColor, ambientColor;
    vec3 L, N, E;

    vec4 base = texture(u_colormap, v_uv);

    // Calculate camera and light direction vectors
    viewDir = normalize(u_camPos - v_worldPos);
    E = normalize(viewDir);

    L = -u_lightDir.xyz;
    float sqrLightDist = dot(L, L);
    L /= sqrt(sqrLightDist);

    lightColor = vec3(0.9, 0.9, 0.7);  // Sunlight color
    ambientColor = vec3(0.0, 0.0, 0.0);  // Ambient color
    diffuse = base;
    attenuation = 1.0;

    // Calculate surface normal
    N = CalcNormal(v_normal.xyz, v_uv);

    lightColor *= PI;

    vec4 specular = vec4(1.0);
    float roughness = 0.99;
    float AO = 1.0;

    AO = texture(u_aoMap, v_uv).r;  // Ambient Occlusion from AO map

    float specularMap = texture(u_specularMap, v_uv).r;
    specular.rgb = vec3(specularMap);  // Treat it as F

    roughness = 0.05;

    // Modify ambient color based on AO (Ambient Occlusion)
    ambientColor *= AO;

    // Calculate lighting effects
    vec3 H = normalize(L + E);
    float NE = abs(dot(N, E)) + 1e-5;
    float NL = clamp(dot(N, L), 0.0, 1.0);
    float LH = clamp(dot(L, H), 0.0, 1.0);

    vec3 Fd = CalcDiffuse(diffuse.rgb, NE, NL, LH, roughness);
    vec3 Fs = vec3(0.0);

    float NH = clamp(dot(N, H), 0.0, 1.0);
    float VH = clamp(dot(E, H), 0.0, 1.0);
    Fs = CalcSpecular(specular.rgb, NH, NL, NE, LH, VH, roughness);

    vec3 reflectance = Fd + Fs;

    // Transition factor: Night fade-in based on dot product between N and L
    float nightBlend = smoothstep(0.0, 0.15, 1.0 - NL); // Smoothly fade in as the surface faces away from light

    // Sample night texture (grayscale data), and adjust its intensity
    float nightTex = texture(u_nightMap, v_uv).r;
    nightTex = pow(nightTex, 0.9);  // Soften intensity of night texture
    nightTex = clamp(nightTex, 0.0, 0.5);  // Cap it to prevent overexposure

    // Color of night light (soft yellowish tint)
    vec3 nightColor = nightTex * vec3(1.0, 0.8, 0.4);
    nightColor *= nightBlend;  // Apply night blend based on how much sunlight is on the surface

	float nightBlendFactor = smoothstep(0.1, 1.0, NL);

	if (NL <= 0.1) {
		nightColor = nightColor * (1.0 + nightBlendFactor);
	} else {
		nightColor = nightColor * (1.0 - nightBlendFactor);
	}

    // Shadowing & lighting
    float shadow = get_shadow(v_lightSpacePos, N, L);
    vec3 litColor = lightColor * reflectance * (attenuation * NL);
    litColor += ambientColor * diffuse.rgb;
    litColor *= vec3(shadow);

    // Combine day and night effects
    vec3 finalColor = litColor + nightColor;

    // Tone mapping for better color response
    finalColor = vec3(1.0) - exp(-finalColor * 3.0);


    // Output final color
    out_color.rgb = finalColor;
    out_color.a = diffuse.a;
}
