#include "renderer.hpp"
#include "scene.hpp"
using std::vector;

void Renderer::render(Scene &scene)
{
	scene.shader->attach(scene.cameras, scene.lights, scene.models, scene.ps);
	scene.shader->render();
}

 