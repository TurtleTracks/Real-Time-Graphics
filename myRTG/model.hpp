#pragma once
#include "material.hpp"
#include <glm\glm.hpp>

class Mesh;
struct BVHT;
enum ShaderProgram;

struct Model
{
	glm::mat4 ModelMatrix;
	SurfaceProperties surface;
	Mesh *mesh;
	BVHT *bvht;
};