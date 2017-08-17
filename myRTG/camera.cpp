#include "camera.hpp"

void Camera::pitch(float rad)
{
	//dir = glm::rotateX(glm::vec4(dir, 0), rad);
}

void Camera::roll(float rad)
{
	//dir = glm::rotateZ(glm::vec4(dir, 0), rad);
	dir = glm::rotate(dir, (float)rad, right());
}

void Camera::yaw(float rad)
{
	//dir = glm::rotateY(glm::vec4(dir, 0), rad);
	dir = glm::rotate(dir, (float)rad, up);
}

glm::mat4 Camera::getProjection()
{
	return glm::perspective(fov, aspect, znear, zfar);
}

glm::mat4 Camera::getView() 
{ 
	return glm::lookAt(pos, pos + dir, up); 
}

glm::vec3 Camera::right()
{ 
	return glm::cross(dir, up); 
}

void Camera::rotX(double rad) 
{ 
	dir = glm::rotate(dir, (float)rad, right());
}

void Camera::rotY(double rad) 
{ 
	dir = glm::rotate(dir, (float)rad, up);
}