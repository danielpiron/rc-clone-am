#include <gtest/gtest.h>

#include <string>
#include <vector>

/*
struct Vertex {
    float x, y, z, w = 1.f;
};

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

    static LinePartition partition_line(const std::string& line)
    {

        const auto space_index = line.find_first_of(' ');
        return {line.substr(0, space_index), line.substr(space_index + 1)};
    }

    static std::string parse_quoted_string(const std::string& quoted_string)
    {
        const auto opening_quote = quoted_string.find_first_of('"');
        const auto closing_quote = quoted_string.find_last_of('"');
        return quoted_string.substr(opening_quote + 1, closing_quote - opening_quote - 1);
    }

    void process_line(const std::string& s)
    {
        const auto line = partition_line(s);
        if (line.command == "o") {
            _object_names.push_back(parse_quoted_string(line.parameters));
        }
    }

    int object_count() const { return static_cast<int>(_object_names.size()); }
    const std::vector<std::string>& objects() const { return _object_names; }
    std::vector<std::string> _object_names;
};

TEST(ObjFileLineParition, CanPartitionWithSingleSpace)
{
    const auto result = ObjFile::partition_line("cmd remaining text");
    EXPECT_EQ(result.command, "cmd");
    EXPECT_EQ(result.parameters, "remaining text");
}

TEST(objFileLoader, CanReadObjectName)
{
    auto text = "o Cube";

    ObjFile obj;
    obj.process_line(text);

    ASSERT_EQ(obj.object_count(), 1);
    ASSERT_EQ(obj.objects()[0], "Cube");
}