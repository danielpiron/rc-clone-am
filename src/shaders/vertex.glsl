#version 330
uniform mat4 MVP;
in vec4 vPos;
in vec2 vTex;
in vec3 vNorm;

out vec3 color;
out vec2 tex_coord;
void main()
{
    gl_Position = MVP * vPos;
    color = vNorm * 0.5 + vec3(0.5);
    tex_coord = vTex;
}
