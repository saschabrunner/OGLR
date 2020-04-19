#version 330 core
in vec2 textureCoord;

out vec4 color;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float texture2Opacity;

void main()
{
    vec4 texel1 = texture(texture1, textureCoord);
    vec4 texel2 = texture(texture2, textureCoord);
    color = mix(texel1, texel2, texel2.a * texture2Opacity);
}