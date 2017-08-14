#pragma once
#include "..\Dependencies\src\glad\glad.h"
#include <glm\common.hpp>

_declspec(align(4)) struct SurfaceProperties
{
	glm::vec4 Kd;
	glm::vec4 Ks;
	GLuint isVertexColored;
	GLfloat smoothness;
	//GLfloat texcoords[2];
	SurfaceProperties() : Kd(glm::vec4(0)), Ks(Kd), isVertexColored(0),
		smoothness(0) {}
};
