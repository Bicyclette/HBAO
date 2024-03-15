#include "mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace std
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.m_position) ^ (hash<glm::vec3>()(vertex.m_normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.m_uv) << 1);
		}
	};
}

Mesh::Mesh(std::string const& iFilePath, bool iDynamic)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, iFilePath.c_str()))
	{
		std::cerr << "Error loading obj file \"" << iFilePath << "\" : " << err << std::endl;
		std::exit(-1);
	}

	std::unordered_map<Vertex, uint32_t> unique_vertices;
	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			// position
			float px = attrib.vertices[3 * index.vertex_index];
			float py = attrib.vertices[3 * index.vertex_index + 1];
			float pz = attrib.vertices[3 * index.vertex_index + 2];
			// normal
			float nx = attrib.normals[3 * index.normal_index];
			float ny = attrib.normals[3 * index.normal_index + 1];
			float nz = attrib.normals[3 * index.normal_index + 2];
			// uv
			float u = attrib.texcoords[2 * index.texcoord_index];
			float v = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1];
			
			Vertex vertex;
			vertex.set_position(px, py, pz);
			vertex.set_normal(nx, ny, nz);
			vertex.set_uv(u, v);

			if (unique_vertices.count(vertex) == 0)
			{
				unique_vertices[vertex] = static_cast<uint32_t>(m_cpu_geometry.m_vertices.size());
				m_cpu_geometry.m_vertices.push_back(vertex);
			}
			m_cpu_geometry.m_triangleIndices.push_back(unique_vertices[vertex]);
		}
	}

	// compute tangent and bitangent per vertex
	for (size_t i = 0; i < m_cpu_geometry.m_triangleIndices.size(); i += 3)
	{
		uint32_t idx1 = m_cpu_geometry.m_triangleIndices[i];
		uint32_t idx2 = m_cpu_geometry.m_triangleIndices[i + 1];
		uint32_t idx3 = m_cpu_geometry.m_triangleIndices[i + 2];

		glm::vec3 v1 = m_cpu_geometry.m_vertices[idx1].m_position;
		glm::vec3 v2 = m_cpu_geometry.m_vertices[idx2].m_position;
		glm::vec3 v3 = m_cpu_geometry.m_vertices[idx3].m_position;

		glm::vec2 uv1 = m_cpu_geometry.m_vertices[idx1].m_uv;
		glm::vec2 uv2 = m_cpu_geometry.m_vertices[idx2].m_uv;
		glm::vec2 uv3 = m_cpu_geometry.m_vertices[idx3].m_uv;

		glm::vec3 edge1 = v2 - v1;
		glm::vec3 edge2 = v3 - v1;
		glm::vec2 dt_uv1 = uv2 - uv1;
		glm::vec2 dt_uv2 = uv3 - uv1;

		float f = 1.0f / (dt_uv1.x * dt_uv2.y - dt_uv2.x * dt_uv1.y);
		
		float tx = f * (dt_uv2.y * edge1.x - dt_uv1.y * edge2.x);
		float ty = f * (dt_uv2.y * edge1.y - dt_uv1.y * edge2.y);
		float tz = f * (dt_uv2.y * edge1.z - dt_uv1.y * edge2.z);

		float btx = f * (-dt_uv2.x * edge1.x + dt_uv1.x * edge2.x);
		float bty = f * (-dt_uv2.x * edge1.y + dt_uv1.x * edge2.y);
		float btz = f * (-dt_uv2.x * edge1.z + dt_uv1.x * edge2.z);
		
		m_cpu_geometry.m_vertices[idx1].set_tangent(tx, ty, tz);
		m_cpu_geometry.m_vertices[idx1].set_bitangent(btx, bty, btz);
		m_cpu_geometry.m_vertices[idx2].set_tangent(tx, ty, tz);
		m_cpu_geometry.m_vertices[idx2].set_bitangent(btx, bty, btz);
		m_cpu_geometry.m_vertices[idx3].set_tangent(tx, ty, tz);
		m_cpu_geometry.m_vertices[idx3].set_bitangent(btx, bty, btz);
	}

	// other data
	m_model = glm::mat4(1.0f);
	m_dynamic = iDynamic;
	create_GPU_objects();
}

void Mesh::create_GPU_objects()
{
	// CREATE DATA ARRAYS
	std::vector<float> positionBuffer;
	std::vector<float> normalBuffer;
	std::vector<float> uvBuffer;
	std::vector<float> tangentBuffer;
	std::vector<float> bitangentBuffer;

	for (Vertex const& v : m_cpu_geometry.m_vertices)
	{
		positionBuffer.push_back(v.m_position[0]);
		positionBuffer.push_back(v.m_position[1]);
		positionBuffer.push_back(v.m_position[2]);

		normalBuffer.push_back(v.m_normal[0]);
		normalBuffer.push_back(v.m_normal[1]);
		normalBuffer.push_back(v.m_normal[2]);

		uvBuffer.push_back(v.m_uv[0]);
		uvBuffer.push_back(v.m_uv[1]);

		tangentBuffer.push_back(v.m_tangent[0]);
		tangentBuffer.push_back(v.m_tangent[1]);
		tangentBuffer.push_back(v.m_tangent[2]);

		bitangentBuffer.push_back(v.m_bitangent[0]);
		bitangentBuffer.push_back(v.m_bitangent[1]);
		bitangentBuffer.push_back(v.m_bitangent[2]);
	}

	// VAO
	glGenVertexArrays(1, &m_gpu_geometry.m_vao);
	glBindVertexArray(m_gpu_geometry.m_vao);

	// POSITION VBO
	glGenBuffers(1, &m_gpu_geometry.m_pos_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_gpu_geometry.m_pos_vbo);
	glBufferData(GL_ARRAY_BUFFER, positionBuffer.size() * sizeof(float), positionBuffer.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// NORMAL VBO
	glGenBuffers(1, &m_gpu_geometry.m_normal_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_gpu_geometry.m_normal_vbo);
	glBufferData(GL_ARRAY_BUFFER, normalBuffer.size() * sizeof(float), normalBuffer.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// TEXTURE COORDINATES VBO
	glGenBuffers(1, &m_gpu_geometry.m_tex_coord_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_gpu_geometry.m_tex_coord_vbo);
	glBufferData(GL_ARRAY_BUFFER, uvBuffer.size() * sizeof(float), uvBuffer.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	// TANGENT VBO
	glGenBuffers(1, &m_gpu_geometry.m_tangent_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_gpu_geometry.m_tangent_vbo);
	glBufferData(GL_ARRAY_BUFFER, tangentBuffer.size() * sizeof(float), tangentBuffer.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// BITANGENT VBO
	glGenBuffers(1, &m_gpu_geometry.m_bitangent_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_gpu_geometry.m_bitangent_vbo);
	glBufferData(GL_ARRAY_BUFFER, bitangentBuffer.size() * sizeof(float), bitangentBuffer.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// ELEMENT BUFFER
	glGenBuffers(1, &m_gpu_geometry.m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gpu_geometry.m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_cpu_geometry.m_triangleIndices.size() * sizeof(unsigned int), m_cpu_geometry.m_triangleIndices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void Mesh::draw(std::shared_ptr<Shader> iShader)
{
	iShader->use();

	if (iShader->getType() == Shader::Type::GBUFFER)
	{
		iShader->set("material.albedo", m_material.m_albedo);
		iShader->set("material.roughness", m_material.m_roughness);
		iShader->set("material.metallic", m_material.m_metallic);
		iShader->set("material.opacity", m_material.m_opacity);

		iShader->set("material.has_tex_albedo", false);
		iShader->set("material.has_tex_metallic", false);
		iShader->set("material.has_tex_roughness", false);
		iShader->set("material.has_tex_normal", false);
		for (size_t i = 0; i < m_material.m_textures.size(); ++i)
		{
			TextureType tex_type = m_material.m_textures[i].m_type;
			GLuint tex_handler = m_material.m_textures[i].m_tex;

			glActiveTexture(GL_TEXTURE0 + static_cast<int>(i));
			glBindTexture(GL_TEXTURE_2D, tex_handler);

			if (tex_type == TextureType::ALBEDO)
			{
				iShader->set("material.tex_albedo", static_cast<int>(i));
				iShader->set("material.has_tex_albedo", true);
			}
			else if (tex_type == TextureType::METALLIC)
			{
				iShader->set("material.tex_metallic", static_cast<int>(i));
				iShader->set("material.has_tex_metallic", true);
			}
			else if (tex_type == TextureType::ROUGHNESS)
			{
				iShader->set("material.tex_roughness", static_cast<int>(i));
				iShader->set("material.has_tex_roughness", true);
			}
			else if (tex_type == TextureType::NORMAL)
			{
				iShader->set("material.tex_normal", static_cast<int>(i));
				iShader->set("material.has_tex_normal", true);
			}
		}
	}

	glBindVertexArray(m_gpu_geometry.m_vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_cpu_geometry.m_triangleIndices.size()), GL_UNSIGNED_INT, 0);
}