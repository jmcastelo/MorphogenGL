#version 330 core

layout (location = 0) in vec3 pos;

out vec3 color;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(pos, 1.0);
    color = pos;
}
