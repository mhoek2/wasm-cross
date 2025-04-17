#pragma once

#ifndef __EMSCRIPTEN__
#include <includes/glew/include/GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__
// compile using
// .\emsdk_env.bat  
// emcc src/main.cpp -o index.html -s FULL_ES3=1 -s USE_GLFW=3
// emcc src/main.cpp src/shader.cpp -o index.html -s FULL_ES3=1 -s USE_GLFW=3 --preload-file src/glsl
// emcc src/main.cpp src/shader.cpp -o index.html -s FULL_ES3=1 -s USE_GLFW=3 --preload-file src/glsl -I./external-wasm
//
#include <emscripten.h>
#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

#include "shader.h"