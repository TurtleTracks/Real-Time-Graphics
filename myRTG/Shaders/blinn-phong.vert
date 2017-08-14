#version 420 core

layout (std140, binding = 0) uniform ViewingTransforms{
	mat4 Projection;
	mat4 View;
	mat4 Model;
};


layout (location = 0) in vec4 vertex;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec4 color;

out vec3 fpos;
out vec3 fnorm;
out vec3 pos;
out vec3 norm;
out vec3 eye;
out vec4 vcol;
void main()
{
	gl_Position = Projection * View * Model * vec4(vertex.xyz, 1);
	pos = vec3(View * Model * vec4(vertex.xyz, 1));
	norm = vec3(transpose(inverse(View * Model)) * normal); 
	fpos = vec3(Model * vec4(vertex.xyz, 1));
	fnorm = vec3(transpose(inverse(Model)) * normal); 
	eye = vec3(inverse(View) * vec4(0,0,0,1));
	vcol = color;
}