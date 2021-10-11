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

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <png.h>

#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct Vertex {
    glm::vec4 pos;
    glm::vec3 norm;
};

struct Entity {
    glm::vec2 position;
    float angle = 0;
};

struct TrackSegmentCoordinate {
    std::string track_segment;
    glm::vec4 offset;
};

bool holding_left = false;
bool holding_right = false;
bool holding_accel = false;

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
    else if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS) {
            holding_left = true;
        } else if (action == GLFW_RELEASE) {
            holding_left = false;
        }
    } else if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS) {
            holding_right = true;
        } else if (action == GLFW_RELEASE) {
            holding_right = false;
        }
    } else if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS) {
            holding_accel = true;
        } else if (action == GLFW_RELEASE) {
            holding_accel = false;
        }
    }
}

struct Collector : ObjFile::TriangleCollector {
    std::vector<Vertex> vertices;

    void handle_vertex(const ObjFile::Vertex& v, const ObjFile::TextureCoordinates&,
                       const ObjFile::VertexNormal& n) override
    {
        vertices.push_back({{v.x, v.y, v.z, v.w}, {n.x, n.y, n.z}});
    }
};

static std::vector<Vertex> load_model(const char* filename, const char* obj_name)
{
    ObjFile obj;
    obj.process_text(load_text_from(filename));

    Collector collector;
    obj.produce_triangle_list(obj_name, &collector);

    return collector.vertices;
}

glm::mat4 model_matrix_from_entity(const Entity& entity)
{
    glm::mat4 model{1.0f};
    model = glm::translate(model, glm::vec3(entity.position.x, 0, entity.position.y));
    model = glm::rotate(model, entity.angle, glm::vec3(0, 1.0f, 0));
    return model;
}

std::map<std::string, std::vector<Vertex>> load_track_segments(const char* filename)
{
    ObjFile obj;
    obj.process_text(load_text_from(filename));

    std::map<std::string, std::vector<Vertex>> result;
    for (const auto& obj_name : obj.objects()) {
        Collector collector;
        obj.produce_triangle_list(obj_name, &collector);
        result[obj_name] = collector.vertices;
    }
    return result;
}

const char* translate_track_ascii(char c)
{
    switch (c) {
    case '-':
        return "Horizontal";
    case ';':
        return "Top_Right";
    case '|':
        return "Vertical";
    case 'j':
        return "Bottom_Right";
    case 'l':
        return "Bottom_Left";
    case 'r':
        return "Top_Left";
    default:
        return "\0";
    }
}

std::vector<TrackSegmentCoordinate> translate_track_layout(const char* track_layout)
{
    std::vector<TrackSegmentCoordinate> result;

    constexpr auto tile_size = 6.0f;
    float y_offset = 0;
    for (const auto& row_text : ObjFile::split(track_layout, '\n')) {
        float x_offset = 0;
        for (const char c : row_text) {
            const char* key = translate_track_ascii(c);
            if (key[0] != '\0') {
                result.push_back({key, {x_offset, 0, y_offset, 0}});
            }
            x_offset += tile_size;
        }
        y_offset += tile_size;
    }
    return result;
}

void place_track_segment_with_offset(const std::vector<Vertex>& src, const glm::vec4& offset,
                                     std::vector<Vertex>& dest)
{
    for (auto vertex : src) {
        vertex.pos += offset;
        dest.emplace_back(vertex);
    }
}

void try_png(const char* filename)
{
    png_image image;

    /* Only the image structure version number needs to be set. */
    std::memset(&image, 0, sizeof image);
    image.version = PNG_IMAGE_VERSION;

    if (png_image_begin_read_from_file(&image, filename)) {
        image.format = PNG_FORMAT_RGBA;
        auto buffer = reinterpret_cast<png_bytep>(malloc(PNG_IMAGE_SIZE(image)));

        if (png_image_finish_read(&image, NULL /*background*/, buffer, 0 /*row_stride*/,
                                  NULL /*colormap for PNG_FORMAT_FLAG_COLORMAP */)) {
            auto ptr = buffer;
            for (size_t i = 0; i < 64; ++i) {
                std::cout << static_cast<int>(ptr[0]) << ", ";
                std::cout << static_cast<int>(ptr[1]) << ", ";
                std::cout << static_cast<int>(ptr[2]) << ", ";
                std::cout << static_cast<int>(ptr[3]);
                ptr += 4;
                std::cout << std::endl;
            }
        }
    }
}

int main(void)
{
    const char* track_layout = {"r-;  \n"
                                "| l-;\n"
                                "|   |\n"
                                "l---j\n"};

    try_png("/Users/pironvila/Downloads/ImphenziaPalette01.png");
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

    auto truck_verts = load_model("rc-truck.obj", "Cube");
    auto tree_verts = load_model("tree.obj", "Tree");
    decltype(truck_verts) track_verts;

    auto track_segments = load_track_segments("track_segments.obj");
    const auto track_segment_offsets = translate_track_layout(track_layout);

    for (const auto& track_offset : track_segment_offsets) {
        place_track_segment_with_offset(track_segments[track_offset.track_segment],
                                        track_offset.offset, track_verts);
    }

    decltype(truck_verts) vertices;
    vertices.reserve(truck_verts.size() + tree_verts.size());
    std::copy(truck_verts.begin(), truck_verts.end(), std::back_inserter(vertices));
    std::copy(tree_verts.begin(), tree_verts.end(), std::back_inserter(vertices));
    std::copy(track_verts.begin(), track_verts.end(), std::back_inserter(vertices));

    constexpr auto tree_count = 40;
    constexpr auto max_distance = 60.f;
    constexpr auto min_distance = 40.f;
    std::vector<Entity> entities(tree_count + 1);
    Entity& truck = entities[0];

    {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<float> radian_dist(0, static_cast<float>(M_PI) * 2.f);
        std::uniform_real_distribution<float> distance_dist(0, max_distance - min_distance);

        for (auto& entity : entities) {
            if (&entity == &truck)
                continue;
            auto distance = distance_dist(mt) + min_distance;
            auto theta = radian_dist(mt);
            entity.position.x = sinf(theta) * distance;
            entity.position.y = cosf(theta) * distance;
            entity.angle = theta;
        }
    }

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

        glClearColor(0.33f, 0.72f, 0.36f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view{1.0f};
        view = glm::translate(view, glm::vec3(0, 0, -40.0f));
        view = glm::rotate(view, glm::radians(35.264f), glm::vec3(1.0f, 0, 0));
        view = glm::rotate(view, glm::radians(45.0f), glm::vec3(0, 1.0f, 0));

        view = glm::translate(view, glm::vec3(-truck.position.x, 0, -truck.position.y));

        glm::mat4 projection = glm::perspective(glm::radians(35.f), ratio, 0.1f, 100.0f);

        glm::mat4 track_model = glm::scale(glm::vec3(10.0f));
        track_model = glm::rotate(track_model, glm::radians(-90.0f), glm::vec3(0, 1.0f, 0));
        glm::mat4 track_mvp = projection * view * track_model;
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(track_mvp));
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, static_cast<int>(truck_verts.size() + tree_verts.size()),
                     static_cast<int>(track_verts.size()));

        for (const auto& entity : entities) {
            glm::mat4 model = model_matrix_from_entity(entity);
            glm::mat4 mvp = projection * view * model;

            glUseProgram(program);
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
            glBindVertexArray(vertex_array);

            if (&entity == &truck) {
                glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(truck_verts.size()));
            } else {
                glDrawArrays(GL_TRIANGLES, static_cast<int>(truck_verts.size()),
                             static_cast<int>(tree_verts.size()));
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (holding_left) {
            truck.angle += 0.05f;
        }
        if (holding_right) {
            truck.angle -= 0.05f;
        }
        if (holding_accel) {
            glm::vec2 velocity{sinf(truck.angle), cosf(truck.angle)};
            truck.position += velocity * -0.3f;
        }
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
