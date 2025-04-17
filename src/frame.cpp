#include "local.h"

void begin_frame(void)
{
	//self.frameTime = self.clock.tick(60)
	//self.deltaTime = self.frameTime / self.DELTA_SHIFT

	bind_fbo(&fbo);
}

void end_frame(void)
{
	glUseProgram(0);
	glFlush();

	unbind_fbo();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_fbo(&fbo);

	glfwSwapBuffers(g_window);
	//glfwMakeContextCurrent(g_window);
}
