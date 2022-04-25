#pragma once

#include <GLFW/glfw3.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include <stdio.h>

class GUIWindow 
{
private:
	void drawElements();
public:
	GUIWindow();
	void init(GLFWwindow* window);
	void perFrame();
	void cleanUp();
};