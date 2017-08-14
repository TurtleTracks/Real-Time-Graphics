#version 420 core

uniform mat4 MVP;
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec4 normal;

void main()
{
	gl_Position = MVP * vec4(vertex.xyz, 1);
}