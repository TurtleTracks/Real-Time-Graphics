#include "scene.hpp"
#include "parse-xml.h"
#include "utilities.hpp"

Scene::Scene()
{
}


Scene::~Scene()
{
}

void Scene::addCam(Camera cam)
{
	cameras.push_back(cam);
}

void Scene::addLight(Light light)
{
	lights.push_back(light);
}

void Scene::addModel(Model model)
{
	model.mesh->bufferMesh();
	models.push_back(model);
}

void Scene::addShader(Shader *s)
{
	s->generate();
	shader = s;
}

void Scene::addParticleSystem(ParticleSystem *system)
{
	ps = system;
	ps->createTower();
}

/**
* Update call should iterate through every object in the scene and call it's update function. 
* every object in the scene should have an update call
*/
void Scene::updateSystem(double time)
{
	this->timeDelta = time - this->time;
	this->time = time;
	if(ps != nullptr)
		ps->update();
	//bounceLights();
}

/* Seperate file parsing from scene construction. Objects should already be "scene" ready
by the time add<Object>() call is made*/
using pugi::xml_node;
Scene Scene::loadXML(pugi::xml_node  root)
{
	static Scene scene;
	for (xml_node m = root.child("model"); m; m = m.next_sibling("model"))
	{
		scene.addModel(ParseXML::parseModel(m));
	}
	for (xml_node c = root.child("camera"); c; c = c.next_sibling("camera"))
	{
		scene.addCam(ParseXML::parseCamera(c));
	}
	for (xml_node l = root.child("light"); l; l = l.next_sibling("light"))
	{
		scene.addLight(ParseXML::parseLight(l));
	}
	for (xml_node ps = root.child("psystem"); ps; ps = ps.next_sibling("psystem"))
	{
		scene.addParticleSystem(ParseXML::parsePSystem(ps));
	}
	for (xml_node s = root.child("shader"); s; s = s.next_sibling("shader"))
	{
		scene.addShader(ParseXML::parseShader(s));
	}
	return scene;
}