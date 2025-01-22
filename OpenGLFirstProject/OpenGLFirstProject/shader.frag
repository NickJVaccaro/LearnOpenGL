#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};

uniform Material material;

struct Light {
    //vec3 position; // We are a directional light now
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
    // Ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

    vec3 emission = texture(material.emission, TexCoords).rgb;

    vec3 result = ambient + diffuse + specular + emission;
    FragColor = vec4(result, 1.0);
}