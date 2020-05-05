#version 330 core
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec3 normal;
in vec3 fragmentViewPosition;

out vec4 color;

uniform Material material;
uniform vec3 lightColor;
uniform vec3 lightViewPosition;

void main()
{
    // ambient
    vec3 ambient = lightColor * material.ambient;

    // diffuse
    vec3 normalizedNormal = normalize(normal);
    vec3 lightDirection = normalize(lightViewPosition - fragmentViewPosition);
    float lightAngle = max(dot(normalizedNormal, lightDirection), 0.0); // take max, because value becomes negative if angle is over 90 degrees
    vec3 diffuse = lightColor * (lightAngle * material.diffuse);

    // specular
    vec3 viewDirection = normalize(-fragmentViewPosition);

    // reflect needs the light direction to be from the light to the fragment, not the other way around so we negate it
    // reflect returns the direction the light reflects, based on the normal and the light direction
    vec3 reflectDirection = reflect(-lightDirection, normalizedNormal);

    // calculate specularity by calculating the angle between the reflection and the camera direction
    // the power decides how 'shiny' the reflection will look like
    float specularity = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
    vec3 specular = lightColor * (specularity * material.specular);
    
    color = vec4(ambient + diffuse + specular, 1.0);
}