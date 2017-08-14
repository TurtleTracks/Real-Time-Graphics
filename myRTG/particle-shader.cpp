#include "particle-shader.hpp"
#include "particle-system.hpp"
#include "camera.hpp"

ParticleShader::ParticleShader()
{
}


ParticleShader::~ParticleShader()
{
}

void ParticleShader::generate()
{
	generateHelper(pshader, "particle", 2);
}

void ParticleShader::render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(pshader);
	Camera &cam = (*sceneCameras)[0];
	glm::mat4 P = cam.getProjection();
	glm::mat4 V = cam.getView();
	glm::mat4 M = glm::scale(glm::mat4(), glm::vec3(3));
	M = glm::translate(M, glm::vec3(-0.5f, 0, 0));
	glm::mat4 MVP = P * V * M;
	GLint loc = glGetUniformLocation(pshader, "MVP");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(MVP));
	scenePS->draw();
}