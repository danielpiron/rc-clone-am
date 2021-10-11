#version 330

in vec2 tex_coord;
in vec3 world_normal;
out vec4 fragment;

uniform sampler2D imphenzia;

void main()
{
    vec3 L = normalize(vec3(-1.0, 1.0, 1.0) - vec3(0));
    float diffuse = max(0, dot(L, normalize(world_normal)));
    fragment = diffuse * texture(imphenzia, tex_coord) + vec4(0.2, 0.1, 0.2, 0);
}