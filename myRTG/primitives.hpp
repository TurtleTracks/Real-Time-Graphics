#pragma once
#include "..\Dependencies\src\glad\glad.h"
#include <glm\glm.hpp>
_declspec(align(4)) struct Vertex
{
	glm::vec4 pos;
	glm::vec4 norm;
	glm::vec4 rgba;
	glm::vec2 tex;
};

_declspec(align(4)) struct Particle
{
	glm::vec3 pos;
	GLuint zidx;
	//---
	glm::vec3 vel;
	GLfloat pad;
	//---
	glm::vec3 pred;
	GLfloat lambda;
	//---
	glm::vec3 delta;
};

#pragma pack(push, 1)
struct Triangle
{
	GLuint a;
	GLuint b;
	GLuint c;
};
#pragma pack(pop)
