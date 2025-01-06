#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor; // Specify a color output to the fragment shader

uniform float offset;

void main()
{
    // Shaders Exercise 1: Draw the triangle upside down
    gl_Position = vec4(aPos.x, -aPos.y, aPos.z, 1.0f);

    // Shaders Exercise 2: Specify an "offset" uniform and shift the triangle to the right using it
    gl_Position = vec4(gl_Position.x + offset, gl_Position.yzw);

    // Shaders Exercise 3: Send position to frag shader as the color
    // Why is bottom left black? Because it passes through the origin, which is (0,0,0)
    vertexColor = gl_Position.xyz;

    //gl_Position = vec4(aPos.x, aPos.y + 0.25f, aPos.z, 1.0f);
    //vertexColor = aColor;
}