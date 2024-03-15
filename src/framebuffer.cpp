#include "framebuffer.hpp"

Framebuffer::Framebuffer(unsigned int iWidth, unsigned int iHeight)
{
	m_width = iWidth;
	m_height = iHeight;

	glGenFramebuffers(1, &m_fbo);
}

void Framebuffer::updateDimensions(unsigned int iWidth, unsigned int iHeight)
{
	glDeleteFramebuffers(1, &m_fbo);

	m_width = iWidth;
	m_height = iHeight;
	glGenFramebuffers(1, &m_fbo);

	for (size_t i = 0; i < m_attachments.size(); ++i)
	{
		glDeleteTextures(1, &m_attachments[i].m_texture);
		GLuint texture = createTexture2D(m_width, m_height, m_attachments[i].m_internal_format, m_attachments[i].m_format, m_attachments[i].m_dataType);
		m_attachments[i].m_texture = texture;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	int attachment_color_count = 0;
	std::vector<GLuint> attachments;
	for (size_t i = 0; i < m_attachments.size(); ++i)
	{
		glBindTexture(GL_TEXTURE_2D, m_attachments[i].m_texture);
		if (m_attachments[i].m_format != GL_DEPTH_COMPONENT)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment_color_count, GL_TEXTURE_2D, m_attachments[i].m_texture, 0);
			attachments.push_back(GL_COLOR_ATTACHMENT0 + attachment_color_count);
			attachment_color_count++;
		}
		else
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_attachments[i].m_texture, 0);
		}
	}

	if (attachments.size() > 0)
	{
		glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
	}
	else
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
		std::exit(-1);
	}
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &m_fbo);
	for (size_t i = 0; i < m_attachments.size(); ++i)
	{
		glDeleteTextures(1, &m_attachments[i].m_texture);
	}
}

GLuint createTexture2D(GLuint iWidth, GLuint iHeight, GLint iInternalFormat, GLenum iFormat, GLenum iType)
{
	// CREATE TEXTURE OBJECT
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, iInternalFormat, iWidth, iHeight, 0, iFormat, iType, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

GLuint createShadowMap2D(GLuint iWidth, GLuint iHeight)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, iWidth, iHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

GLuint createAORotationVectorsTexture(std::array<glm::vec3, 16> const & iData)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	std::vector<float> data;
	for (glm::vec3 const& v : iData)
	{
		data.push_back(v.x);
		data.push_back(v.y);
		data.push_back(v.z);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, data.data());

	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

GLuint createRandomRayMarchingStepsTexture(std::array<int, 16> const& iData)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8I, 4, 4, 0, GL_RED_INTEGER, GL_INT, iData.data());

	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}