#pragma once
#include "constants.hpp"
#include "primitives.hpp"
#include "mesh.hpp"
#include <vector>

class ParticleSystem
{
	GLuint VAO;
	GLuint VBO;
	
	unsigned int getZidx(glm::vec3 pos);
	glm::vec3 decodeIdx(unsigned int idx);
	void updateVBO();
	void sortParticles();
	void resetHandles();
	void fillNeighborhoods();
public:
	ParticleSystem() {}
	~ParticleSystem() {}
	
	std::vector<Particle> particles;
	std::vector<Mesh> meshes;
	std::vector<glm::vec3> forces;
	void init();
	void createParticle(glm::vec3 p0);
	void bufferParticles();
	void createTower();
	void update();
	void draw();
	void reset();
	void advanceTime(double dt);
};