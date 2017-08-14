#include "shader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm\ext.hpp>

using std::vector;
using std::string;

Shader::Shader()
{
	
}

Shader::~Shader()
{
}

void Shader::render()
{
	;
}

void Shader::generate()
{
	// use derived class method
}

void Shader::attach(std::vector<Camera>& cameras, 
	std::vector<Light>& lights, std::vector<Model>& models,
	ParticleSystem *ps)
{
	sceneCameras = &cameras;
	sceneLights = &lights;
	sceneModels = &models;
	scenePS = ps;
}

void Shader::use()
{
	;// use derived class method
}

void Shader::generateHelper(GLuint &program, string shader, int num)
{
	string vert = ".vert";
	string frag = ".frag";
	string geom = ".geom";
	string fld = "Shaders//";
	program = glCreateProgram();
	GLuint vertShader = compileSFiles(GL_VERTEX_SHADER, fld + shader + vert);
	GLuint fragShader = num < 2 ? 0 : compileSFiles(GL_FRAGMENT_SHADER, fld + shader + frag);
	GLuint geomShader = num < 3 ? 0 : compileSFiles(GL_GEOMETRY_SHADER, fld + shader + geom);
	glAttachShader(program, vertShader);
	if (num >= 2) glAttachShader(program, fragShader);
	if (num >= 3) glAttachShader(program, geomShader);

	glLinkProgram(program);
	glDeleteShader(vertShader);
	if (num >= 2) glDeleteShader(fragShader);
	if (num >= 3) glDeleteShader(geomShader);

	int err;
	glGetProgramiv(program, GL_LINK_STATUS, &err);

	GLint maxLength = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

	//The maxLength includes the NULL character
	std::vector<GLchar> infoLog(maxLength);
	if (maxLength > 0)
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
	{
		for (auto c : infoLog)
			std::cout << c;
		std::cout << std::endl;
	}
	if (err == GL_FALSE)
	{
		//We don't need the program anymore.
		glDeleteProgram(program);
		return;
	}
}

GLuint Shader::compileSFiles(GLenum type, string filename)
{

	//TCHAR NPath[MAX_PATH];
	//GetCurrentDirectory(MAX_PATH, NPath);

	std::fstream in(filename, std::ifstream::in);

	std::stringstream shaderData;

	shaderData << in.rdbuf();
	in.close();
	GLuint shader = glCreateShader(type);
	const string &shaderString = shaderData.str();
	const GLchar *shaderChars = shaderString.c_str();
	GLint strSize = shaderString.size();
	glShaderSource(shader, 1, &shaderChars, &strSize);
	glCompileShader(shader);
	int err = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &err);

	GLint maxLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	// The maxLength includes the NULL character
	std::vector<GLchar> errorLog(maxLength);
	if (maxLength > 0)
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
	if (type == GL_VERTEX_SHADER)
		std::cout << "VERTEX SHADER" << std::endl;
	if (type == GL_FRAGMENT_SHADER)
		std::cout << "FRAGMENT SHADER" << std::endl;
	if (type == GL_GEOMETRY_SHADER)
		std::cout << "GEOMETRY SHADER" << std::endl;
	for (auto c : errorLog)
		std::cout << c;
	std::cout << std::endl;
	// Provide the infolog in whatever manor you deem best.
	if (err == GL_FALSE)
	{
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.
		std::cout << "compilaion failure, shader not created" << std::endl;
	}
	return shader;
}