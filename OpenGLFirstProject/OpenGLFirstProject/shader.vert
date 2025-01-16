#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 FragCol;
//out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Exercise 4: Do Gouraud shading (Phong shading but in the vertex shader)
    // NOTE: Reverting exercise 3, so we do it in world space again
    vec3 pos = vec3(model * vec4(aPos, 1.0));
    vec3 normal = normalize(mat3(transpose(inverse(model))) * aNormal);

    vec3 lightDir = normalize(lightPos - pos);

    // Ambient
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 1f;
    vec3 viewDir = normalize(viewPos - pos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColor;

    // Result
    FragPos = pos;
    FragCol = ambient + diffuse + specular;
}