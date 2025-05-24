#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>


// Shader compiler
GLuint CompileShader(GLenum type, const char* source);

// Shader program
GLuint CreateShaderProgram();

// Fullscreen quad
void InitQuad();

void CreateFBO(int w, int h);

void RenderToFBO(GLuint prog, float time, int w, int h);

// create imgui window containg shader
void create_window(GLuint prog, float time, const float& width, const float& height);

#endif
