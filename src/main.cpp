
#include <stdio.h>

#include "local.h"
#include "shader.h"

GLFWwindow* g_window;

#include <vector>
#include <iostream>


struct vec2_t {
	float x;
	float y;
};

struct mat4_t {
	float m[16];
};

struct Framebuffer {
	GLuint fbo;
	GLuint colorTexture;
};

GLuint screenVAO, screenVBO;
GLuint color_shader;
GLuint gamma_shader;
Framebuffer fbo;
Framebuffer *current_fbo = nullptr;


#define SCREEN_W 1200
#define SCREEN_H 800
const vec2_t screen_size = { SCREEN_W , SCREEN_H };


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

Framebuffer create_fbo_with_depth(const vec2_t& size) {
	Framebuffer result;

	glGenFramebuffers(1, &result.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, result.fbo);

	// Create color texture
	glGenTextures(1, &result.colorTexture);
	glBindTexture(GL_TEXTURE_2D, result.colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y),
		0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result.colorTexture, 0);

	// Create depth renderbuffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
		static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	// Check framebuffer status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		switch (status) {
		case GL_FRAMEBUFFER_UNSUPPORTED:
			std::cerr << "Framebuffer is unsupported." << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			std::cerr << "Framebuffer incomplete: Attachment not complete." << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			std::cerr << "Framebuffer incomplete: Missing attachment." << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			std::cerr << "Framebuffer incomplete: Missing draw buffer." << std::endl;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			std::cerr << "Framebuffer incomplete: Missing read buffer." << std::endl;
			break;
		default:
			std::cerr << "Framebuffer incomplete: Unknown error." << std::endl;
			break;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return result;
}

static int init_framebuffer(void)
{
	fbo = create_fbo_with_depth(screen_size);
	create_screen_vao();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//float fov = 60.0f * 3.14159f / 180.0f; // convert to radians
	//float aspect = screen_size.x / screen_size.y;
	//float near_plane = 0.1f;
	//float far_plane = 10.0f;
	//
	//mat4_t projection_mat = perspective(fov, aspect, near_plane, far_plane);
	glm::mat4 projection_mat = glm::perspective(glm::radians(60.0f), (float)screen_size.x / screen_size.y, 0.1f, 10.0f);
	
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
	color_shader = load_shaders(
		"src/glsl/color.vert",
		"src/glsl/color.frag");
#else
	gamma_shader = load_shaders(
		"gamma.vert",
		"gamma.frag");
	color_shader = load_shaders(
		"color.vert",
		"color.frag");
#endif
	return 0;
}

static void unbind_fbo(void)
{
	if (current_fbo != nullptr)
		current_fbo = nullptr;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void bind_fbo(Framebuffer *fbo)
{
	unbind_fbo();
	current_fbo = fbo;

	glBindFramebuffer(GL_FRAMEBUFFER, current_fbo->fbo);

	//int display_w, display_h;
	//glfwMakeContextCurrent(g_window);
	//glfwGetFramebufferSize(g_window, &display_w, &display_h);
	
	glViewport(0, 0, (int)screen_size.x, (int)screen_size.y);
	glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
}

static void render_fbo( Framebuffer *fbo ) {

	use_shader(gamma_shader);

	glBindVertexArray(screenVAO);
	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, fbo->colorTexture );

	glUniform1i(glGetUniformLocation(gamma_shader, "screenTexture"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
}

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

void frame( void ) 
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
	emscripten_set_main_loop(frame, 0, 1);
#else
	while (!glfwWindowShouldClose(g_window))
	{
		frame();
	}
#endif

	return 0;
}
