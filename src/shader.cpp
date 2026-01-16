#include "local.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

#ifdef __EMSCRIPTEN__
#define SHADER_PATH "src/glsl/"
#else
#define SHADER_PATH "glsl/"
#endif

#define LOAD_SHADER( _shader, _name ) \
    app.shaders._shader = load_shaders( SHADER_PATH #_name ".vert", SHADER_PATH  #_name ".frag" );

void use_shader(GLuint shader) {
	glUseProgram(shader);
}

static GLuint load_shader(const char* path, GLenum shader_type) {
    std::string shader_code;

    std::ifstream file_stream(path, std::ios::in);
    if (file_stream.is_open()) {
        shader_code = std::string(std::istreambuf_iterator<char>(file_stream),
            std::istreambuf_iterator<char>());
        file_stream.close();
    }
    else {
        std::cout << "could not open " << path << std::endl;
        return 0;
    }

    const char* c_str_shader_code = shader_code.c_str();

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &c_str_shader_code, NULL);
    glCompileShader(shader);
    return shader;
}

static GLuint load_shaders(const char* vshader, const char* fshader) {
    GLuint vshader_id = load_shader(vshader, GL_VERTEX_SHADER);
    GLuint fshader_id = load_shader(fshader, GL_FRAGMENT_SHADER);
    GLint success = GL_FALSE;

    glGetShaderiv(vshader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[255];
        glGetShaderInfoLog(fshader_id, 255, NULL, log);
        std::cout << "error at vertex shader compilation" << log << std::endl;
        glDeleteShader(vshader_id);
        return 0;
    }

    glGetShaderiv(fshader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[255];
        glGetShaderInfoLog(fshader_id, 255, NULL, log);
        std::cout << "error at fragment shader compilation : " << log << std::endl;
        glDeleteShader(vshader_id);
        glDeleteShader(fshader_id);
        return 0;
    }

    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vshader_id);
    glAttachShader(program_id, fshader_id);
    glLinkProgram(program_id);

    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success) {
        char log[255];
        glGetShaderInfoLog(program_id, 255, NULL, log);
        std::cout << "error in linking the shaders : " << log << std::endl;
        return 0;
    }

    glDeleteShader(vshader_id);
    glDeleteShader(fshader_id);

    return program_id;
}

int init_shaders(void)
{
    LOAD_SHADER( gamma_shader, gamma )
    LOAD_SHADER( color_shader, color )
    LOAD_SHADER( earth_shader, earth )
    LOAD_SHADER( shadow_depth, shadow_depth )
    LOAD_SHADER( shadow_debug, shadow_debug )
    LOAD_SHADER( atmosphere_shader, atmosphere )

	return 0;
}