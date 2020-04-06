#version 330 core
in vec3 vertexColor;
in vec2 vertexTexCoord;

out vec4 color;

uniform sampler2D textureData;

void main()
{
    color = texture(textureData, vertexTexCoord) * vec4(vertexColor, 1.0);
}