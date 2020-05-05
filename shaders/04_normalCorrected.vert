#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 iNormal;

out vec3 normal;
out vec3 fragmentViewPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 viewSpace = view * model * vec4(pos, 1.0);
    fragmentViewPosition = vec3(viewSpace);

    // the normal matrix is the transpose of the inverse of the upper-left 3x3 of the model * view matrix
    // (model * view because we are doing lighting in view space)
    // it is needed in case we do non uniform scales, so that the normal vector is perpendicular again
    // hint: for better performance the normal matrix should be passed as a uniform instead,
    //       since inversing a matrix is very expensive
    normal = mat3(transpose(inverse(view * model))) * iNormal;
    
    gl_Position = projection * viewSpace;
}