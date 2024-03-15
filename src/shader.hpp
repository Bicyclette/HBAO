#pragma once

#include "GLCommon.h"
#include "utils.h"

class Shader
{
public:
	enum class Type
	{
		GBUFFER,
		DEFERRED,
		DIR_SHADOWMAP,
		AO,
		HBAO,
		BLUR,
		OTHER
	};

public:
	Shader() = delete;
	Shader(std::string const & iVertex, std::string const & iFragment);
	Shader(std::string const & iVertex, std::string const & iFragment, std::string const & iGeometry);
	~Shader();
	void checkCompileError(GLuint const & iStage, GLenum iType);
	bool checkLinkError();
	void set(std::string const & iUniform, bool const & iVal);
	void set(std::string const & iUniform, int const & iVal);
	void set(std::string const & iUniform, float const & iVal);
	void set(std::string const & iUniform, glm::vec2 const & iVec);
	void set(std::string const & iUniform, glm::vec3 const & iVec);
	void set(std::string const & iUniform, glm::mat3 const & iMat);
	void set(std::string const & iUniform, glm::mat4 const & iMat);
	void use();
	Shader::Type getType();
	void setType(Shader::Type iType);

private:
	GLuint m_program;
	Shader::Type m_type;
};