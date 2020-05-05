#version 330 core
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 normal;
in vec3 fragmentViewPosition;

out vec4 color;

uniform Material material;
uniform Light light;

void main()
{
    // ambient
    vec3 ambient = light.ambient * material.ambient;

    // diffuse
    vec3 normalizedNormal = normalize(normal);
    vec3 lightDirection = normalize(light.position - fragmentViewPosition);
    float lightAngle = max(dot(normalizedNormal, lightDirection), 0.0); // take max, because value becomes negative if angle is over 90 degrees
    vec3 diffuse = light.diffuse * (lightAngle * material.diffuse);

    // specular
    vec3 viewDirection = normalize(-fragmentViewPosition);

    // reflect needs the light direction to be from the light to the fragment, not the other way around so we negate it
    // reflect returns the direction the light reflects, based on the normal and the light direction
    vec3 reflectDirection = reflect(-lightDirection, normalizedNormal);

    // calculate specularity by calculating the angle between the reflection and the camera direction
    // the power decides how 'shiny' the reflection will look like
    float specularity = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
    vec3 specular = light.specular * (specularity * material.specular);
    
    color = vec4(ambient + diffuse + specular, 1.0);
}