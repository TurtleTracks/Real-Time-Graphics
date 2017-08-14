#pragma once
#include "camera.hpp"
#include "light.hpp"
#include "input-processor.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "particle-system.hpp"
#include "..\Dependencies\src\PUGI\pugixml.hpp"
#include <queue>
class Scene
{
public:
	std::vector<Camera> cameras;
	std::vector<Light> lights;
	std::vector<Model> models;
	ParticleSystem *ps;
	Shader *shader;
	double time;
	double timeDelta;
	bool mouseDragged = false;

	Scene();
	~Scene();

	// add objects
	void addLight(Light light);
	void addModel(Model model);
	void addCam(Camera cam);
	void addShader(Shader *s);
	void addParticleSystem(ParticleSystem *system);

	// routine 
	static Scene Scene::loadXML(pugi::xml_node  root);
	void Scene::updateSystem(double time);
};

