#pragma once
#include "camera.hpp"
#include "model.hpp"
#include "light.hpp"
#include "particle-system.hpp"
#include "..\Dependencies\src\glad\glad.h"
#include <string>
#include <vector>


class Shader
{
protected:
	std::string path;
	std::vector<Camera> *sceneCameras;
	std::vector<Light> *sceneLights;
	std::vector<Model> *sceneModels;
	ParticleSystem *scenePS; 

	GLuint compileSFiles(GLenum, std::string);
	void generateHelper(GLuint &program, std::string shader, int num);
public:
	Shader();
	~Shader();
	virtual void use();
	virtual void generate();
	virtual void render();
	void attach(std::vector<Camera> &cameras,
		std::vector<Light> &lights,
		std::vector<Model> &models, ParticleSystem *ps);
				
};
