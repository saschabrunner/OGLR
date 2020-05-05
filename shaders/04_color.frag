#version 330 core
out vec4 color;

uniform vec3 iColor;

void main()
{
    color = vec4(iColor, 1.0);
}