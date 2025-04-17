
#include <stdio.h>

#include "local.h"
#include "shader.h"

GLFWwindow* g_window;

#include <vector>

GLuint screenVAO, screenVBO;
GLuint gamma_shader;

void create_screen_vao() {
	// Define quad vertices (positions + texture coordinates)
	std::vector<float> quad = {
		// positions        // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f,
	};

	// Generate VAO and VBO
	glGenVertexArrays(1, &screenVAO);
	glGenBuffers(1, &screenVBO);

	// Bind VAO
	glBindVertexArray(screenVAO);

	// Bind VBO and load data
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(float), quad.data(), GL_STATIC_DRAW);

	// Set vertex attribute pointers for position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Set vertex attribute pointers for texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Unbind VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


static int init_window( void ) {
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return 1;
	}

	int canvasWidth = 1200;
	int canvasHeight = 800;
	g_window = glfwCreateWindow(canvasWidth, canvasHeight, "WASM Demo", NULL, NULL);
	if (g_window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent( g_window );

#ifndef __EMSCRIPTEN__
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
		return -1;
	}
#endif

	return 0;
}

static int init_framebuffer(void)
{
	create_screen_vao();

	return 0;
}

static void use_shader( GLuint shader ) {
	glUseProgram(shader);
}

static int init_shaders(void)
{
#ifdef __EMSCRIPTEN__
	gamma_shader = load_shaders(
		"src/glsl/gamma.vert", 
		"src/glsl/gamma.frag");
#else
	gamma_shader = load_shaders(
		"gamma.vert",
		"gamma.frag");
#endif
	return 0;
}

static void render_fbo() {

	use_shader(gamma_shader);

	glBindVertexArray(screenVAO);
	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);

	//glBindTexture(GL_TEXTURE_2D, fbo);

	glUniform1i(glGetUniformLocation(gamma_shader, "screenTexture"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
}

void frame( void ) {
	glfwPollEvents();

	int display_w, display_h;
	glfwMakeContextCurrent(g_window);
	glfwGetFramebufferSize(g_window, &display_w, &display_h);

	glViewport(0, 0, display_w, display_h);
	glClearColor(0.75f, 0, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	render_fbo();

	glfwSwapBuffers(g_window);
	//glfwMakeContextCurrent(g_window);
}

int main(int argc, char* argv[])
{
	if (init_window() != 0) return 1;
	if (init_shaders() != 0) return 1;
	if (init_framebuffer() != 0) return 1;

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(frame, 0, 1);
#else
	while (!glfwWindowShouldClose(g_window))
	{
		frame();
	}
#endif

	return 0;
}
