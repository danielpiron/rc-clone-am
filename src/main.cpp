//========================================================================
// OpenGL triangle example
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//! [code]

#include "load_obj.h"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <string>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct Vertex {
    glm::vec4 pos;
    glm::vec3 norm;
};

static std::string load_text_from(const char* filename)
{
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    return str;
}

static void error_callback(int /*error*/, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int /* scancode */, int action,
                         int /* mods */)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static std::vector<Vertex> load_model(const char* filename)
{
    ObjFile obj;
    obj.process_text(load_text_from(filename));

    struct Collector : ObjFile::TriangleCollector {
        std::vector<Vertex> vertices;

        void handle_vertex(const ObjFile::Vertex& v, const ObjFile::TextureCoordinates&,
                           const ObjFile::VertexNormal& n) override
        {
            vertices.push_back({{v.x, v.y, v.z, v.w}, {n.x, n.y, n.z}});
        }
    };

    Collector collector;
    obj.produce_triangle_list("Cube", &collector);

    return collector.vertices;
}

int main(void)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    auto vertices = load_model("rc-truck.obj");

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(vertices[0]) * vertices.size()),
                 &vertices[0], GL_STATIC_DRAW);

    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

    const auto vertex_shader_string = load_text_from("vertex.glsl");
    const char* vertex_shader_text = vertex_shader_string.c_str();

    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    const auto fragment_shader_string = load_text_from("fragment.glsl");
    const char* fragment_shader_text = fragment_shader_string.c_str();

    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    const GLint mvp_location = glGetUniformLocation(program, "MVP");
    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vnorm_location = glGetAttribLocation(program, "vNorm");

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glEnableVertexAttribArray(static_cast<GLuint>(vpos_location));
    glEnableVertexAttribArray(static_cast<GLuint>(vnorm_location));
    glVertexAttribPointer(static_cast<GLuint>(vpos_location), 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, pos));
    glVertexAttribPointer(static_cast<GLuint>(vnorm_location), 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*)offsetof(Vertex, norm));

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view{1.0f};
        view = glm::translate(view, glm::vec3(0, 0, -20.0f));
        view = glm::rotate(view, glm::radians(35.264f), glm::vec3(1.0f, 0, 0));
        view = glm::rotate(view, glm::radians(45.0f), glm::vec3(0, 1.0f, 0));
        glm::mat4 projection = glm::perspective(glm::radians(40.f), ratio, 0.1f, 100.0f);
        glm::mat4 mvp = projection * view;

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size()));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
