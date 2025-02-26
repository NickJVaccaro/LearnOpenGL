#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
    //vec3 fColor;
} gs_in[];

out vec2 TexCoords;
//out vec3 fColor;

void main()
{
    gl_Position = gl_in[0].gl_Position;
    TexCoords = gs_in[0].texCoords;
    //fColor = gs_in[0].fColor;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    TexCoords = gs_in[1].texCoords;
    //fColor = gs_in[1].fColor;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    TexCoords = gs_in[2].texCoords;
    //fColor = gs_in[2].fColor;
    EmitVertex();

    EndPrimitive();
}