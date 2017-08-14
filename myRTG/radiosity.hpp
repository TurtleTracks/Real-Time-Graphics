#pragma once
#include "shader.hpp"

class Radiosity : public Shader
{
	GLuint blinnphong;
	GLuint smap;
	GLuint UBO[3];

	GLuint depthMapFBO;
	GLuint depthCubeMap;
	GLuint cubeWidth;

	GLuint screenFBO;
	GLuint screenTex[3];
	GLuint depthTex;
	GLuint RBO;

	GLuint transformBlock;
	GLuint lightBlock;
	GLuint surfaceBlock;

	void bindTextures(GLuint cubeMap, int pass);
	void bindLights(std::vector<Light> lights, int pass);
	void bindMeshUniforms(const SurfaceProperties &surface,
		const glm::mat4 &Projection, const glm::mat4 &View,
		const glm::mat4 &Model);
	void bindShadowMap(glm::vec3, std::vector<glm::mat4>);
	void configureCSM(GLuint length);
	void generateFBO(void);

	void mapShadow(const Light &l, const std::vector<Model> &models);
	void renderBlinnPhong(int pass);

	GLuint getScreenTex(int i) { return screenTex[i]; }
	GLuint getCubeMap() { return depthCubeMap; }
	GLuint getDepthFBO() { return depthMapFBO; }
	GLuint getScreenFBO() { return screenFBO; }
public:
	Radiosity(GLuint length) : cubeWidth(length) {}
	Radiosity() : cubeWidth(512) {}
	~Radiosity();
	void generate();
	void render();
};

