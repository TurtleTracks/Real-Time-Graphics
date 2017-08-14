#version 420 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 View[6];

out vec4 pos; // FragPos from GS (output per emitvertex)

void main()
{
    for(int i = 0; i < 6; i++)
    {
        gl_Layer = i;
        for(int j = 0; j < 3; j++) 
        {
            pos = gl_in[j].gl_Position;
            gl_Position = View[i] * gl_in[j].gl_Position;
            EmitVertex();
        }    
        EndPrimitive();
    }
}  