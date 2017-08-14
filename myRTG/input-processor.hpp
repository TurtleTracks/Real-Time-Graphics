#pragma once
#include <queue>

class Scene;

struct KeyPress
{
	int button;
	int action;
};

struct MousePress
{
	double x;
	double y;
	int action;
};

enum Buttons {W, A, S, D, UP, DOWN, LEFT, RIGHT, U};

class InputProcessor
{
	
public:
	//InputProcessor();
	//~InputProcessor();
	int move[10] = {};
	void handleKeyPress(Scene *scene, std::queue<MousePress> *mouse);
};

