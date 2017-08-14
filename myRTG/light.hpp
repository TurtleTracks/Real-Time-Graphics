#pragma once
#include "..\Dependencies\src\glad\glad.h"
#include <glm\glm.hpp>

enum LightType {LPOINT, LSPOT, LDIRECTIONAL};

_declspec(align(4)) struct Light
{
	glm::vec4 col;
	glm::vec4 pos; 
	glm::vec4 dir; // for cone or directional lights
	GLfloat spotAngle;
	GLfloat attenuation;
	GLint isOn;
	LightType type; 

	Light() : 
		col(), 
		pos(), 
		dir(), 
		spotAngle(), 
		attenuation(), 
		isOn(),
		type() {}

	Light(glm::vec3 c, glm::vec3 p) :
		col(glm::vec4(c,1)), 
		pos(glm::vec4(p,0)), 
		type(LPOINT), 
		isOn(1) {}
};
