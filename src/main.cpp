
#include <stdio.h>

#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__
// compile using
// emcc src/main.cpp -o index.html -s FULL_ES3=1 -s USE_GLFW=3
//
#include <emscripten.h>
#include <GLES3/gl3.h>
#else
#include <GL/gl.h>
#endif

GLFWwindow* g_window;

static int init_window( void ) {
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return 1;
	}

	int canvasWidth = 400;
	int canvasHeight = 400;
	g_window = glfwCreateWindow(canvasWidth, canvasHeight, "WASM Demo", NULL, NULL);
	if (g_window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent( g_window );

	return 0;
}

void frame( void ) {
	glfwPollEvents();

	int display_w, display_h;
	glfwMakeContextCurrent(g_window);
	glfwGetFramebufferSize(g_window, &display_w, &display_h);

	glViewport(0, 0, display_w, display_h);
	glClearColor(0.75f, 0, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glfwSwapBuffers(g_window);
	glfwMakeContextCurrent(g_window);
}

int main(int argc, char* argv[])
{
	if (init_window() != 0) return 1;

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
