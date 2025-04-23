
#include <stdio.h>

#include "local.h"

#include <vector>
#include <iostream>

GLFWwindow* g_window;

GLuint screenVAO, screenVBO;
GLuint color_shader;
GLuint gamma_shader;

camera_t camera;
renderer_t renderer;

framebuffer_t fbo;
framebuffer_t* current_fbo = nullptr;

static int init_window( void ) {
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return 1;
	}

	g_window = glfwCreateWindow(screen_size.x, screen_size.y, "WASM Demo", NULL, NULL);
	if (g_window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent( g_window );

	//glfwSetInputMode( g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
	glfwSetCursorPosCallback( g_window, mouse_callback );

#ifndef __EMSCRIPTEN__
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
		return -1;
	}
#endif

	return 0;
}

static GLuint gridVAO = 0;
static GLuint gridVBO = 0;

static void setup_grid() {
	if (gridVAO != 0)
		return;

	glGenVertexArrays(1, &gridVAO);
	glGenBuffers(1, &gridVBO);

	glBindVertexArray(gridVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);

	float size = 10.0f;
	float spacing = 1.0f;

	std::vector<GLfloat> vertices;

	// Draw grid lines on the XZ plane
	for (float i = -size; i <= size; i += spacing) {
		vertices.push_back(i);
		vertices.push_back(0.0f); 
		vertices.push_back(-size);

		vertices.push_back(i);
		vertices.push_back(0.0f); 
		vertices.push_back(size); 

		vertices.push_back(-size); 
		vertices.push_back(0.0f);  
		vertices.push_back(i);     

		vertices.push_back(size);  
		vertices.push_back(0.0f);  
		vertices.push_back(i);     
	}

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

	GLint posAttrib = glGetAttribLocation( color_shader, "aVertex");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

static void draw_grid(void)
{
	uint32_t i;

	use_shader(color_shader);

	// Bind the projection matrix
	glUniformMatrix4fv(glGetUniformLocation(color_shader, "uPMatrix"), 1, GL_FALSE, glm::value_ptr(renderer.projection));

	// Bind the view matrix
	glUniformMatrix4fv(glGetUniformLocation(color_shader, "uVMatrix"), 1, GL_FALSE, glm::value_ptr(renderer.view));
	
	/*
	GLint loc = glGetUniformLocation(color_shader, "uPMatrix");
	if (loc == -1) {
		std::cerr << "Warning: uPMatrix uniform not found or optimized out." << std::endl;
	}

	loc = glGetUniformLocation(color_shader, "uVMatrix");
	if (loc == -1) {
		std::cerr << "Warning: uVMatrix uniform not found or optimized out." << std::endl;
	}

	loc = glGetUniformLocation(color_shader, "uColor");
	if (loc == -1) {
		std::cerr << "Warning: uColor uniform not found or optimized out." << std::endl;
	}
	*/

	glUniform4f(glGetUniformLocation(color_shader, "uColor"), 1.0f, 1.0f, 1.0f, 1.0f);

	setup_grid();

	glBindVertexArray(gridVAO);
	glDrawArrays(GL_LINES, 0, 2 * (20 * 2)); // 20 lines, each consisting of 2 vertices (start and end)
	glBindVertexArray(0);
}

void draw(void)
{
	begin_frame();

	draw_grid();
	draw_triangle();
	draw_cube();

	end_frame();

#ifdef __EMSCRIPTEN__
	///const GLubyte* version = glGetString(GL_VERSION);
	//std::cout << "GL_VERSION: " << version << std::endl;
#endif
}

int main(int argc, char* argv[])
{
	if (init_window() != 0) return 1;
	if (init_shaders() != 0) return 1;
	if (init_camera() != 0) return 1;
	if (init_framebuffer() != 0) return 1;

	setup_grid();
	setup_cube();

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(draw, 0, 1);
#else
	while (!glfwWindowShouldClose(g_window))
	{
		draw();
	}
#endif

	return 0;
}