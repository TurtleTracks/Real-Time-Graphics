#include "parse-xml.h"
#include "shader.hpp"
#include "radiosity.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "light.hpp"
#include "utilities.hpp"
#include <iostream>

ParseXML::ParseXML()
{
}


ParseXML::~ParseXML()
{
}

Camera ParseXML::parseCamera(pugi::xml_node obj)
{
	Camera cam = Camera();
	pugi::xml_attribute attr;
	if (attr = obj.attribute("znear"))
		cam.znear = attr.as_float();
	if (attr = obj.attribute("zfar"))
		cam.zfar = attr.as_float();
	if (attr = obj.attribute("fov"))
		cam.fov = attr.as_float();
	pugi::xml_node lookat = obj.child("lookat");
	if (attr = lookat.attribute("pos"))
		cam.pos = Utilities::strToVec3(attr.as_string());
	if (attr = lookat.attribute("dir"))
		cam.dir = Utilities::strToVec3(attr.as_string());
	if (attr = lookat.attribute("up"))
		cam.up = Utilities::strToVec3(attr.as_string());
	return cam;
}

Light ParseXML::parseLight(pugi::xml_node obj)
{
	Light light = Light();
	light.col = glm::vec4(1, 1, 1, 1);
	light.pos = glm::vec4(1, 1, 1, 0);
	light.type = LPOINT;
	light.isOn = 1;
	pugi::xml_attribute attr;
	if (attr = obj.attribute("col"))
		light.col = glm::vec4(Utilities::strToVec3(attr.as_string()), 1);
	if (attr = obj.attribute("pos"))
		light.pos = glm::vec4(Utilities::strToVec3(attr.as_string()), 1);
	if (attr = obj.attribute("dir"))
		light.dir = glm::vec4(Utilities::strToVec3(attr.as_string()), 1);
	if (attr = obj.attribute("angle"))
		light.spotAngle = attr.as_float();
	if (attr = obj.attribute("att"))
		light.attenuation = attr.as_float();
	if (attr = obj.attribute("isOn"))
		light.isOn = attr.as_int();
	if (attr = obj.attribute("type"))
		light.type = (LightType)attr.as_int();
	return light;
}


Model ParseXML::parseModel(pugi::xml_node obj)
{
	Model model = Model();
	model.ModelMatrix = glm::mat4();
	pugi::xml_attribute attr;
	pugi::xml_node MMat = obj.child("transform");
	if (MMat)
	{
		for (pugi::xml_node t = MMat.first_child(); t; t = t.next_sibling())
		{
			std::string name(t.name());
			glm::vec3 v;
			if ("scale" == name)
			{
				v = Utilities::strToVec3(t.attribute("vec").as_string());
				model.ModelMatrix = glm::scale(model.ModelMatrix, v);
			}
			if ("rotate" == name)
			{
				v = Utilities::strToVec3(t.attribute("vec").as_string());
				model.ModelMatrix = glm::rotate(
					model.ModelMatrix, 
					t.attribute("angle").as_float(), 
					v);
			}
			if ("translate" == name)
			{
				v = Utilities::strToVec3(t.attribute("vec").as_string());
				model.ModelMatrix = glm::translate(model.ModelMatrix, v);
			}
		}
	}
	for (pugi::xml_node m = obj.child("mesh"); m; m = m.next_sibling("mesh"))
	{
		Mesh *mesh;
		// if mesh is stored in .obj file
		if (attr = m.attribute("type"))
		{
			std::string path;
			SurfaceProperties props;
			pugi::xml_node file = m.child("filename");
			if (attr = file.attribute("value"))
				path = attr.as_string();
			else
				std::cerr << "could not open xml\n"; 
			mesh = Mesh::loadMesh(path);
			model.mesh = mesh;
			pugi::xml_node surf = m.child("surface");
			if (surf)
			{
				props = SurfaceProperties();
				if (attr = surf.attribute("kd"))
					props.Kd = glm::vec4(Utilities::strToVec3(attr.as_string()), 1);
				if (attr = surf.attribute("ks"))
					props.Ks = glm::vec4(Utilities::strToVec3(attr.as_string()), 1);
				if (attr = surf.attribute("vc"))
					props.isVertexColored = attr.as_int();
				if (attr = surf.attribute("smoothness"))
					props.smoothness = attr.as_float();
				model.surface = props;
			}
		}
			
	}
	
	return model;
}

ParticleSystem * ParseXML::parsePSystem(pugi::xml_node obj)
{
	ParticleSystem *ps = new ParticleSystem();
	pugi::xml_attribute attr;
	return ps;
}


Shader * ParseXML::parseShader(pugi::xml_node obj)
{
	Shader *shader = new Radiosity();
	pugi::xml_attribute attr;
	return shader;
}
