#pragma once

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct ObjFile {
    struct LinePartition {
        std::string command;
        std::string parameters;
    };

    struct Vertex {
        float x, y, z, w = 1.f;
        bool operator==(const Vertex& other) const
        {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }
    };

    struct TextureCoordinates {
        float u, v;
        bool operator==(const TextureCoordinates& other) const
        {
            return u == other.u && v == other.v;
        }
    };

    struct VertexNormal {
        float x, y, z;
        bool operator==(const VertexNormal& other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    struct TriangleCollector {
        virtual void handle_vertex(const Vertex&, const TextureCoordinates&,
                                   const VertexNormal&) = 0;
        virtual ~TriangleCollector() {}
    };

    struct Face {
        struct Indices {
            int vertex;
            int texture;
            int normal;

            Indices(int vertex_index, int texture_index, int normal_index)
                : vertex(vertex_index), texture(texture_index), normal(normal_index)
            {
            }

            Indices(const std::vector<int>& v)
            {
                if (v.size() >= 1) {
                    vertex = v[0];
                }
                if (v.size() >= 2) {
                    texture = v[1];
                }
                if (v.size() >= 3) {
                    normal = v[2];
                }
            }

            bool operator==(const Indices& other) const
            {
                return vertex == other.vertex && texture == other.texture && normal == other.normal;
            }
        };
        std::vector<Indices> indices;
        Face() {}
        Face(const Face&) = default;
        Face(std::initializer_list<Indices> il) : indices(il) {}

        bool operator==(const Face& other) const { return indices == other.indices; }
    };

    struct Object {
        std::vector<Vertex> vertices;
        std::vector<TextureCoordinates> tex_coords;
        std::vector<VertexNormal> vertex_normals;
        std::vector<Face> faces;
    };

    static LinePartition partition_line(const std::string& line)
    {

        const auto space_index = line.find_first_of(' ');
        return {line.substr(0, space_index), line.substr(space_index + 1)};
    }

    static std::string strip_comments(const std::string& line)
    {
        const auto hash_index = line.find_first_of('#');
        return line.substr(0, hash_index);
    }

    static std::string strip_whitespace(const std::string& s)
    {
        constexpr auto whitespace = ' ';
        const auto first_non_whitepace = s.find_first_not_of(whitespace);
        const auto last_non_whitespace = s.find_last_not_of(whitespace);
        if (first_non_whitepace == std::string::npos || last_non_whitespace == std::string::npos) {
            return "";
        }
        return s.substr(first_non_whitepace, last_non_whitespace - first_non_whitepace + 1);
    }

    static std::vector<std::string> split(const std::string& s, char delimiter)
    {
        std::stringstream ss(s);
        std::string element;

        std::vector<std::string> result;
        while (std::getline(ss, element, delimiter)) {
            result.emplace_back(element);
        }
        return result;
    }

    static Vertex parse_vertex(const std::string& s)
    {
        std::stringstream ss(s);

        float x = 0;
        float y = 0;
        float z = 0;
        float w = 1.0f;

        ss >> x;
        ss >> y;
        ss >> z;

        return {x, y, z, w};
    }

    static TextureCoordinates parse_texture_coordinates(const std::string& s)
    {
        std::stringstream ss(s);

        float u = 0;
        float v = 0;

        ss >> u;
        ss >> v;

        return {u, v};
    }

    static VertexNormal parse_vertex_normal(const std::string& s)
    {
        std::stringstream ss(s);

        float x = 0;
        float y = 0;
        float z = 0;

        ss >> x;
        ss >> y;
        ss >> z;

        return {x, y, z};
    }

    static Face parse_face(const std::string& s)
    {
        Face result;
        const auto vertex_data = split(s, ' ');
        for (const auto& vertex_text : vertex_data) {
            const auto raw_indices = split(vertex_text, '/');
            std::vector<int> indices;
            std::transform(raw_indices.begin(), raw_indices.end(), std::back_inserter(indices),
                           [](const auto& s) { return std::stoi(s); });
            result.indices.emplace_back(indices);
        }

        return result;
    }

    const Object& operator[](const std::string& s) const { return _objects.at(s); }

    void process_line(const std::string& s)
    {
        const auto line = partition_line(strip_whitespace(strip_comments(s)));
        if (line.command == "o") {
            const auto object_name = line.parameters;
            _object_names.push_back(object_name);
            _current_object = object_name;
            _objects[_current_object];
        } else if (line.command == "v") {
            _objects[_current_object].vertices.push_back(parse_vertex(line.parameters));
        } else if (line.command == "vt") {
            _objects[_current_object].tex_coords.push_back(
                parse_texture_coordinates(line.parameters));
        } else if (line.command == "vn") {
            _objects[_current_object].vertex_normals.push_back(
                parse_vertex_normal(line.parameters));
        } else if (line.command == "f") {
            _objects[_current_object].faces.push_back(parse_face(line.parameters));
        }
    }

    void process_text(const std::string& text)
    {
        for (const auto& line : split(text, '\n')) {
            process_line(line);
        }
    }

    int object_count() const { return static_cast<int>(_object_names.size()); }
    const std::vector<std::string>& objects() const { return _object_names; }

    void produce_triangle_list(const std::string& object_name, TriangleCollector* collector)
    {
        const Object& obj = _objects.at(object_name);
        for (const auto& face : obj.faces) {
            // grab only the first 3 vertices of each face. If output was not trianglated, we will
            // have some "holes", but at least our output will be useable.
            for (size_t i = 0; i < 3; ++i) {
                const auto v_index = static_cast<size_t>(face.indices[i].vertex - 1);
                const auto t_index = static_cast<size_t>(face.indices[i].texture - 1);
                const auto n_index = static_cast<size_t>(face.indices[i].normal - 1);
                collector->handle_vertex(obj.vertices[v_index], obj.tex_coords[t_index],
                                         obj.vertex_normals[n_index]);
            }
        }
    }

    std::vector<std::string> _object_names;
    std::map<std::string, Object> _objects;
    std::string _current_object;
};