#pragma once
#include "..\Dependencies\src\glad\glad.h"
#include <glm\glm.hpp>
#include <glm\ext.hpp>

struct Camera
{
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 up;
	GLfloat aspect;
	GLfloat znear;
	GLfloat zfar;
	GLfloat fov;
public:
	Camera() : 
		pos(glm::vec3(0, 3, 4)),
		dir(glm::normalize(glm::vec3(0, -2.5, -4))),
		up(glm::vec3(0, 1, 0)),
		aspect(4.0f/3.0f),
		znear(0.1f),
		zfar(100),
		fov(45) {}
	void pitch(float rad); 
	void roll(float rad);
	void yaw(float rad);
	glm::mat4 getProjection();
	glm::mat4 getView();
	glm::vec3 right();
	void rotX(double rad);
	void rotY(double rad);
};

