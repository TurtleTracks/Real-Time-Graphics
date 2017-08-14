#pragma once
#include "shader.hpp"

class ParticleShader : public Shader
{
	GLuint pshader;
public:
	ParticleShader();
	~ParticleShader();
	void generate();
	void render();
};

