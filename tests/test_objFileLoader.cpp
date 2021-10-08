#include <gtest/gtest.h>

#include <map>
#include <sstream>
#include <string>
#include <vector>

/*

struct VertexNormal {
    float x, y, z;
};

struct TextureCoordinates {
    float u, v;
}
*/

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

    struct Object {
        std::vector<Vertex> vertices;
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
        }
    }

    void process_text(const std::string& s)
    {
        std::stringstream ss(s);
        std::string line;

        while (std::getline(ss, line, '\n')) {
            process_line(line);
        }
    }

    int object_count() const { return static_cast<int>(_object_names.size()); }
    const std::vector<std::string>& objects() const { return _object_names; }
    std::vector<std::string> _object_names;
    std::map<std::string, Object> _objects;
    std::string _current_object;
};

TEST(ObjFileLineParition, CanPartitionWithSingleSpace)
{
    const auto result = ObjFile::partition_line("cmd remaining text");
    EXPECT_EQ(result.command, "cmd");
    EXPECT_EQ(result.parameters, "remaining text");
}

TEST(ObjFileStripWhitespace, CanStripProperly)
{
    EXPECT_EQ(ObjFile::strip_whitespace(""), "");
    EXPECT_EQ(ObjFile::strip_whitespace("   "), "");
    EXPECT_EQ(ObjFile::strip_whitespace("Bob Jones"), "Bob Jones");
    EXPECT_EQ(ObjFile::strip_whitespace("   Bob Jones  "), "Bob Jones");
}

TEST(objFileLoader, CanReadObjectName)
{
    auto text = "o Cube";

    ObjFile obj;
    obj.process_line(text);

    ASSERT_EQ(obj.object_count(), 1);
    ASSERT_EQ(obj.objects()[0], "Cube");
}

TEST(objFileLoader, CanIgnoreLeadingAndTrailingWhitespace)
{
    auto text = "  o Cube  ";

    ObjFile obj;
    obj.process_line(text);

    ASSERT_EQ(obj.object_count(), 1);
    ASSERT_EQ(obj.objects()[0], "Cube");
}

TEST(objFileLoader, CanIgnoreComments)
{
    auto text = "o Cube # Ignore me";

    ObjFile obj;
    obj.process_line(text);

    ASSERT_EQ(obj.object_count(), 1);
    ASSERT_EQ(obj.objects()[0], "Cube");
}

TEST(objFileLoader, CanReadVectors)
{
    auto text = R"(o Cube
                   v 0.0 0.25 0.5
                   v -1.0 0.75 0.25)";

    ObjFile obj;
    obj.process_text(text);

    std::vector<ObjFile::Vertex> expected{{0, 0.25f, 0.5f, 1.0f}, {-1.0, 0.75f, 0.25f, 1.0f}};
    ASSERT_EQ(obj.object_count(), 1);
    ASSERT_EQ(obj.objects()[0], "Cube");
    EXPECT_EQ(obj["Cube"].vertices, expected);
}