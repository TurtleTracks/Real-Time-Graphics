#pragma once
#include "..\Dependencies\src\glad\glad.h"
#include <GLFW\glfw3.h>
#include <queue>
#include "input-processor.hpp"
#include "scene.hpp"
class Launcher
{
	double xpos;
	double ypos;
	bool mouseDragged = false;
	std::queue<KeyPress> keys;
	std::queue<MousePress> mouse;
	Scene scene;
	int init_resources(void);
	void free_resources(void);
	void renderScene(void);
	void updateSystem(GLFWwindow *window);
	GLFWwindow *window;
public:
	Launcher();
	~Launcher();
	static void error_callback(int error, const char * description);
	static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods);
	static void mouse_callback(GLFWwindow * window, int button, int action, int mods);
	int initialize_window(void);
	InputProcessor inputProcessor = InputProcessor();
	
};

int main();