#pragma once

#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "light.hpp"
#include "framebuffer.hpp"

struct SSAO
{
	float m_hemi_radius;
	int m_prev_sample_count;
	int m_sample_count;
	std::array<glm::vec3, 16> m_rotationVectors;
	GLuint m_rvec_tex;
	// samples in a unit hemisphere (rescaled with m_hemi_radius in fragment shader)
	std::vector<glm::vec3> m_samples;
	std::shared_ptr<Shader> m_shader_AO;
	std::shared_ptr<Shader> m_shader_AO_blur;

	~SSAO()
	{
		glDeleteTextures(1, &m_rvec_tex);
	}

	// has to be called each time the sample count is changed
	void populate_samples()
	{
		m_samples.clear();
		for (int i = 0; i < m_sample_count; ++i)
		{
			float sx = gen_random_float(-1.0f, 1.0f);
			float sy = gen_random_float(-1.0f, 1.0f);
			float sz = gen_random_float(0.0f, 1.0f);

			glm::vec3 sample(sx, sy, sz);
			sample = glm::normalize(sample);

			sample *= lerp(0.1f, 1.0f, pow(static_cast<float>(i) / static_cast<float>(m_sample_count), 2.0f));

			m_samples.push_back(sample);
		}
	}
};

struct HBAO
{
	void init();
	void recompute_directions();

	int m_prev_Nd;
	int m_Nd;
	int m_Ns;
	float m_R;
	float m_angle_bias;

	std::vector<glm::vec3> m_directions;
	GLuint m_rvec_tex;
	GLuint m_randStep_texture;

	std::unique_ptr<Framebuffer> m_fbo;
	std::unique_ptr<Framebuffer> m_blur_fbo;
	std::shared_ptr<Shader> m_shader;
	std::shared_ptr<Shader> m_shader_blur;
};

struct Scene
{
	// camera
	std::shared_ptr<Camera> m_cam;

	// meshes
	std::vector<std::shared_ptr<Mesh>> m_meshes;

	// lighting
	std::unique_ptr<DirectionalLight> m_directional_light;
};

struct Application
{
	Application() = delete;
	Application(unsigned int const& iWidth, unsigned int const& iHeight);
	void create_3Dscenes();
	void create_GBuffer();
	void create_directional_shadowMap_framebuffer(GLuint iWidth, GLuint iHeight);
	void create_ssao_framebuffers();
	void init_ssao();
	void create_render_quad();

	// viewport dimensions
	unsigned int m_width;
	unsigned int m_height;

	// scenes
	Scene m_scene[2];

	// shaders
	std::shared_ptr<Shader> m_shader_GBuffer;
	std::shared_ptr<Shader> m_shader_deferred;
	std::shared_ptr<Shader> m_shader_directional_shadowMap;

	// mouse & user interface
	struct Mouse m_mouse;
	struct UI m_ui;

	// Geometry Framebuffer
	std::unique_ptr<Framebuffer> m_g_fbo;

	// SSAO Framebuffers
	std::unique_ptr<Framebuffer> m_ssao_fbo;
	std::unique_ptr<Framebuffer> m_ssao_blurred_fbo;

	// SSAO data
	SSAO m_ssao;

	// HBAO
	HBAO m_hbao;

	// Directional ShadowMap Framebuffer
	std::unique_ptr<Framebuffer> m_directional_shadowMap_fbo;

	// render quad
	std::vector<float> m_quad_vertex;
	std::vector<float> m_quad_texCoords;
	std::vector<unsigned int> m_quad_index;
	GLuint m_quad_vao;
	GLuint m_quad_pos_vbo;
	GLuint m_quad_tex_coord_vbo;
	GLuint m_quad_ibo;
};