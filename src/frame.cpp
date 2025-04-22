#include "local.h"
#include <iostream>

void begin_frame(void)
{
	if (!camera.mouse_moving) {
		glfwSetCursorPos(g_window, screen_center.x, screen_center.y);
		camera.ignore_next_mouse_event = true;
	}

	// input event handling
	glfwPollEvents();
	process_keyboard(g_window);

	//self.frameTime = self.clock.tick(60)
	//self.deltaTime = self.frameTime / self.DELTA_SHIFT

	bind_fbo(&fbo);

	renderer.view = get_view_matrix();
}

void end_frame(void)
{
	glUseProgram(0);
	glFlush();

	unbind_fbo();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_fbo(&fbo);

	glfwSwapBuffers(g_window);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "OpenGL Error: " << err << std::endl;
	}
	//glfwMakeContextCurrent(g_window);
}
