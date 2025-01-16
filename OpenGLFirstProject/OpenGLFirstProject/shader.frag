#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform mat4 view;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    // Common vars
    vec3 norm = normalize(Normal);
    // Exercise 3: Also make sure to update the light position (and down below, the viewDir)
    vec3 viewLightPos = vec3(view * vec4(lightPos, 1.0));
    vec3 lightDir = normalize(viewLightPos - FragPos);

    // Ambient Light
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse Light
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular Light
    // reflect() expects a vector FROM lightsource TO frag, so we have to reverse it
    // since lightDir is lightPos - FragPos, so it points FROM frag TO lightsource
    vec3 reflectDir = reflect(-lightDir, norm); 
    float specularStrength = 0.8f;
    //vec3 viewDir = normalize(viewPos - FragPos);
    vec3 viewDir = normalize(-FragPos);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColor;

    // Resultant Color
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}