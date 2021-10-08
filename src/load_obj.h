#pragma once

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

    struct Object {
        std::vector<Vertex> vertices;
        std::vector<TextureCoordinates> tex_coords;
        std::vector<VertexNormal> vertex_normals;
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

    const Object& operator[](const std::string& s) const { return _objects.at(s); }

    void process_line(const std::string& s)
    {
        const auto line = partition_line(strip_whitespace(strip_comments(s)));
        std::cout << "Line " << line.command << std::endl;
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
    std::vector<std::string> _object_names;
    std::map<std::string, Object> _objects;
    std::string _current_object;
};