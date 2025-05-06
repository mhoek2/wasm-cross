#include "local.h"

GLuint vao[3], triangleVBO, cubeVBO, shadowDebugVBO;

//
// triangle
//
void setup_triangle(void)
{
	GLfloat vertices[] = {
		0.0f,  0.5f, 0.0f,
	   -0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f
	};

	glGenVertexArrays(1, &vao[0]);
	glBindVertexArray(vao[0]);

	glGenBuffers(1, &triangleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
}

void draw_triangle(void)
{

	glUseProgram(app.shaders.color_shader);

	glBindVertexArray(vao[0]);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

//
// cube
//
void setup_cube(void)
{
    GLfloat cubeVertices[] = {        
        -0.5f, -0.5f, -0.5f, // Back face
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f, // Front face
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f, // Left face
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f, // Right face
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f, // Bottom face
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f, // Top face
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };

    glGenVertexArrays(1, &vao[1]);
    glBindVertexArray(vao[1]);

    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
}

void draw_cube(void)
{
    glUseProgram(app.shaders.color_shader);

    glBindVertexArray(vao[1]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

//
// shadow depth
//
void setup_shadow_map_debug(void) {
    float x = -0.8f, y = -0.8f, w = 0.4f;

#if 1
    float quad[] = {
        x,     y,     0.0f, 0.0f,
        x+w,   y,     1.0f, 0.0f,
        x,     y+w,   0.0f, 1.0f,
        x+w,   y+w,   1.0f, 1.0f
    };
#else
    float quad[] = {
        // positions   // uvs
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
    };
#endif

    glGenVertexArrays(1, &vao[2]);
    glGenBuffers(1, &shadowDebugVBO);

    glBindVertexArray(vao[2]);
    glBindBuffer(GL_ARRAY_BUFFER, shadowDebugVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void draw_shadow_map_debug(void)
{
    glUseProgram(app.shaders.shadow_debug); // Load & compile shadow_debug shaders

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app.framebuffers.shadow.attachment);
    glUniform1i(glGetUniformLocation(app.shaders.shadow_debug, "u_shadowMap"), 0);

    glBindVertexArray(vao[2]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}