#version 330 core
in vec2 textureCoord;

out vec4 color;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float texture2Opacity;

void main()
{
    color = mix(texture(texture1, textureCoord), texture(texture2, textureCoord), texture2Opacity);
}