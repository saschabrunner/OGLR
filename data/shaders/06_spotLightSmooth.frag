#version 330 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float emissionVerticalOffset;
    float shininess;
};

// spotlight with smoothed edges
struct Light {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
    float cutOff;       // cos value of the light cut-off angle
    float outerCutOff;  // cos value of the cut-off angle of the outer light ring
};

in vec3 normal;
in vec3 fragmentViewPosition;
in vec2 textureCoordinates;

out vec4 color;

uniform Material material;
uniform Light light;

void main()
{
    // vector pointing from light to fragment
    vec3 lightDirection = normalize(light.position - fragmentViewPosition);

    // angle between light direction and direction of the fragment
    float lightFragmentAngle = dot(lightDirection, normalize(-light.direction));

    // difference in angle between the cut-off and the outer cut-off
    float lightInterpolationRange = light.cutOff - light.outerCutOff;

    // ((theta - gamma) / epsilon)
    float intensity = clamp((lightFragmentAngle - light.outerCutOff) / lightInterpolationRange, 0.0, 1.0);
    
    float distance = length(light.position - fragmentViewPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, textureCoordinates));

    // diffuse
    vec3 normalizedNormal = normalize(normal);
    float lightAngle = max(dot(normalizedNormal, lightDirection), 0.0); // take max, because value becomes negative if angle is over 90 degrees
    vec3 diffuse = light.diffuse * attenuation * intensity * lightAngle * vec3(texture(material.diffuse, textureCoordinates));

    // specular
    vec3 viewDirection = normalize(-fragmentViewPosition);

    // reflect needs the light direction to be from the light to the fragment, not the other way around so we negate it
    // reflect returns the direction the light reflects, based on the normal and the light direction
    vec3 reflectDirection = reflect(-lightDirection, normalizedNormal);

    // calculate specularity by calculating the angle between the reflection and the camera direction
    // the power decides how 'shiny' the reflection will look like
    float specularity = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
    vec3 specularTexel =  vec3(texture(material.specular, textureCoordinates));
    vec3 specular = light.specular * attenuation * intensity * specularity * specularTexel;

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
    color = vec4(ambient + diffuse + specular + emission, 1.0);
}