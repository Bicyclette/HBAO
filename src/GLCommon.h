#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/epsilon.hpp>

#define VIEWPORT_WIDTH 1600
#define VIEWPORT_HEIGHT 900

struct Mouse
{
	Mouse()
	{
		m_prevX = 0.0;
		m_prevY = 0.0;
		m_currX = 0.0;
		m_currY = 0.0;
	}

	double m_prevX;
	double m_prevY;
	double m_currX;
	double m_currY;
};

struct UI
{
	int m_draw_mode; // 0 = shaded, 1 = depth, 2 = albedo, 3 = view position, 4 = view normal, 5 = shadowMap, 6 = SSAO, 7 = HBAO
	float shadowMap_bias;
	int m_ao_mode; // 0 = SSAO, 1 = HBAO
	int m_scene; // 0 = sponza, 1 = dragon
	bool m_draw_ui;
};