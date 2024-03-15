#include "material.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Material::addTexture2D(std::string const & iFilePath, GLenum iFormat, TextureType iType)
{
	// CHECK IF FORMAT IS RGB OR RGBA
	if(iFormat != GL_RGB && iFormat != GL_RGBA)
	{
		std::cerr << "Unsupported texture format." << std::endl;
		std::exit(-1);
	}

	// LOAD TEXTURE DATA FROM FILE
	int width;
	int height;
	int numChannels;
	unsigned char * data = stbi_load(iFilePath.c_str(), &width, &height, &numChannels, 0);
	if(data == nullptr)
	{
		std::cerr << "Failed loading texture file \"" << iFilePath << "\" !" << std::endl;
		std::exit(-1);
	}

	// CREATE TEXTURE ON GPU
	Texture2D tex2D;
	tex2D.m_type = iType;
	glGenTextures(1, &tex2D.m_tex);
	glBindTexture(GL_TEXTURE_2D, tex2D.m_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, iFormat, width, height, 0, iFormat, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// FREE MEMORY
	stbi_image_free(data);

	// ADD TEXTURE
	m_textures.push_back(tex2D);
}