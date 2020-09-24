#version 330 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float emissionVerticalOffset;
    float shininess;
};

// point light with attenuation
struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 normal;
in vec3 fragmentPosition;
in vec2 textureCoordinates;

out vec4 color;

uniform vec3 viewPosition;
uniform Material material;
uniform Light light;

void main()
{
    float distance = length(light.position - fragmentPosition); // distance from light to fragment
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // ambient
    vec3 ambient = light.ambient * attenuation * vec3(texture(material.diffuse, textureCoordinates));

    // diffuse
    vec3 normalizedNormal = normalize(normal);
    vec3 lightDirection = normalize(light.position - fragmentPosition);
    float lightAngle = max(dot(normalizedNormal, lightDirection), 0.0); // take max, because value becomes negative if angle is over 90 degrees
    vec3 diffuse = light.diffuse * attenuation * lightAngle * vec3(texture(material.diffuse, textureCoordinates));

    // specular
    vec3 viewDirection = normalize(viewPosition - fragmentPosition);

    // reflect needs the light direction to be from the light to the fragment, not the other way around so we negate it
    // reflect returns the direction the light reflects, based on the normal and the light direction
    vec3 reflectDirection = reflect(-lightDirection, normalizedNormal);

    // calculate specularity by calculating the angle between the reflection and the camera direction
    // the power decides how 'shiny' the reflection will look like
    float specularity = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
    vec3 specularTexel =  vec3(texture(material.specular, textureCoordinates));
    vec3 specular = light.specular * attenuation * specularity * specularTexel;

    // add light that the object itself emits (ignore areas with any specular)
    vec3 emission;
    if (specularTexel.r == 0.0) 
    {
        vec2 emissionTextureCoordinates = vec2(textureCoordinates.x, textureCoordinates.y + material.emissionVerticalOffset);
        emission = vec3(texture(material.emission, emissionTextureCoordinates));
    }
    else
    {
        emission = vec3(0.0);
    }
    
    // color = vec4(ambient + diffuse + specular + emission, 1.0);
    color = vec4(ambient + diffuse + specular, 1.0);
}