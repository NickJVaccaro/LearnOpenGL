#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};

uniform Material material;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Light light;

in vec3 FragPos;
in vec3 FragCol;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform mat4 view;
uniform vec3 viewPos;

void main()
{
    // Figure out some calculations ahead of time
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Ambient
    vec3 ambient = attenuation * light.ambient * texture(material.diffuse, TexCoords).rgb;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = attenuation * light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = attenuation * light.specular * spec * texture(material.specular, TexCoords).rgb;

    //vec3 emission = texture(material.emission, TexCoords).rgb;

    vec3 result = ambient + diffuse + specular;// + emission;
    FragColor = vec4(result, 1.0);
}