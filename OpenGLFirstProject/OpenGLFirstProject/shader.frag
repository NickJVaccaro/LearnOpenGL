#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixVal;

void main()
{
    // Original:
    FragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), 0.2);

    // Exercise 1: Make ONLY the happy face look in the other direction
    vec2 faceCoord = vec2(-texCoord.x, texCoord.y);
    FragColor = mix(texture(texture1, texCoord), texture(texture2, faceCoord), 0.2);

    // Exercise 2: Display 4 smiley faces on a single container image clamped at its edge
    // This is done in the source.cpp file, not here. It's done via different texture wraps on each of the textures

    // Exercise 3: Display only the center pixels of the texture(s) on the rectangle
    // This is also done in source.cpp, by changing the verts' texture coords to be a subset of the entire texture

    // Exercise 4: Press up/down keys to change opacity of textures
    // This is MOSTLY done in source.cpp, via input processing & setting the mixVal uniform
    FragColor = mix(texture(texture1, texCoord), texture(texture2, faceCoord), mixVal);
}