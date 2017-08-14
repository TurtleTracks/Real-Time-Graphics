/*
 * Created: Femi Adegunloye
 * A Compact mesh data structure.
 */
#include "mesh.hpp"
#include "utilities.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm\glm.hpp>
#include <glm\ext.hpp>

using std::vector;
using namespace Utilities;

void splitVNT(std::string tok, unsigned int &i, unsigned int &j, unsigned int &k)
{
	for (char &c : tok)
	{
		if ('/' == c)
			c = ' ';
	}
	std::istringstream str(tok);
	str >> i >> j >> k;
	i -= 1;
	j -= 1;
	k -= 1;
}

void splitVNT(std::string tok, unsigned int &i, unsigned int &j)
{
	for (char &c : tok)
	{
		if ('/' == c)
			c = ' ';
	}
	std::istringstream str(tok);
	std::string t1;
	str >> i >> t1 >> j;
	i -= 1;
	j -= 1;
}

Mesh * Mesh::loadMesh(std::string file)
{
	std::ifstream in;
	loadFile(file, in);

	if (!in)
	{
		std::cerr << "could not open file Data\\" << file << std::endl;
		return nullptr; 
	}

	Mesh *mesh = nullptr;

	// obj format elements v, vt, vn, f
	bool hasNormals = false;
	bool hasTextures = false;
	int numVerts = 0;
	int numV = 0;
	int numVT = 0; 
	int numVN = 0;
	int numFaces = 0;
	std::string line;

	// scan file for mesh details 
	while (std::getline(in, line))
	{
		std::istringstream str(line);
		std::string elem;
		str >> elem;
		if ("#" == elem)
		{
			str >> elem;
			if ("v" == elem)
			{
				str >> numV;
			}
			else if ("f" == elem)
			{
				str >> numFaces;
			}
			else if ("vt" == elem)
			{
				str >> numVT;
				hasTextures = true;
			}
			else if ("vn" == elem)
			{
				str >> numVN;
				hasNormals = true;
			}
		}
		else break;
	}

	// clear eof, return to file start
	in.clear();
	in.seekg(0, std::ios::beg);

	std::vector<Triangle> triangles;
	std::vector<Triangle> fv;
	std::vector<glm::vec3> v;
	std::vector<glm::vec3> vn;
	std::vector<glm::vec2> vt;

	triangles.reserve(numFaces);
	fv.reserve(numFaces * 3); // vertex specified in each face list
	v.reserve(numV);
	vn.reserve(numVN);
	vt.reserve(numVT);
	std::streampos pos;
	// store v, vn, vt  
	while (std::getline(in, line))
	{
		std::istringstream str(line);
		std::string type;
		double x, y, z;
	
		str >> type;
		if ("v" == type)
		{
			str >> x >> y >> z;
			v.push_back({x, y, z});
		}
		else if ("vn" == type)
		{
			str >> x >> y >> z;
			vn.push_back({ x, y, z });
		}
		else if ("vt" == type)
		{
			str >> x >> y;
			vt.push_back({x, y});
		}
		else if ("f" == type)
		{
			if (vn.size() > 0) hasNormals = true;
			if (vt.size() > 0) hasTextures = true;
			unsigned int a, b, c;
			unsigned int i, j, k;
			unsigned int p1, p2, p3;
			std::string tok1, tok2, tok3;

			if (hasNormals && hasTextures)
			{

				str >> tok1 >> tok2 >> tok3;
				splitVNT(tok1, i, j, p1);
				fv.push_back({ i, j, p1 });

				splitVNT(tok2, i, j, p2);
				fv.push_back({ i, j, p2 });

				splitVNT(tok3, i, j, p3);
				fv.push_back({ i, j, p3 });
				triangles.push_back({ p1, p2, p3 });
			}
			else if (hasNormals)
			{
				str >> tok1 >> tok2 >> tok3;
				splitVNT(tok1, i, p1);
				fv.push_back({ i, 0, p1 });

				splitVNT(tok2, i, p2);
				fv.push_back({ i, 0, p2 });

				splitVNT(tok3, i, p3);
				fv.push_back({ i, 0, p3 });
				triangles.push_back({ p1, p2, p3 });
			}
			else if (hasTextures)
			{
				str >> tok1 >> tok2 >> tok3;
				splitVNT(tok1, p1, j);
				fv.push_back({ p1, j, 0 });

				splitVNT(tok2, p2, j);
				fv.push_back({ p2, j, 0 });

				splitVNT(tok3, p3, j);
				fv.push_back({ p3, j, 0 });
				triangles.push_back({ p1, p2, p3 });
			}
			else
			{
				str >> i >> j >> k;
				triangles.push_back({ i - 1, j - 1, k - 1 });
			}
		}
	}

	std::vector<Vertex> vertices(glm::max(v.size(), vn.size()));

	in.close();

	// combine vector data
	if (hasNormals && hasTextures)
	{
		for (Triangle p : fv)
		{
			Vertex vertex
			{
				glm::vec4(v[p.a], 1.0f),
				glm::vec4(vn[p.c], 0.0f),
				{0, 0, 0, 0},
				glm::vec2(vt[p.b]) 
			};
			vertices[p.c] = vertex; 
		}
	}
	else if (hasNormals)
	{
		for (Triangle p : fv)
		{
			Vertex vertex
			{
				glm::vec4(v[p.a], 1.0f),
				glm::vec4(vn[p.c], 0.0f),
				{ 0, 0, 0, 0 },
				{ 0, 0 }
			};
			vertices[p.c] = vertex;
		}
	}
	else if (hasTextures)
	{
		for (Triangle p : fv)
		{
			Vertex vertex
			{
				glm::vec4(v[p.a], 1.0f),
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 },
				glm::vec2(vt[p.b])
			};
			vertices[p.a] = vertex;
		}
	}
	else
	{
		unsigned int m = 0;
		for (glm::vec3 pos : v)
		{
			Vertex vertex
			{	glm::vec4(pos, 1),
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 },
				{ 0, 0 }
			};
			vertices[m] = vertex;
			m++;
		}
	}

	mesh = new Mesh(std::move(vertices), std::move(triangles));
	if (!hasNormals) mesh->generateNormals();

	return mesh;
}

void Mesh::generateNormals()
{
	// set all normals to 0 
	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i].norm = glm::vec4(0);
	}
	// calculate triangle normal contributions (with area weights)
	for (int i = 0; i < triangles.size(); i++)
	{
		glm::vec3 a = glm::vec3(vertices[triangles[i].a].pos);
		glm::vec3 b = glm::vec3(vertices[triangles[i].b].pos);
		glm::vec3 c = glm::vec3(vertices[triangles[i].c].pos);

		glm::vec3 v1 = b - a;
		glm::vec3 v2 = c - a;
		glm::vec3 n = glm::cross(v1, v2);

		vertices[triangles[i].a].norm += glm::vec4(n, 0);
		vertices[triangles[i].b].norm += glm::vec4(n, 0);
		vertices[triangles[i].c].norm += glm::vec4(n, 0);
	}
	// normalize normals 
	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i].norm = glm::normalize(vertices[i].norm);
	}
}

void Mesh::bufferMesh()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &EBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(Triangle),
		&triangles[0], GL_STATIC_DRAW);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
		&vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		(const GLvoid*)(sizeof(float) * 4));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
		(const GLvoid*)(sizeof(float) * 8));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}

void Mesh::updateEBO()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(Triangle),
		&triangles[0], GL_STATIC_DRAW);
}

void Mesh::updateVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
		&vertices[0], GL_STATIC_DRAW);
}

void Mesh::draw()
{
	glBindVertexArray(VAO);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, triangles.size() * 3, GL_UNSIGNED_INT,
		nullptr);
}
// Creates a cube mesh with with divs many vertices lenghtwise
Mesh *Mesh::createCube(GLuint divs)
{
	if (divs < 2) divs = 2;

	GLuint numFaces = 6;
	GLuint numIndsPerTri = 3;
	GLuint numVertsPerFace = divs*divs;
	GLuint numTrisPerFace = (divs - 1) * (divs - 1) * 2;
	vector<Vertex> vertices(numVertsPerFace * numFaces);
	vector<Triangle> triangles(numTrisPerFace * numFaces);
	GLfloat step = 1.0f / (divs - 1);

	// builds each face at the same time. 
	// memory accesses might be inefficient 
	for (GLuint i = 0; i < divs; i++)
	{
		for (GLuint j = 0; j < divs; j++)
		{
			GLuint index = divs*i + j;
			// left face
			vertices[index] =
			{ { -0.5f, step * i, step * j - 0.5f, 1 }, { -1, 0, 0, 0 } };
			// bottom face
			vertices[numVertsPerFace + index] =
			{ { step * j - 0.5f, 0, step * i - 0.5f, 1 }, { 0, -1, 0, 0 } };
			// back face
			vertices[numVertsPerFace * 2 + index] =
			{ { -step * j + 0.5f, step * i, -0.5f, 1 }, { 0, 0, -1, 0 } };
			// right face
			vertices[numVertsPerFace * 3 + index] =
			{ { 0.5, step * i, -step * j + 0.5f,1  }, { 1, 0, 0, 0 } };
			// top face 
			vertices[numVertsPerFace * 4 + index] =
			{ { step * j - 0.5f, 1, -step * i + 0.5f, 1 }, { 0, 1, 0, 0 } };
			// front face
			vertices[numVertsPerFace * 5 + index] =
			{ { step * j - 0.5f, step * i, 0.5f, 1 }, { 0, 0, 1, 0 } };

		}
	}
	// Another way
	GLuint n = 0;
	GLuint count = 0;
	GLuint skip = 0;
	for (int i = 0; i < triangles.size(); i += 2)
	{
		triangles[i] =
		{
			n,
			n + divs + 1,
			n + divs
		};
		triangles[i + 1] =
		{
			n,
			n + 1,
			n + divs + 1
		};

		n++;
		count++;
		if (count == divs - 1) {
			n++;
			skip++;
			count = 0;
		}
		if (skip == divs - 1) {
			n += divs;
			skip = 0;
		}
	}
	Mesh *mesh = new Mesh(std::move(vertices), std::move(triangles));
	return mesh;
}

// Creates an upright sheet mesh with side length "scale"
// and with divs many vertices lenghtwise
Mesh *Mesh::createSheet(GLuint divs)
{
	if (divs < 2) divs = 2;

	GLuint numIndsPerTri = 3;
	GLuint numVertsPerFace = divs*divs;
	GLuint numTrisPerFace = (divs - 1) * (divs - 1) * 2;
	vector<Vertex> vertices(numVertsPerFace);
	vector<GLuint> indices(numIndsPerTri * numTrisPerFace);
	vector<Triangle> triangles(numTrisPerFace);
	GLfloat step = 1.0f / (divs - 1);

	for (GLuint i = 0; i < divs; i++)
	{
		for (GLuint j = 0; j < divs; j++)
		{
			vertices[divs*i + j] =
			{ { step * j - 0.5f, 0, -step * i + 0.5f, 1 }, { 0, 1, 0, 0 }, {} };
		}
	}

	GLuint n = 0;
	GLuint count = 0;
	GLuint skip = 0;
	for (int i = 0; i < triangles.size(); i += 2) {
		triangles[i] =
		{
			n,
			n + divs + 1,
			n + divs
		};
		triangles[i + 1] =
		{
			n,
			n + 1,
			n + divs + 1
		};

		n++;
		count++;
		if (count == divs - 1) {
			n++;
			skip++;
			count = 0;
		}
		if (skip == divs - 1) {
			n += divs;
			skip = 0;
		}
	}

	Mesh *mesh = new Mesh(std::move(vertices), std::move(triangles));
	return mesh;
}
