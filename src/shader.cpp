#include <iostream>

#include <imgui.h>

#include "shader.hpp"

// Shader compiler
GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
    }
    return shader;
}

// Shader program
GLuint CreateShaderProgram() {
    const char* vs = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aUV;
        out vec2 TexCoords;
        void main() {
            TexCoords = aUV;
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* fs = R"(
        #version 330 core
        in vec2 TexCoords;
        out vec4 FragColor;
        uniform float time;
        void main() {
            vec2 uv = TexCoords;
            float r = 0.5 + 0.5 * sin(time + uv.x * 10.0);
            float g = 0.5 + 0.5 * cos(time + uv.y * 10.0);
            float b = 0.5 + 0.5 * sin(time);
            FragColor = vec4(r, g, b, 1.0);
        }
    )";

    GLuint v = CompileShader(GL_VERTEX_SHADER, vs);
    GLuint f = CompileShader(GL_FRAGMENT_SHADER, fs);
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

// Fullscreen quad
GLuint vao, vbo;
void InitQuad() {
    float verts[] = {-1, 1, 0, 1, -1, -1, 0, 0, 1, -1, 1, 0, -1, 1, 0, 1, 1, -1, 1, 0, 1, 1, 1, 1};
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// FBO
GLuint fbo = 0, tex = 0;
void CreateFBO(int w, int h) {
    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &tex);
    }
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cerr << "FBO incomplete\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderToFBO(GLuint prog, float time, int w, int h) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(prog);
    glUniform1f(glGetUniformLocation(prog, "time"), time);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}




void create_window(GLuint prog, float time) {
        // Show shader in a window
        ImGui::Begin("Shader Output");
        ImVec2 region = ImGui::GetContentRegionAvail();
        int w = static_cast<int>(region.x);
        int h = static_cast<int>(region.y);
        static int prevW = 0, prevH = 0;
        if (w != prevW || h != prevH) {
            CreateFBO(w, h);
            prevW = w;
            prevH = h;
        }
        RenderToFBO(prog, time, w, h);
        ImGui::Image(tex, region, ImVec2(0,1), ImVec2(1,0));
        ImGui::End();
}
