#include "input-processor.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include <GLFW\glfw3.h>
#include <glm\common.hpp>
#include <iostream>


void InputProcessor::handleKeyPress(Scene *scene, std::queue<MousePress> *mouseClicks)
{
	Camera &cam = scene->cameras[0];
	if (0 < move[W])
	{
		cam.pos += glm::vec3(cam.dir.x,0, cam.dir.z) * scene->timeDelta * 3;
	}
	if (0 < move[S])
	{
		cam.pos -= glm::vec3(cam.dir.x, 0, cam.dir.z) * scene->timeDelta * 3;
	}
	if (0 < move[A])
	{
		cam.pos -= cam.right() * scene->timeDelta * 3;
	}
	if (0 < move[D])
	{
		cam.pos += cam.right() * scene->timeDelta * 3;
	}
	if (0 < move[UP])
	{
		cam.pos += cam.up * scene->timeDelta * 3;
	}
	if (0 < move[DOWN])
	{
		cam.pos -= cam.up * scene->timeDelta * 3;
	}
	if (0 < move[LEFT])
	{
		cam.rotY(scene->timeDelta);
	}
	if (0 < move[RIGHT])
	{
		cam.rotY(-scene->timeDelta);
	}
	if (0 < move[U])
	{
		//scene->ps->update();
	}
	while (!mouseClicks->empty())
	{
		MousePress mouse = (MousePress)mouseClicks->front();
		
		mouseClicks->pop();
		if (GLFW_RELEASE == mouse.action)
		{
			scene->cameras[0].rotY(-mouse.x / 600);
			scene->cameras[0].rotX(-mouse.y / 800);
		}
	}
}