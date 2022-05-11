#pragma once

#include <GLFW/glfw3.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "scene.h"
#include <stdio.h>

class GUIWindow 
{
private:
	void drawElements(Scene& scene);
public:
	float ufoPos[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float spotPos[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	GUIWindow();
	void init(GLFWwindow* window);
	void perFrame(Scene& scene);
	void cleanUp();
};