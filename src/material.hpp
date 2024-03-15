#pragma once

#include "utils.h"
#include "GLCommon.h"

enum class TextureType
{
	ALBEDO,
	METALLIC,
	ROUGHNESS,
	NORMAL
};

struct Texture2D
{
	TextureType m_type;
	GLuint m_tex;
};

class Material
{
public:
	Material()
	{
		m_albedo = glm::vec3(0.05f, 0.95f, 0.15f);
		m_metallic = 0.0f;
		m_roughness = 0.75f;
		m_opacity = 1.0f;
	}

	~Material()
	{
		for(size_t i = 0; i < m_textures.size(); ++i)
		{
			glDeleteTextures(1, &m_textures[i].m_tex);
		}
	}

	Material(glm::vec3 const& iAlbedo, float const & iMetallic, float const & iRoughness, float const & iOpacity)
	{
		m_albedo = iAlbedo;
		m_metallic = iMetallic;
		m_roughness = iRoughness;
		m_opacity = iOpacity;
	}

	void addTexture2D(std::string const & iFilePath, GLenum iFormat, TextureType iType);

	glm::vec3 m_albedo;
	float m_metallic;
	float m_roughness;
	float m_opacity;
	std::vector<Texture2D> m_textures;
};