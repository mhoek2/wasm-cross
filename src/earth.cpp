#include "local.h"
#include <iostream>

#include <math.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct {
    float           *vertices;
    float           *uvs;
    float           *normals;
    unsigned int    *indices;
    int             vertex_count;
    int             index_count;
} sphere_mesh_t;

sphere_mesh_t earth_mesh;
sphere_mesh_t atmosphere_mesh;

glm::vec3 lightDir;
glm::vec3 lightPos;
glm::mat4 lightView;
glm::mat4 lightProj;
glm::mat4 lightSpaceMatrix;
glm::mat4 earth_rotation;
glm::mat4 atmosphere_model;

const float height_scale = 0.1f;
const float normal_scale = 1.91f;

#ifdef __EMSCRIPTEN__
#define ASSETS_PATH "assets/"
#else
#define ASSETS_PATH "assets/"
#endif

#define LOAD_TEXTURE( _name ) \
    load_texture( ASSETS_PATH _name);

GLuint load_texture(const char* path, bool flip_vertically = true)
{
    if (flip_vertically) stbi_set_flip_vertically_on_load(true);
    //if (flip_vertically) stbi_set_flip_horizontally_on_load(true);

    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLenum format = GL_RGB;
#ifdef __EMSCRIPTEN__
    if (channels == 1) format = GL_LUMINANCE;
#else
    if (channels == 1) format = GL_RED;
#endif
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}

sphere_mesh_t create_uv_sphere(int stacks, int slices, float radius) {
    int vertex_count = (stacks + 1) * (slices + 1);
    int index_count = stacks * slices * 6;

    float* vertices = (float*)malloc(sizeof(float) * 3 * vertex_count);
    float* uvs = (float*)malloc(sizeof(float) * 2 * vertex_count);
    float* normals = (float*)malloc(sizeof(float) * 3 * vertex_count);
    unsigned int* indices = (unsigned int*)malloc(sizeof(unsigned int) * index_count);

    int vi = 0, ui = 0, ni = 0;
    for (int i = 0; i <= stacks; ++i) {
        float v = (float)i / stacks;
        float phi = v * M_PI;

        for (int j = 0; j <= slices; ++j) {
            float u = (float)j / slices;
            float theta = u * 2.0f * M_PI;

            float x = sinf(phi) * cosf(theta);
            float y = cosf(phi);
            float z = sinf(phi) * sinf(theta);

            // Position
            vertices[vi++] = radius * x;
            vertices[vi++] = radius * y;
            vertices[vi++] = radius * z;

            // UV
            uvs[ui++] = 1.0f - u;
            //uvs[ui++] = 1.0f - v;  // Flip Y for GL texture coords
            uvs[ui++] = 1.0f - v;  // Flip Y for GL texture coords

            // Normal
            normals[ni++] = x;
            normals[ni++] = y;
            normals[ni++] = z;
        }
    }

    // Index buffer
    int ii = 0;
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = (i * (slices + 1)) + j;
            int second = first + slices + 1;

            indices[ii++] = first;
            indices[ii++] = second;
            indices[ii++] = first + 1;

            indices[ii++] = second;
            indices[ii++] = second + 1;
            indices[ii++] = first + 1;
        }
    }

    sphere_mesh_t mesh;
    memset( &mesh, 0, sizeof(sphere_mesh_t) );

    mesh.vertices = vertices;
    mesh.uvs = uvs;
    mesh.normals = normals;
    mesh.indices = indices;
    mesh.vertex_count = vertex_count;
    mesh.index_count = index_count;

    return mesh;
}


static void setup_atmosphere( void )
{
    atmosphere_mesh = create_uv_sphere(64, 64, 1.0f);

    glGenVertexArrays(1, &app.atmosphere.vao);
    glBindVertexArray(app.atmosphere.vao);

    glGenBuffers(3, app.atmosphere.vbo);
    glGenBuffers(1, &app.atmosphere.ibo);

    // Position
    glBindBuffer(GL_ARRAY_BUFFER, app.atmosphere.vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, atmosphere_mesh.vertex_count * 3 * sizeof(float), atmosphere_mesh.vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // UV
    glBindBuffer(GL_ARRAY_BUFFER, app.atmosphere.vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, atmosphere_mesh.vertex_count * 2 * sizeof(float), atmosphere_mesh.uvs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // Normal
    glBindBuffer(GL_ARRAY_BUFFER, app.atmosphere.vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, atmosphere_mesh.vertex_count * 3 * sizeof(float), atmosphere_mesh.normals, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app.atmosphere.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, atmosphere_mesh.index_count * sizeof(unsigned int), atmosphere_mesh.indices, GL_STATIC_DRAW);

    free(atmosphere_mesh.vertices);
    free(atmosphere_mesh.uvs);
    free(atmosphere_mesh.normals);
    free(atmosphere_mesh.indices);

    app.atmosphere.noise_tex = LOAD_TEXTURE("noise.png");

    glBindVertexArray(0);
}

void setup_earth( void )
{
    earth_mesh = create_uv_sphere(256, 256, 1.0f);

    glGenVertexArrays(1, &app.earth.vao);
    glBindVertexArray(app.earth.vao);

    glGenBuffers(3, app.earth.vbo);
    glGenBuffers(1, &app.earth.ibo);

    // Position
    glBindBuffer(GL_ARRAY_BUFFER, app.earth.vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, earth_mesh.vertex_count * 3 * sizeof(float), earth_mesh.vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // UV
    glBindBuffer(GL_ARRAY_BUFFER, app.earth.vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, earth_mesh.vertex_count * 2 * sizeof(float), earth_mesh.uvs, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // Normal
    glBindBuffer(GL_ARRAY_BUFFER, app.earth.vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, earth_mesh.vertex_count * 3 * sizeof(float), earth_mesh.normals, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app.earth.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, earth_mesh.index_count * sizeof(unsigned int), earth_mesh.indices, GL_STATIC_DRAW);

    free(earth_mesh.vertices);
    free(earth_mesh.uvs);
    free(earth_mesh.normals);
    free(earth_mesh.indices);

#ifdef __EMSCRIPTEN__
    app.earth.colormap_tex = LOAD_TEXTURE("earth_daymap_pot.jpg");
    app.earth.nightmap_tex = LOAD_TEXTURE("earth_nightmap_l_pot.jpg");
#else
    app.earth.colormap_tex = LOAD_TEXTURE("earth_daymap.jpg");
    app.earth.nightmap_tex = LOAD_TEXTURE("earth_nightmap_l.jpg");
#endif
    app.earth.heightmap_tex = LOAD_TEXTURE("earth_bump.jpg");
    app.earth.normalmap_tex = LOAD_TEXTURE("earth_normal.png");
    app.earth.aomap_tex = LOAD_TEXTURE("earth_ao.png");
    app.earth.specularmap_tex = LOAD_TEXTURE("earth_specular.jpg");

    glBindVertexArray(0);

    setup_atmosphere();
}

static void calc_light( void )
{
    lightPos = glm::vec3(1.0f, 1.0f, 2.0f);  // Set this to your desired world-space light position
    lightDir = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - lightPos);

    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
    lightProj = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.01f, 5.0f);
    lightSpaceMatrix = lightProj * lightView;
}

float earthRadius = 1.0f;  // Earth's radius (normalize this for simplicity)
float atmosphereRadius = 1.15f;  // Slightly larger than Earth

void prepare_uniforms( void )
{
    //float rotation_speed = 360.0f / 24.0f;  // Full rotation every 24 seconds (simulating a 24-hour cycle)
    float rotation_speed = 0.05f;
    
    earth_rotation = glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(earthRadius)), app.time * rotation_speed, glm::vec3(0.0f, 1.0f, 0.0f));
    atmosphere_model = glm::scale(glm::mat4(1.0f), glm::vec3(atmosphereRadius));

    calc_light(); 
}

void draw_earth( GLuint shader )
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);  // Enable depth writing

    use_shader( shader );

	glUniformMatrix4fv(glGetUniformLocation(shader, "u_proj"), 1, GL_FALSE, glm::value_ptr(app.renderer.projection));
	glUniformMatrix4fv(glGetUniformLocation(shader, "u_view"), 1, GL_FALSE, glm::value_ptr(app.renderer.view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "u_model"), 1, GL_FALSE, glm::value_ptr(earth_rotation));

    glUniformMatrix4fv(glGetUniformLocation(shader, "u_lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glUniform3f(glGetUniformLocation(shader, "u_lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(glGetUniformLocation(shader, "u_camPos"), app.camera.camera_pos.x, app.camera.camera_pos.y, app.camera.camera_pos.z);

    glUniform1f(glGetUniformLocation(shader, "u_heightScale"), height_scale);
    glUniform1f(glGetUniformLocation(shader, "u_normalScale"), normal_scale);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app.earth.colormap_tex);
    glUniform1i(glGetUniformLocation(shader, "u_colormap"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, app.framebuffers.shadow.attachment);
    glUniform1i(glGetUniformLocation(shader, "u_shadowMap"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, app.earth.heightmap_tex);
    glUniform1i(glGetUniformLocation(shader, "u_heightmap"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, app.earth.normalmap_tex);
    glUniform1i(glGetUniformLocation(shader, "u_normalMap"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, app.earth.aomap_tex);
    glUniform1i(glGetUniformLocation(shader, "u_aoMap"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, app.earth.nightmap_tex);
    glUniform1i(glGetUniformLocation(shader, "u_nightMap"), 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, app.earth.specularmap_tex);
    glUniform1i(glGetUniformLocation(shader, "u_specularMap"), 6);

    glBindVertexArray(app.earth.vao);
    glDrawElements(GL_TRIANGLES, earth_mesh.index_count, GL_UNSIGNED_INT, 0);
}

void draw_atmosphere( GLuint shader )
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE, GL_ONE);

    use_shader( shader );

	glUniformMatrix4fv(glGetUniformLocation(shader, "u_proj"), 1, GL_FALSE, glm::value_ptr(app.renderer.projection));
	glUniformMatrix4fv(glGetUniformLocation(shader, "u_view"), 1, GL_FALSE, glm::value_ptr(app.renderer.view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "u_model"), 1, GL_FALSE, glm::value_ptr(atmosphere_model));

    glUniform3f(glGetUniformLocation(shader, "u_lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(glGetUniformLocation(shader, "u_camPos"), app.camera.camera_pos.x, app.camera.camera_pos.y, app.camera.camera_pos.z);

    glUniform1f(glGetUniformLocation(shader, "u_earthRadius"), earthRadius);
    glUniform1f(glGetUniformLocation(shader, "u_atmosphereRadius"), atmosphereRadius);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app.atmosphere.noise_tex);
    glUniform1i(glGetUniformLocation(shader, "u_noiseTex"), 0);
    glUniform1f(glGetUniformLocation(shader, "u_time"), app.time);

    glBindVertexArray(app.atmosphere.vao);
    glDrawElements(GL_TRIANGLES, atmosphere_mesh.index_count, GL_UNSIGNED_INT, 0 );

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
  }

void render_shadow_pass( GLuint shader )
{
    glViewport(0, 0, SHADOW_RES, SHADOW_RES);
    glBindFramebuffer(GL_FRAMEBUFFER, app.framebuffers.shadow.fbo);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 3.0f); 

    use_shader( shader );

    glUniformMatrix4fv(glGetUniformLocation(shader, "u_lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader, "u_model"), 1, GL_FALSE, glm::value_ptr(earth_rotation));
    glUniform1f(glGetUniformLocation(shader, "u_heightScale"), height_scale);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app.earth.heightmap_tex);
    glUniform1i(glGetUniformLocation(shader, "u_heightmap"), 0);

    glBindVertexArray(app.earth.vao);
    glDrawElements(GL_TRIANGLES, earth_mesh.index_count, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_POLYGON_OFFSET_FILL);
}