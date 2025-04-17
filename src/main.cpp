
#include <stdio.h>

#include "local.h"

#include <vector>
#include <iostream>

GLFWwindow* g_window;

GLuint screenVAO, screenVBO;
GLuint color_shader;
GLuint gamma_shader;

framebuffer_t fbo;
framebuffer_t* current_fbo = nullptr;

// helpers
mat4_t perspective(float fov_radians, float aspect, float near, float far) {
	mat4_t result = {};

	float f = 1.0f / tanf(fov_radians / 2.0f);
	result.m[0] = f / aspect;
	result.m[5] = f;
	result.m[10] = (far + near) / (near - far);
	result.m[11] = -1.0f;
	result.m[14] = (2.0f * far * near) / (near - far);

	return result;
}

static int init_window( void ) {
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return 1;
	}

	int canvasWidth = SCREEN_W;
	int canvasHeight = SCREEN_W;

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

static void draw_grid(void)
{
	/*uint32_t i;

	use_shader(color_shader);

	// bind projection matrix
	glUniformMatrix4fv(self.renderer.shader.uniforms['uPMatrix'], 1, GL_FALSE, self.renderer.projection)

	// viewmatrix
	glUniformMatrix4fv(self.renderer.shader.uniforms['uVMatrix'], 1, GL_FALSE, self.renderer.view)

	// color
	grid_color = self.settings.grid_color
	glUniform4f(self.renderer.shader.uniforms['uColor'], grid_color[0], grid_color[1], grid_color[2], 1.0)

	size = self.settings.grid_size
	spacing = self.settings.grid_spacing

	// Draw the grid lines on the XZ plane
	for (float i = -gridSize; i <= gridSize; i += gridSpacing) 
	{
		// Draw lines parallel to Z axis
		glBegin(GL_LINES)
			glVertex3f(i, 0, -size)
			glVertex3f(i, 0, size)
			glEnd()

			// Draw lines parallel to X axis
			glBegin(GL_LINES)
			glVertex3f(-size, 0, i)
			glVertex3f(size, 0, i)
		glEnd()
	}*/
}

void draw(void)
{
	glfwPollEvents();
	begin_frame();

	end_frame();
}

int main(int argc, char* argv[])
{
	if (init_window() != 0) return 1;
	if (init_shaders() != 0) return 1;
	if (init_framebuffer() != 0) return 1;

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
