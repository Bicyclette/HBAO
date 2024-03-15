#pragma once
#include "GLCommon.h"
#include "utils.h"
#include "material.hpp"
#include "shader.hpp"

struct Vertex
{
	void set_position(float const & x, float const & y, float const & z)
	{
		m_position[0] = x;
		m_position[1] = y;
		m_position[2] = z;
	}

	void set_normal(float const& nx, float const& ny, float const& nz)
	{
		m_normal[0] = nx;
		m_normal[1] = ny;
		m_normal[2] = nz;
	}

	void set_uv(float const& u, float const& v)
	{
		m_uv[0] = u;
		m_uv[1] = v;
	}

	void set_tangent(float const& tx, float const& ty, float const& tz)
	{
		m_tangent[0] = tx;
		m_tangent[1] = ty;
		m_tangent[2] = tz;
	}

	void set_bitangent(float const& btx, float const& bty, float const& btz)
	{
		m_bitangent[0] = btx;
		m_bitangent[1] = bty;
		m_bitangent[2] = btz;
	}

	bool operator==(const Vertex & other) const
	{
		float eps = 0.001f;
		bool cond_position = glm::all(glm::epsilonEqual(other.m_position, m_position, eps));
		bool cond_normal = glm::all(glm::epsilonEqual(other.m_normal, m_normal, eps));
		bool cond_uv = glm::all(glm::epsilonEqual(other.m_uv, m_uv, eps));
		return cond_position && cond_normal && cond_uv;
	}

	glm::vec3 m_position;
	glm::vec3 m_normal;
	glm::vec2 m_uv;
	glm::vec3 m_tangent;
	glm::vec3 m_bitangent;
};

struct CPUGeometry
{
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_triangleIndices;
};

struct GPUGeometry
{
	GLuint m_vao;
	GLuint m_pos_vbo;
	GLuint m_normal_vbo;
	GLuint m_tex_coord_vbo;
	GLuint m_tangent_vbo;
	GLuint m_bitangent_vbo;
	GLuint m_ibo;

	~GPUGeometry()
	{
		glDeleteVertexArrays(1, &m_vao);
		glDeleteBuffers(1, &m_pos_vbo);
		glDeleteBuffers(1, &m_normal_vbo);
		glDeleteBuffers(1, &m_tex_coord_vbo);
		glDeleteBuffers(1, &m_tangent_vbo);
		glDeleteBuffers(1, &m_bitangent_vbo);
		glDeleteBuffers(1, &m_ibo);
	}
};

class Mesh
{
public:
	Mesh(std::string const& iFilePath, bool iDynamic);
	void create_GPU_objects();
	void draw(std::shared_ptr<Shader> iShader);

	Material m_material;
	CPUGeometry m_cpu_geometry;
	GPUGeometry m_gpu_geometry;
	bool m_dynamic;
	glm::mat4 m_model;
};