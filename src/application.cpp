#include "application.hpp"

Application::Application(unsigned int const & iWidth, unsigned int const & iHeight)
{
	// application viewport width & height
	m_width = iWidth;
	m_height = iHeight;

	// add meshes
	create_3Dscenes();

	// set shaders
	m_shader_GBuffer = std::make_shared<Shader>(std::string(PROJECT_DIRECTORY) + "/shaders/GBuffer/vertex.glsl", std::string(PROJECT_DIRECTORY) + "/shaders/GBuffer/fragment.glsl");
	m_shader_GBuffer->setType(Shader::Type::GBUFFER);
	m_shader_deferred = std::make_shared<Shader>(std::string(PROJECT_DIRECTORY) + "/shaders/deferred/vertex.glsl", std::string(PROJECT_DIRECTORY) + "/shaders/deferred/fragment.glsl");
	m_shader_deferred->setType(Shader::Type::DEFERRED);
	m_shader_directional_shadowMap = std::make_shared<Shader>(std::string(PROJECT_DIRECTORY) + "/shaders/shadows/directional/vertex.glsl", std::string(PROJECT_DIRECTORY) + "/shaders/shadows/directional/fragment.glsl");
	m_shader_directional_shadowMap->setType(Shader::Type::DIR_SHADOWMAP);

	// user interface
	m_ui.m_draw_mode = 0;
	m_ui.shadowMap_bias = 0.001f;
	m_ui.m_ao_mode = 1;
	m_ui.m_scene = 0;
	m_ui.m_draw_ui = true;

	// Geometry Framebuffer
	create_GBuffer();

	// Directional shadowMap Framebuffer
	create_directional_shadowMap_framebuffer(2048, 2048);

	// SSAO Framebuffers
	create_ssao_framebuffers();

	// init ssao data
	init_ssao();

	// init HBAO structure
	m_hbao.init();

	// Final render quad
	create_render_quad();
}

void Application::create_3Dscenes()
{
	// ============================== SPONZA SCENE ==============================
	// ==========================================================================

	// set camera
	m_scene[0].m_cam = std::make_shared<Camera>(glm::vec3(5.0f, 1.5f, 0.0f), glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), m_width, m_height);

	// set lighting
	m_scene[0].m_directional_light = std::make_unique<DirectionalLight>(glm::vec3(5.0f, 20.0f, 0.0f), 20.0f, glm::vec3(-0.60f, -1.5f, -0.180f), glm::vec3(1.0f), 2.85f);

	m_scene[0].m_meshes.push_back(std::make_shared<Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/roof.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/roof_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/roof_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/roof_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/roof_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/vase.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/vase_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/vase_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/vase_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/vase_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/lion.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/lion_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/lion_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/lion_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/lion_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/walls.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/walls_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/walls_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/walls_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/walls_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/voutes.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/voutes_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/voutes_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/voutes_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/voutes_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/ceiling.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/ceiling_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/ceiling_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/ceiling_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/ceiling_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl0.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl0_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl0_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl0_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl0_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/ground.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/ground_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/ground_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/ground_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/ground_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl1.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl1_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl1_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl1_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl1_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/doors.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/doors_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/doors_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/doors_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/doors_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl1_2.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl1_2_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl1_2_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl1_2_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/pillars_lvl1_2_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/bar.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/bar_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/bar_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/bar_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/bar_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/green_fabric_circle.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/green_fabric_circle_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/green_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/green_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/green_fabric_circle_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/blue_fabric_circle.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/blue_fabric_circle_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/blue_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/blue_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/blue_fabric_circle_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/red_fabric_circle.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/red_fabric_circle_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/red_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/red_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/red_fabric_circle_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/blue_fabric.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/blue_fabric_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/blue_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/blue_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/blue_fabric_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/red_fabric.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/red_fabric_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/red_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/red_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/red_fabric_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/green_fabric.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/green_fabric_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/green_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/green_fabric_circle_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/green_fabric_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/cuve.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/cuve_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/cuve_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/cuve_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/cuve_normal.jpg", GL_RGB, TextureType::NORMAL);

	m_scene[0].m_meshes.push_back(std::make_shared < Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/sponza/face.obj", false));
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/face_albedo.jpg", GL_RGB, TextureType::ALBEDO);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/face_metallic_roughness.jpg", GL_RGB, TextureType::METALLIC);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/face_metallic_roughness.jpg", GL_RGB, TextureType::ROUGHNESS);
	m_scene[0].m_meshes[m_scene[0].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/sponza/face_normal.jpg", GL_RGB, TextureType::NORMAL);

	// ============================== DRAGON ==============================
	// ====================================================================

	// set camera
	m_scene[1].m_cam = std::make_shared<Camera>(glm::vec3(0.43f, 8.8f, 9.92f), glm::vec3(-2.71f, 3.69f, 1.86f), glm::vec3(0.0f, 1.0f, 0.0f), m_width, m_height);

	// set lighting
	m_scene[1].m_directional_light = std::make_unique<DirectionalLight>(glm::vec3(0.0f, 7.0f, 0.0f), 10.0f, glm::vec3(-0.60f, -1.0f, -0.25f), glm::vec3(1.0f), 2.85f);
	
	m_scene[1].m_meshes.push_back(std::make_shared<Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/dragon/dragon.obj", false));
	m_scene[1].m_meshes[m_scene[1].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/dragon/albedo.jpeg", GL_RGB, TextureType::ALBEDO);
	m_scene[1].m_meshes[m_scene[1].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/dragon/roughness.png", GL_RGB, TextureType::ROUGHNESS);
	m_scene[1].m_meshes[m_scene[1].m_meshes.size() - 1]->m_material.addTexture2D(std::string(PROJECT_DIRECTORY) + "/assets/dragon/normal.png", GL_RGB, TextureType::NORMAL);
	m_scene[1].m_meshes[m_scene[1].m_meshes.size() - 1]->m_material.m_metallic = 0.0f;

	m_scene[1].m_meshes.push_back(std::make_shared<Mesh>(std::string(PROJECT_DIRECTORY) + "/assets/dragon/plane.obj", false));
	m_scene[1].m_meshes[m_scene[1].m_meshes.size() - 1]->m_material.m_albedo = glm::vec3(1.0f);
	m_scene[1].m_meshes[m_scene[1].m_meshes.size() - 1]->m_material.m_metallic = 0.0f;
	m_scene[1].m_meshes[m_scene[1].m_meshes.size() - 1]->m_material.m_roughness = 0.65f;
}

void Application::create_GBuffer()
{
	// init geometry framebuffer
	m_g_fbo = std::make_unique<Framebuffer>(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_g_fbo->m_fbo);

	GLuint view_position_texture = createTexture2D(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, view_position_texture, 0);

	GLuint view_normal_texture = createTexture2D(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, view_normal_texture, 0);

	GLuint albedo_texture = createTexture2D(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedo_texture, 0);

	GLuint metallic_roughness_texture = createTexture2D(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, metallic_roughness_texture, 0);

	GLuint depth_texture = createTexture2D(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

	GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

	glDrawBuffers(4, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
		std::exit(-1);
	}

	Attachment viewPosTex(view_position_texture, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	Attachment viewNormalTex(view_normal_texture, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	Attachment albedoTex(albedo_texture, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	Attachment metallicRoughnessTex(metallic_roughness_texture, GL_RGBA16F, GL_RGBA, GL_FLOAT);
	Attachment depthTex(depth_texture, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
	
	m_g_fbo->m_attachments.push_back(viewPosTex);
	m_g_fbo->m_attachments.push_back(viewNormalTex);
	m_g_fbo->m_attachments.push_back(albedoTex);
	m_g_fbo->m_attachments.push_back(metallicRoughnessTex);
	m_g_fbo->m_attachments.push_back(depthTex);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::create_directional_shadowMap_framebuffer(GLuint iWidth, GLuint iHeight)
{
	m_directional_shadowMap_fbo = std::make_unique<Framebuffer>(iWidth, iHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, m_directional_shadowMap_fbo->m_fbo);

	GLuint shadowMap = createShadowMap2D(iWidth, iHeight);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: framebuffer is not complete !" << std::endl;
		std::exit(-1);
	}

	Attachment shadowMapTex(shadowMap, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);

	m_directional_shadowMap_fbo->m_attachments.push_back(shadowMapTex);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::create_ssao_framebuffers()
{
	m_ssao_fbo = std::make_unique<Framebuffer>(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	m_ssao_blurred_fbo = std::make_unique<Framebuffer>(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_ssao_fbo->m_fbo);

		GLuint ao_map = createTexture2D(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GL_RED, GL_RED, GL_FLOAT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ao_map, 0);

		GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };

		glDrawBuffers(1, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Error: framebuffer is not complete !" << std::endl;
			std::exit(-1);
		}

		Attachment aoTex(ao_map, GL_RED, GL_RED, GL_FLOAT);

		m_ssao_fbo->m_attachments.push_back(aoTex);
	}

	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_ssao_blurred_fbo->m_fbo);

		GLuint ao_blurred_map = createTexture2D(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GL_RED, GL_RED, GL_FLOAT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ao_blurred_map, 0);

		GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };

		glDrawBuffers(1, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Error: framebuffer is not complete !" << std::endl;
			std::exit(-1);
		}

		Attachment aoBlurredTex(ao_blurred_map, GL_RED, GL_RED, GL_FLOAT);

		m_ssao_blurred_fbo->m_attachments.push_back(aoBlurredTex);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::init_ssao()
{
	m_ssao.m_hemi_radius = 0.75f;
	m_ssao.m_prev_sample_count = 32;
	m_ssao.m_sample_count = 32;

	// init random rotation vectors
	for (int i = 0; i < 16; ++i)
	{
		float rx = gen_random_float(-1.0f, 1.0f);
		float ry = gen_random_float(-1.0f, 1.0f);
		glm::vec3 rvec(rx, ry, 0.0f);
		rvec = glm::normalize(rvec);
		m_ssao.m_rotationVectors[i] = rvec;
	}

	// create 4x4 texture out of those random rotation vectors
	m_ssao.m_rvec_tex = createAORotationVectorsTexture(m_ssao.m_rotationVectors);

	// create hemisphere sampling vectors
	m_ssao.populate_samples();

	// create AO shader
	m_ssao.m_shader_AO = std::make_shared<Shader>(std::string(PROJECT_DIRECTORY) + "/shaders/AO/vertex.glsl", std::string(PROJECT_DIRECTORY) + "/shaders/AO/fragment.glsl");
	m_ssao.m_shader_AO->setType(Shader::Type::AO);
	m_ssao.m_shader_AO_blur = std::make_shared<Shader>(std::string(PROJECT_DIRECTORY) + "/shaders/AO/blur/vertex.glsl", std::string(PROJECT_DIRECTORY) + "/shaders/AO/blur/fragment.glsl");
	m_ssao.m_shader_AO_blur->setType(Shader::Type::BLUR);
}

void Application::create_render_quad()
{
	// create render quad CPU data
	m_quad_vertex = std::vector<float>{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};
	m_quad_texCoords = std::vector<float>{
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};
	m_quad_index = std::vector<unsigned int>{
		0, 1, 2,
		1, 3, 2
	};

	// create render quad GPU data
	// VAO
	glGenVertexArrays(1, &m_quad_vao);
	glBindVertexArray(m_quad_vao);

	// POSITION VBO
	glGenBuffers(1, &m_quad_pos_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_quad_pos_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_quad_vertex.size() * sizeof(float), m_quad_vertex.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// TEXTURE COORDINATES VBO
	glGenBuffers(1, &m_quad_tex_coord_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_quad_tex_coord_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_quad_texCoords.size() * sizeof(float), m_quad_texCoords.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	// ELEMENT BUFFER
	glGenBuffers(1, &m_quad_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quad_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_quad_index.size() * sizeof(unsigned int), m_quad_index.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void HBAO::init()
{
	m_fbo = std::make_unique<Framebuffer>(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	m_blur_fbo = std::make_unique<Framebuffer>(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo->m_fbo);

		GLuint occlusion_texture = createTexture2D(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GL_RED, GL_RED, GL_FLOAT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, occlusion_texture, 0);

		GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };

		glDrawBuffers(1, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Error: framebuffer is not complete !" << std::endl;
			std::exit(-1);
		}

		Attachment occlusionTex(occlusion_texture, GL_RED, GL_RED, GL_FLOAT);

		m_fbo->m_attachments.push_back(occlusionTex);
	}

	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_blur_fbo->m_fbo);

		GLuint occlusion__blur_texture = createTexture2D(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, GL_RED, GL_RED, GL_FLOAT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, occlusion__blur_texture, 0);

		GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };

		glDrawBuffers(1, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Error: framebuffer is not complete !" << std::endl;
			std::exit(-1);
		}

		Attachment occlusionBlurTex(occlusion__blur_texture, GL_RED, GL_RED, GL_FLOAT);

		m_blur_fbo->m_attachments.push_back(occlusionBlurTex);
	}

	m_prev_Nd = 32;
	m_Nd = 32;
	m_Ns = 4;
	m_R = 1.15f;
	m_angle_bias = 0.35f;

	// create initial sampling directions
	for (int i = 0; i < m_Nd; ++i)
	{
		float x = gen_random_float(-1.0f, 1.0f);
		float y = gen_random_float(-1.0f, 1.0f);
		float z = 0.0f;
		glm::vec3 direction(x, y, z);
		direction = glm::normalize(direction);
		m_directions.push_back(direction);
	}

	// create 4x4 texture of random rotation vectors
	std::array<glm::vec3, 16> rotationVectors;
	for (int i = 0; i < 16; ++i)
	{
		float rx = gen_random_float(-1.0f, 1.0f);
		float ry = gen_random_float(-1.0f, 1.0f);
		glm::vec3 rvec(rx, ry, 0.0f);
		rvec = glm::normalize(rvec);
		rotationVectors[i] = rvec;
	}
	m_rvec_tex = createAORotationVectorsTexture(rotationVectors);

	// create 4x4 texture of random ray marching steps
	std::array<int, 16> randStep;
	for (int i = 0; i < 16; ++i)
	{
		int step = gen_random_int(2, 8);
		randStep[i] = step;
	}
	m_randStep_texture = createRandomRayMarchingStepsTexture(randStep);

	m_shader = std::make_shared<Shader>(std::string(PROJECT_DIRECTORY) + "/shaders/HBAO/vertex.glsl", std::string(PROJECT_DIRECTORY) + "/shaders/HBAO/fragment.glsl");
	m_shader->setType(Shader::Type::HBAO);
	m_shader_blur = std::make_shared<Shader>(std::string(PROJECT_DIRECTORY) + "/shaders/HBAO/blur/vertex.glsl", std::string(PROJECT_DIRECTORY) + "/shaders/HBAO/blur/fragment.glsl");
	m_shader_blur->setType(Shader::Type::BLUR);
}

void HBAO::recompute_directions()
{
	m_directions.clear();
	for (int i = 0; i < m_Nd; ++i)
	{
		float x = gen_random_float(-1.0f, 1.0f);
		float y = gen_random_float(-1.0f, 1.0f);
		float z = 0.0f;
		glm::vec3 direction(x, y, z);
		direction = glm::normalize(direction);
		m_directions.push_back(direction);
	}
}