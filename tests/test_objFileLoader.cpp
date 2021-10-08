#include <gtest/gtest.h>

#include <load_obj.h>

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