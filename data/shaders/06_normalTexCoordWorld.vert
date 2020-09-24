#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec2 iTextureCoordinates;

out vec3 normal;
out vec3 fragmentPosition;
out vec2 textureCoordinates;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldSpace = model * vec4(pos, 1.0);
    fragmentPosition = vec3(worldSpace);

    // the normal matrix is the transpose of the inverse of the upper-left 3x3 of the model * view matrix
    // (model * view because we are doing lighting in view space)
    // it is needed in case we do non uniform scales, so that the normal vector is perpendicular again
    // hint: for better performance the normal matrix should be passed as a uniform instead,
    //       since inversing a matrix is very expensive
    normal = mat3(transpose(inverse(view * model))) * iNormal;

    textureCoordinates = iTextureCoordinates;
    
    gl_Position = projection * view * worldSpace;
}