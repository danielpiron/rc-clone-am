#include <gtest/gtest.h>

#include <load_obj.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <ostream>

static const char* blender_output =
    R"(
# Blender v2.93.4 OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 1.000000 1.000000 -1.000000
v 1.000000 -1.000000 -1.000000
v 1.000000 1.000000 1.000000
v 1.000000 -1.000000 1.000000
v -1.000000 1.000000 -1.000000
v -1.000000 -1.000000 -1.000000
v -1.000000 1.000000 1.000000
v -1.000000 -1.000000 1.000000
vt 0.875000 0.500000
vt 0.625000 0.750000
vt 0.625000 0.500000
vt 0.375000 1.000000
vt 0.375000 0.750000
vt 0.625000 0.000000
vt 0.375000 0.250000
vt 0.375000 0.000000
vt 0.375000 0.500000
vt 0.125000 0.750000
vt 0.125000 0.500000
vt 0.625000 0.250000
vt 0.875000 0.750000
vt 0.625000 1.000000
vn 0.0000 1.0000 0.0000
vn 0.0000 0.0000 1.0000
vn -1.0000 0.0000 0.0000
vn 0.0000 -1.0000 0.0000
vn 1.0000 0.0000 0.0000
vn 0.0000 0.0000 -1.0000
usemtl Material
s off
f 5/1/1 3/2/1 1/3/1
f 3/2/2 8/4/2 4/5/2
f 7/6/3 6/7/3 8/8/3
f 2/9/4 8/10/4 6/11/4
f 1/3/5 4/5/5 2/9/5
f 5/12/6 2/9/6 6/7/6
f 5/1/1 7/13/1 3/2/1
f 3/2/2 7/14/2 8/4/2
f 7/6/3 5/12/3 6/7/3
f 2/9/4 4/5/4 8/10/4
f 1/3/5 3/2/5 4/5/5
f 5/12/6 1/3/6 2/9/6
)";

// 6/11/4
// v -1.000000 -1.000000 -1.000000
// vt 0.125000 0.500000
// vn 0.0000 -1.0000 0.0000

std::ostream& operator<<(std::ostream& os, const ObjFile::Face::Indices& i)
{
    os << i.vertex << '/';
    os << i.texture << '/';
    os << i.normal;
    return os;
}

std::ostream& operator<<(std::ostream& os, const ObjFile::Face& f)
{
    for (const auto& indice : f.indices) {
        os << indice << " ";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec2& v)
{
    os << glm::to_string(v);
    return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
{
    os << glm::to_string(v);
    return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec4& v)
{
    os << glm::to_string(v);
    return os;
}

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

TEST(objFileLoader, CanReadTextureCoordinates)
{
    auto text = R"(o Cube
                   v 0.0 0.25 0.5
                   vt 0.0 1.0
                   vt 0.5 -0.25
                   )";
    ObjFile obj;
    obj.process_text(text);

    std::vector<ObjFile::TextureCoordinates> expected{{0, 1.0f}, {0.5f, -0.25f}};
    ASSERT_EQ(obj.object_count(), 1);
    ASSERT_EQ(obj.objects()[0], "Cube");
    EXPECT_EQ(obj["Cube"].tex_coords, expected);
}

TEST(objFileLoader, CanReadVertexNormals)
{
    auto text = R"(o Cube
                   v 0.0 0.25 0.5
                   vt 0.0 1.0
                   vn 0.25 0.5 1.0
                   vn -0.25 -0.5 -1.0
                   )";
    ObjFile obj;
    obj.process_text(text);

    std::vector<ObjFile::VertexNormal> expected{{0.25f, 0.5f, 1.0f}, {-0.25f, -0.5f, -1.0f}};
    ASSERT_EQ(obj.object_count(), 1);
    ASSERT_EQ(obj.objects()[0], "Cube");
    EXPECT_EQ(obj["Cube"].vertex_normals, expected);
}

TEST(objFileLoader, CanReadFaces)
{
    auto text = R"(o Cube
                   f 1/1/1 2/2/2 3/3/3
                   )";
    ObjFile obj;
    obj.process_text(text);

    ObjFile::Face expected{ObjFile::Face::Indices{1, 1, 1}, ObjFile::Face::Indices{2, 2, 2},
                           ObjFile::Face::Indices{3, 3, 3}};
    ASSERT_EQ(obj.object_count(), 1);
    ASSERT_EQ(obj.objects()[0], "Cube");
    ASSERT_EQ(obj["Cube"].faces.size(), 1);
    EXPECT_EQ(obj["Cube"].faces[0], expected);
}

TEST(objFileLoader, CanProductTriangleList)
{
    struct TestCollector : public ObjFile::TriangleCollector {
        struct Vertex {
            glm::vec4 v;
            glm::vec2 t;
            glm::vec3 n;
        };

        void handle_vertex(const ObjFile::Vertex& vert, const ObjFile::TextureCoordinates& tex,
                           const ObjFile::VertexNormal& norm) override
        {
            vertices.push_back(
                {{vert.x, vert.y, vert.z, vert.w}, {tex.u, tex.v}, {norm.x, norm.y, norm.z}});
        }

        std::vector<Vertex> vertices;
    };

    ObjFile obj;
    obj.process_text(blender_output);

    TestCollector collector;
    obj.produce_triangle_list("Cube", &collector);

    ASSERT_EQ(collector.vertices.size(), 36);
    // Vertex #1
    EXPECT_EQ(collector.vertices[0].v, glm::vec4(-1.000000f, 1.000000f, -1.000000f, 1.f));
    EXPECT_EQ(collector.vertices[0].t, glm::vec2(0.875000f, 0.500000f));
    EXPECT_EQ(collector.vertices[0].n, glm::vec3(0, 1.f, 0));

    // Vertex #12
    EXPECT_EQ(collector.vertices[11].v, glm::vec4(-1.000000f, -1.000000f, -1.000000f, 1.f));
    EXPECT_EQ(collector.vertices[11].t, glm::vec2(0.125000f, 0.500000f));
    EXPECT_EQ(collector.vertices[11].n, glm::vec3(0, -1.f, 0));
}