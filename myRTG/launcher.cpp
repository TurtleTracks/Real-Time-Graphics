#include "launcher.hpp"
#include "renderer.hpp"
#include "..\Dependencies\src\PUGI\pugixml.hpp"
#include <iostream>


Launcher::Launcher()
{
	glfwSetWindowUserPointer(window, this);
}


Launcher::~Launcher()
{
}

// Handle errors?
void Launcher::error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

// Grab key presses
void Launcher::key_callback(GLFWwindow* window, int key, int scancode, int action,
	int mods)
{
	Launcher *launcher = (Launcher*)glfwGetWindowUserPointer(window);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key == GLFW_KEY_W)
	{
		launcher->inputProcessor.move[W] = action;
	}
	if (key == GLFW_KEY_A)
	{
		launcher->inputProcessor.move[A] = action;
	}
	if (key == GLFW_KEY_S)
	{
		launcher->inputProcessor.move[S] = action;
	}
	if (key == GLFW_KEY_D)
	{
		launcher->inputProcessor.move[D] = action;
	}
	if (key == GLFW_KEY_UP)
	{
		launcher->inputProcessor.move[UP] = action;
	}
	if (key == GLFW_KEY_DOWN)
	{
		launcher->inputProcessor.move[DOWN] = action;
	}
	if (key == GLFW_KEY_LEFT)
	{
		launcher->inputProcessor.move[LEFT] = action;
	}
	if (key == GLFW_KEY_RIGHT)
	{
		launcher->inputProcessor.move[RIGHT] = action;
	}
	if (key == GLFW_KEY_G)
	{
	}
	if (key == GLFW_KEY_U)
	{
		launcher->inputProcessor.move[U] = action;
	}

}

void Launcher::mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	Launcher *launcher = (Launcher*)glfwGetWindowUserPointer(window);

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double newx, newy;
		glfwGetCursorPos(window, &newx, &newy);
		launcher->xpos = newx;
		launcher->ypos = newy;
		launcher->mouseDragged = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		launcher->mouseDragged = false;
	}
}

int Launcher::init_resources(void)
{
	pugi::xml_document doc;

	pugi::xml_parse_result result = doc.load_file("..//Data//test_scene.xml");
	pugi::xml_node root = doc.document_element();
	scene = Scene::loadXML(root);

	return 1;
}

void Launcher::free_resources(void)
{

}

void Launcher::updateSystem(GLFWwindow *window)
{
	double time = glfwGetTime();
	inputProcessor.handleKeyPress(&scene, &mouse);
	scene.updateSystem(time);
	
	if (mouseDragged)
	{
		double newx, newy;
		double xdelta, ydelta;
		glfwGetCursorPos(window, &newx, &newy);
		xdelta = newx - xpos;
		ydelta = newy - ypos;
		xpos = newx;
		ypos = newy;
		mouse.push({ xdelta, ydelta, GLFW_RELEASE });
	}
	
}

void Launcher::renderScene(void)
{
	Renderer::render(scene);
}

int Launcher::initialize_window(void)
{
	// Set Context, Create Window
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(800, 600, "myRTG", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowUserPointer(window, (void*)this);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);

	// Setup GLAD
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION);	// version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	glViewport(0, 0, 800, 600);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	if (1 != Launcher::init_resources()) exit(EXIT_FAILURE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Game Loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		updateSystem(window);
		renderScene();
		glfwSwapBuffers(window);
	}
	free_resources();
	glfwTerminate( );
	return 0;
}

int main(void)
{
	Launcher myPBR = Launcher();
	myPBR.initialize_window();
	return 0;
}