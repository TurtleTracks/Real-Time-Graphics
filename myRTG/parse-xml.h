#pragma once
#include "..\Dependencies\src\PUGI\pugixml.hpp"
struct Camera;
struct Model;
struct Light;
class ParticleSystem;
class Shader;

class ParseXML
{
public:
	ParseXML();
	~ParseXML();
	static Camera parseCamera(pugi::xml_node obj);
	static Light parseLight(pugi::xml_node obj);
	static Model parseModel(pugi::xml_node obj);
	static ParticleSystem *parsePSystem(pugi::xml_node obj);
	static Shader *parseShader(pugi::xml_node obj);
};

