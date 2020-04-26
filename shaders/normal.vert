#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 iNormal;

out vec3 normal;
out vec3 fragmentWorldPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 world = model * vec4(pos, 1.0);
    fragmentWorldPosition = vec3(world);
    normal = iNormal;
    gl_Position = projection * view * world;
}