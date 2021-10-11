#version 330
uniform mat4 MVP;
uniform mat4 ModelMatrix;
in vec4 vPos;
in vec2 vTex;
in vec3 vNorm;

out vec3 world_normal;
out vec2 tex_coord;
void main()
{
    gl_Position = MVP * vPos;
    world_normal = mat3(ModelMatrix) * vNorm;
    tex_coord = vTex;
}
