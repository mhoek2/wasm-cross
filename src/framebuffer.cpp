#include "local.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

void unbind_fbo(void)
{
	if (current_fbo != nullptr)
		current_fbo = nullptr;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void bind_fbo(framebuffer_t* fbo)
{
	unbind_fbo();
	current_fbo = fbo;

	glBindFramebuffer(GL_FRAMEBUFFER, current_fbo->fbo);

	//int display_w, display_h;
	glfwMakeContextCurrent(g_window);
	//glfwGetFramebufferSize(g_window, &display_w, &display_h);

	glViewport(0, 0, (int)screen_size.x, (int)screen_size.y);
	glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
}

void render_fbo(framebuffer_t* fbo) {

	use_shader(gamma_shader);

	glBindVertexArray(screenVAO);
	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, fbo->attachment);

	glUniform1i(glGetUniformLocation(gamma_shader, "screenTexture"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
}

void create_screen_vao() {
	std::vector<float> quad = {
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

static framebuffer_t create_fbo_with_depth( void ) {
	framebuffer_t result;
	memset( &result, 0, sizeof(framebuffer_t) );

	glGenFramebuffers(1, &result.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, result.fbo);

	// Create color texture
	glGenTextures(1, &result.attachment);
	glBindTexture(GL_TEXTURE_2D, result.attachment);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		static_cast<GLsizei>(screen_size.x), static_cast<GLsizei>(screen_size.y),
		0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result.attachment, 0);

	// Create depth renderbuffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
#ifdef __EMSCRIPTEN__
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
		static_cast<GLsizei>(screen_size.x), static_cast<GLsizei>(screen_size.y));
#else
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
		static_cast<GLsizei>(screen_size.x), static_cast<GLsizei>(screen_size.y));
#endif
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

int init_framebuffer(void)
{
	fbo = create_fbo_with_depth();

	create_screen_vao();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, (int)screen_size.x, (int)screen_size.y);

	renderer.aspect_ratio = screen_size.x / screen_size.y;
	renderer.projection = glm::perspective(glm::radians(45.0f), renderer.aspect_ratio, 0.1f, 1000.0f);
	renderer.view = glm::mat4(1.0f);  // identity as placeholder

	return 0;
}