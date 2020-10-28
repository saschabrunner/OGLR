#version 330 core

#define NR_POINT_LIGHTS 4

/**
 * structs
 */
struct Material {
    sampler2D textureDiffuse0;
    sampler2D textureSpecular0;
    // sampler2D textureEmissive0;
    // float emissiveVerticalOffset;
    float shininess;
};

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
    float cutOff;       // cos value of the light cut-off angle
    float outerCutOff;  // cos value of the cut-off angle of the outer, smoothed ring
};

/**
 * prototypes
 */
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection, vec3 specularTexel);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragmentViewPosition, vec3 viewDirection, vec3 specularTexel);
vec3 calculateSpotLight(SpotLight spotLight, vec3 normal, vec3 fragmentViewPosition, vec3 specularTexel);

/**
 * in/out/uniforms
 */
in vec3 normal;
in vec3 fragmentViewPosition;
in vec2 textureCoordinates;

out vec4 color;

uniform Material material;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

void main()
{
    vec3 normalizedNormal = normalize(normal);
    vec3 viewDirection = normalize(-fragmentViewPosition);
    vec3 specularTexel = vec3(texture(material.textureSpecular0, textureCoordinates));

    vec3 result = vec3(0.0);

    result += calculateDirectionalLight(directionalLight, normalizedNormal, viewDirection, specularTexel);

    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], normalizedNormal, fragmentViewPosition, viewDirection, specularTexel);
    }

    result += calculateSpotLight(spotLight, normalizedNormal, fragmentViewPosition, specularTexel);

    // add light that the object itself emits (ignore areas with any specular)
    // vec3 emission;
    // if (specularTexel.r == 0.0) 
    // {
    //     vec2 emissionTextureCoordinates = vec2(textureCoordinates.x, textureCoordinates.y + material.emissiveVerticalOffset);
    //     result += vec3(texture(material.emission, emissionTextureCoordinates));
    // }
    
    color = vec4(result, 1.0);
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection, vec3 specularTexel) {
    // ambient
    vec3 ambient = light.ambient * vec3(texture(material.textureDiffuse0, textureCoordinates));

    // diffuse
    vec3 lightDirection = normalize(-light.direction);
    float lightAngle = max(dot(normal, lightDirection), 0.0); // take max, because value becomes negative if angle is over 90 degrees
    vec3 diffuse = light.diffuse * lightAngle * vec3(texture(material.textureDiffuse0, textureCoordinates));

    // specular
    // reflect needs the light direction to be from the light to the fragment, not the other way around so we negate it
    // reflect returns the direction the light reflects, based on the normal and the light direction
    vec3 reflectDirection = reflect(-lightDirection, normal);

    // calculate specularity by calculating the angle between the reflection and the camera direction
    // the power decides how 'shiny' the reflection will look like
    float specularity = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
    vec3 specular = light.specular * specularity * specularTexel;
    
    return ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragmentViewPosition, vec3 viewDirection, vec3 specularTexel) {
    float distance = length(light.position - fragmentViewPosition);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // ambient
    vec3 ambient = light.ambient * attenuation * vec3(texture(material.textureDiffuse0, textureCoordinates));

    // diffuse
    vec3 lightDirection = normalize(light.position - fragmentViewPosition);
    float lightAngle = max(dot(normal, lightDirection), 0.0); // take max, because value becomes negative if angle is over 90 degrees
    vec3 diffuse = light.diffuse * attenuation * lightAngle * vec3(texture(material.textureDiffuse0, textureCoordinates));

    // specular
    // reflect needs the light direction to be from the light to the fragment, not the other way around so we negate it
    // reflect returns the direction the light reflects, based on the normal and the light direction
    vec3 reflectDirection = reflect(-lightDirection, normal);

    // calculate specularity by calculating the angle between the reflection and the camera direction
    // the power decides how 'shiny' the reflection will look like
    float specularity = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
    vec3 specular = light.specular * attenuation * specularity * specularTexel;
    
    return ambient + diffuse + specular;
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 fragmentViewPosition, vec3 specularTexel)
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
    vec3 ambient = light.ambient * vec3(texture(material.textureDiffuse0, textureCoordinates));

    // diffuse
    float lightAngle = max(dot(normal, lightDirection), 0.0); // take max, because value becomes negative if angle is over 90 degrees
    vec3 diffuse = light.diffuse * attenuation * intensity * lightAngle * vec3(texture(material.textureDiffuse0, textureCoordinates));

    // specular
    vec3 viewDirection = normalize(-fragmentViewPosition);

    // reflect needs the light direction to be from the light to the fragment, not the other way around so we negate it
    // reflect returns the direction the light reflects, based on the normal and the light direction
    vec3 reflectDirection = reflect(-lightDirection, normal);

    // calculate specularity by calculating the angle between the reflection and the camera direction
    // the power decides how 'shiny' the reflection will look like
    float specularity = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
    vec3 specular = light.specular * attenuation * intensity * specularity * specularTexel;
    
    // color = vec4(ambient + diffuse + specular + emission, 1.0);
    return ambient + diffuse + specular;
}