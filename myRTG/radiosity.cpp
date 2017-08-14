#include "radiosity.hpp"
#include "mesh.hpp"
#include "constants.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "material.hpp"
#include <iostream>
#include <glm\ext.hpp>

using std::string;
using std::vector;

Radiosity::~Radiosity() {}

void Radiosity::generate() 
{
	generateHelper(blinnphong, "blinn-phong", 2);
	generateHelper(smap, "shadow-map", 3);

	glGenBuffers(3, UBO);
	transformBlock = glGetUniformBlockIndex(blinnphong, "ViewingTransforms");
	lightBlock = glGetUniformBlockIndex(blinnphong, "Light");
	surfaceBlock = glGetUniformBlockIndex(blinnphong, "Surface");

	glUseProgram(blinnphong);
	generateFBO();
	GLint loc = glGetUniformLocation(blinnphong, "depthMap");
	glUniform1i(loc, 1);

	loc = glGetUniformLocation(blinnphong, "prevPass");
	glUniform1ui(loc, 0);
	glUseProgram(0);

	configureCSM(cubeWidth);
}

/*
*	For a scene with Virtual Point Lights,
*	renderRadiosity() casts a cube shadow map for each light
*   and renders the scene for each shadow map (saving to a texture
*	for the first n-1 lights) over several passes and averaging
*	the pixel color over each pass.
*	The pixel averaging is done in the shader, so the shader must handle
*	reading from a cube map and render-pass merging.
*/
void Radiosity::render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, getScreenFBO());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, getScreenTex(0), 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//	shadow map pass and render pass
	//	shader must return at least 2 textures from getScrenTex()
	//	because the loop alternates between textures for writing and reading.
	for (int pass = 1; pass < sceneLights->size(); pass++)
	{
		glViewport(0, 0, 512, 512);
		glBindFramebuffer(GL_FRAMEBUFFER,getDepthFBO());
		glClear(GL_DEPTH_BUFFER_BIT);
		mapShadow((*sceneLights)[pass - 1], *sceneModels);

		glViewport(0, 0, 800, 600);
		glBindFramebuffer(GL_FRAMEBUFFER, getScreenFBO());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, getScreenTex(glm::mod(pass, 2)), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "screenFBO incomplete" << std::endl;
		renderBlinnPhong(pass);
	}
	//	final pass, renderShadow() now bound to default Framebuffer
	//	instead of custom FBO. Displays image to screen. 
	glViewport(0, 0, 512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, getDepthFBO());
	glClear(GL_DEPTH_BUFFER_BIT);
	mapShadow(sceneLights->back(), *sceneModels);

	glViewport(0, 0, 800, 600);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderBlinnPhong(sceneLights->size());
}

void Radiosity::bindTextures(GLuint cubeMap, int pass)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenTex[glm::mod(pass + 1, 2)]);

	glUniform1f(glGetUniformLocation(blinnphong, "zdist"), 25.0f);
	glUniform1ui(glGetUniformLocation(blinnphong, "pass"), pass);
}

void Radiosity::bindLights(vector<Light> lights, int pass)
{
	// Light Properties Uniform
	glBindBuffer(GL_UNIFORM_BUFFER, UBO[0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0,
		sizeof(Light), &lights[pass - 1]);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, UBO[0]);
	// set num lights
	GLint numLightsLoc = glGetUniformLocation(blinnphong, "numLights");
	glUniform1ui(numLightsLoc, lights.size());
}

void Radiosity::bindMeshUniforms(const SurfaceProperties &surface,
	const glm::mat4 &Projection, const glm::mat4 &View,
	const glm::mat4 &Model)
{
	// Surface Properties Uniform
	glBindBuffer(GL_UNIFORM_BUFFER, UBO[1]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SurfaceProperties),
		&surface);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, UBO[1]);

	// Viewing Transforms Uniform
	GLchar *names[] = { "Projection", "View", "Model" };
	GLuint indices[3];
	GLint offset[3];
	GLint size;
	glGetUniformIndices(blinnphong, 3, names, indices);
	glGetActiveUniformsiv(blinnphong, 3, indices, GL_UNIFORM_OFFSET, offset);
	glGetActiveUniformBlockiv(blinnphong, transformBlock,
		GL_UNIFORM_BLOCK_DATA_SIZE, &size);
	GLubyte *blockBuffer = (GLubyte*)malloc(size);
	memcpy(blockBuffer + offset[0], glm::value_ptr(Projection),
		sizeof(glm::mat4));
	memcpy(blockBuffer + offset[1], glm::value_ptr(View),
		sizeof(glm::mat4));
	memcpy(blockBuffer + offset[2], glm::value_ptr(Model),
		sizeof(glm::mat4));
	glBindBuffer(GL_UNIFORM_BUFFER, UBO[2]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, size, blockBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO[2]);
}

void Radiosity::bindShadowMap(glm::vec3 lpos,
	std::vector<glm::mat4> LightViews)
{
	for (GLuint i = 0; i < 6; ++i)
	{
		GLint loc = glGetUniformLocation(
			smap, ("View[" + std::to_string(i) + "]").c_str());
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(LightViews[i]));
	}

	GLint pos = glGetUniformLocation(smap, "lightPos");
	glUniform3fv(pos, 1, &lpos[0]);
#undef far
	GLint far = glGetUniformLocation(smap, "zdist");
	glUniform1f(far, 25.0f);
}

void Radiosity::generateFBO(void)
{
	glBindBuffer(GL_UNIFORM_BUFFER, UBO[1]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SurfaceProperties),
		nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO[0]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Light) * 10,
		nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO[2]);
	glBufferData(GL_UNIFORM_BUFFER, 196,
		nullptr, GL_STATIC_DRAW);

	glGenFramebuffers(1, &screenFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);

	glGenTextures(3, screenTex);
	for (int i = 0; i < 3; i++)
	{
		glBindTexture(GL_TEXTURE_2D, screenTex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB,
			GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 800, 600, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		depthTex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Radiosity::configureCSM(GLuint length)
{
	/* CubeDepthMap code taken from LearnOpengl*/
	const GLuint SHADOW_WIDTH = length, SHADOW_HEIGHT = length;
	glGenTextures(1, &depthCubeMap);
	glGenFramebuffers(1, &depthMapFBO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
	for (GLuint i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Incomplete Framebuffer" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// use shadow-map shader
void Radiosity::mapShadow(const Light &l, const std::vector<Model> &mdls)
{
	// set up light projections
	GLfloat aspect = 1.0f;
#undef near
#undef far
	GLfloat near = 0.01f;
	GLfloat far = 25.0f;
	glm::mat4 SProj = glm::perspective((float)PI / 2.0f, aspect, near, far);

	glm::vec3 lightPos = glm::vec3(l.pos);
	// Positive cube face direction        
	glm::vec3 px = glm::vec3(1.0, 0.0, 0.0);
	glm::vec3 py = glm::vec3(0.0, 1.0, 0.0);
	glm::vec3 pz = glm::vec3(0.0, 0.0, 1.0);
	// Up vectors (for glm::lookAt())
	glm::vec3 upx = glm::vec3(0.0, -1.0, 0.0);
	glm::vec3 upy = glm::vec3(0.0, 0.0, 1.0);
	glm::vec3 upz = glm::vec3(0.0, -1.0, 0.0);
	std::vector<glm::mat4> shadowTransforms(6);
	// face transform matrices
	shadowTransforms[0] = SProj * glm::lookAt(lightPos, lightPos + px, upx);
	shadowTransforms[1] = SProj * glm::lookAt(lightPos, lightPos - px, upx);
	shadowTransforms[2] = SProj * glm::lookAt(lightPos, lightPos + py, upy);
	shadowTransforms[3] = SProj * glm::lookAt(lightPos, lightPos - py, -upy);
	shadowTransforms[4] = SProj * glm::lookAt(lightPos, lightPos + pz, upz);
	shadowTransforms[5] = SProj * glm::lookAt(lightPos, lightPos - pz, upz);

	glUseProgram(smap);
	bindShadowMap(lightPos, shadowTransforms);
	for (auto &m : mdls)
	{
		// bind Model matrix
		glm::mat4 M = m.ModelMatrix;
		GLint loc = glGetUniformLocation(smap, "M");
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(M));
		m.mesh->draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// use blinn-phong shader
void Radiosity::renderBlinnPhong(int pass)
{
	//	bind shader, then lights, iterate over scene objs
	glUseProgram(blinnphong);
	bindTextures(getCubeMap(), pass);
	bindLights(*sceneLights, pass);
	Camera &cam = (*sceneCameras)[0];
	glm::mat4 P = cam.getProjection();
	glm::mat4 V = cam.getView();
	for (auto &m : *sceneModels)
	{
		glm::mat4 M = m.ModelMatrix;
		bindMeshUniforms(m.surface, P, V, M);
		m.mesh->draw();
	}
}