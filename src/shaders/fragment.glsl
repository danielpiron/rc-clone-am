#version 330

in vec2 tex_coord;
out vec4 fragment;

uniform sampler2D imphenzia;

void main()
{
    fragment = texture(imphenzia, tex_coord);
}