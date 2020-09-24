#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 iNormal;

out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightViewPosition;

// do lighting in the vertex shader (gouraud shading)
// works together with 01_vertColor.frag
void main()
{
    vec4 viewSpace = view * model * vec4(pos, 1.0);
    vec3 fragmentViewPosition = vec3(viewSpace);

    // the normal matrix is the transpose of the inverse of the upper-left 3x3 of the model matrix
    // it is needed in case we do non uniform scales, so that the normal vector is perpendicular again
    // hint: for better performance the normal matrix should be passed as a uniform instead,
    //       since inversing a matrix is very expensive
    vec3 normal = mat3(transpose(inverse(model * view))) * iNormal;


    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 normalizedNormal = normalize(normal);
    vec3 lightDirection = normalize(lightViewPosition - fragmentViewPosition);
    float lightAngle = max(dot(normalizedNormal, lightDirection), 0.0); // take max, because value becomes negative if angle is over 90 degrees
    vec3 diffuse = lightAngle * lightColor;

    float specularStrength = 0.5;
    vec3 viewDirection = normalize(-fragmentViewPosition);

    // reflect needs the light direction to be from the light to the fragment, not the other way around so we negate it
    // reflect returns the direction the light reflects, based on the normal and the light direction
    vec3 reflectDirection = reflect(-lightDirection, normalizedNormal);

    // calculate specularity by calculating the angle between the reflection and the camera direction
    // the multiplier decides how 'shiny' the reflection will look like
    float specularity = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
    vec3 specular = specularStrength * specularity * lightColor;
    
    vertexColor = (ambient + diffuse + specular) * objectColor;
    
    gl_Position = projection * viewSpace;
}