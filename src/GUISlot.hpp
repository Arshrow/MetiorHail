#ifndef _GUI_SLOT_HPP_
#define _GUI_SLOT_HPP_


#include <glad/glad.h>
#include <GLFW/glfw3.h>


class GUISlot {
private:
	static bool inited;
	static GLFWwindow* windowPtr;

	GUISlot(){}

public:

	static const bool& g_inited();

	static void init(GLFWwindow* window_);
	static void destroy();
	static void draw();
	
};



#endif