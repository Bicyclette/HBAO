#pragma once

#include "GLCommon.h"
#include "utils.h"

struct Attachment
{
	Attachment(GLuint iTex, GLint iInternalFormat, GLenum iFormat, GLenum iType)
	{
		m_texture = iTex;
		m_internal_format = iInternalFormat;
		m_format = iFormat;
		m_dataType = iType;
	}

	GLuint m_texture;
	GLint m_internal_format;
	GLenum m_format;
	GLenum m_dataType;
};

struct Framebuffer
{
	Framebuffer(unsigned int iWidth, unsigned int iHeight);
	void updateDimensions(unsigned int iWidth, unsigned int iHeight);
	~Framebuffer();

	GLuint m_fbo;
	std::vector<Attachment> m_attachments;
	unsigned int m_width;
	unsigned int m_height;
};

GLuint createTexture2D(GLuint iWidth, GLuint iHeight, GLint iInternalFormat, GLenum iFormat, GLenum iType);
GLuint createShadowMap2D(GLuint iWidth, GLuint iHeight);
GLuint createAORotationVectorsTexture(std::array<glm::vec3, 16> const& iData);
GLuint createRandomRayMarchingStepsTexture(std::array<int, 16> const& iData);