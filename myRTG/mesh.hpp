#pragma once
#include "primitives.hpp"
#include <vector>

class Mesh
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;
public:
	//Mesh();
	//~Mesh();
	Mesh(std::vector<Vertex> &&verts, std::vector<Triangle> &&tris) :
		vertices(std::move(verts)), triangles(std::move(tris)) {}

	// Creates a cube mesh with with divs many vertices lenghtwise
	static Mesh *createCube(GLuint divs);

	// not yet implemented
	static Mesh *createSphere(GLuint divs);

	// Creates an upright sheet mesh with side length "scale"
	// and with divs many vertices lenghtwise
	static Mesh *createSheet(GLuint divs);

	// load mesh from .obj file
	static Mesh *loadMesh(std::string file);

	void generateNormals();
	void bufferMesh();
	void updateEBO();
	void updateVBO();
	void draw();
	void setSurfaceProperties();

};

