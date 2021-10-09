#version 330
uniform mat4 MVP;
in vec4 vPos;
in vec3 vNorm;
out vec3 color;
void main()
{
    gl_Position = MVP * vPos;
    color = vNorm;
}
