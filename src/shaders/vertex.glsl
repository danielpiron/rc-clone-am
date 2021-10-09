#version 330
uniform mat4 MVP;
in vec4 vPos;
out vec3 color;
void main()
{
    gl_Position = MVP * vPos;
    color = vec3(1.0, 1.0, 1.0);
}
