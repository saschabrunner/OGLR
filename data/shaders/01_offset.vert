#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;

uniform vec3 offset;

out vec3 vertexColor;

void main()
{
    gl_Position = vec4(pos + offset, 1.0);
    vertexColor = color;
}