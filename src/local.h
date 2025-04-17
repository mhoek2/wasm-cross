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
//
#include <emscripten.h>
#include <GLES3/gl3.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "shader.h"