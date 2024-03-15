#pragma once

#include "GLCommon.h"

class DirectionalLight
{
public:
	DirectionalLight(glm::vec3 const & iPosition, float const & iDimension, glm::vec3 const& iDirection, glm::vec3 const& iColor, float const& iIntensity)
	{
		m_position = iPosition;
		m_dimension = iDimension;
		m_direction = iDirection;
		m_color = iColor;
		m_intensity = iIntensity;
	}

	glm::mat4 getProjectionMatrix(float iCamNear, float iCamFar)
	{
		return glm::ortho(-m_dimension, m_dimension, -m_dimension, m_dimension, iCamNear, iCamFar);
	}

	glm::mat4 getViewMatrix()
	{
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		return glm::lookAt(m_position, m_position + m_direction, up);
	}

	glm::mat4 getLightSpaceMatrix(float iCamNear, float iCamFar)
	{
		return getProjectionMatrix(iCamNear, iCamFar) * getViewMatrix();
	}

	glm::vec3 m_position;
	float m_dimension;
	glm::vec3 m_direction;
	glm::vec3 m_color;
	float m_intensity;
};