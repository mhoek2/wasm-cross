#include "local.h"
#include <iostream>

void prepare_frame(void)
{
	if (!app.camera.mouse_moving) {
		glfwSetCursorPos(app.g_window, screen_center.x, screen_center.y);
		app.camera.ignore_next_mouse_event = true;
	}

	// input event handling
	glfwPollEvents();
	process_keyboard(app.g_window);

	app.time = glfwGetTime();
}

void begin_frame(void)
{
	bind_fbo(&app.framebuffers.main);

	app.renderer.view = get_view_matrix();
}

void end_frame(void)
{
	glUseProgram(0);
	glFlush();

	unbind_fbo();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_fbo(&app.framebuffers.main);

	glfwSwapBuffers(app.g_window);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "OpenGL Error: " << err << std::endl;
	}
	//glfwMakeContextCurrent(app.g_window);
}
