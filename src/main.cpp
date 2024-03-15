#include "application.hpp"

// ==================== FUNCTION HEADERS ====================
// ==========================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void initScene();
void shadowPass();
void ssao_pass();
void hbao_pass();
void renderScene();
void draw_UI();
void ImGui_init(GLFWwindow* win);
void set_deferred_shader();

// ==================== GLOBAL OBJECTS ====================
// ========================================================

std::shared_ptr<Application> g_app;

// ==================== FUNCTION DEFINITIONS ====================
// ==============================================================

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	g_app->m_width = width;
	g_app->m_height = height;
	g_app->m_scene[0].m_cam->updateProjectionMatrix(width, height);
	g_app->m_scene[1].m_cam->updateProjectionMatrix(width, height);
	g_app->m_g_fbo->updateDimensions(width, height);
	g_app->m_ssao_fbo->updateDimensions(width, height);
	g_app->m_ssao_blurred_fbo->updateDimensions(width, height);
	g_app->m_hbao.m_fbo->updateDimensions(width, height);
	g_app->m_hbao.m_blur_fbo->updateDimensions(width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	int activeScene = g_app->m_ui.m_scene;
	g_app->m_scene[activeScene].m_cam->zoomView(static_cast<float>(yoffset));
}

void processInput(GLFWwindow* window)
{
	int activeScene = g_app->m_ui.m_scene;

	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS) // recompile deferred shader, and gbuffer shader
	{
		g_app->m_shader_deferred = std::make_shared<Shader>("shaders/deferred/vertex.glsl", "shaders/deferred/fragment.glsl");
		g_app->m_shader_GBuffer = std::make_shared<Shader>("shaders/gbuffer/vertex.glsl", "shaders/gbuffer/fragment.glsl");
		g_app->m_ssao.m_shader_AO = std::make_shared<Shader>("shaders/AO/vertex.glsl", "shaders/AO/fragment.glsl");
		g_app->m_hbao.m_shader = std::make_shared<Shader>("shaders/HBAO/vertex.glsl", "shaders/HBAO/fragment.glsl");
	}

	// CAMERA RELATED
	double xpos;
	double ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	g_app->m_mouse.m_prevX = g_app->m_mouse.m_currX;
	g_app->m_mouse.m_prevY = g_app->m_mouse.m_currY;
	g_app->m_mouse.m_currX = xpos;
	g_app->m_mouse.m_currY = ypos;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS)
	{
		g_app->m_scene[activeScene].m_cam->rotateView(g_app->m_mouse);
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		g_app->m_scene[activeScene].m_cam->panView(g_app->m_mouse);
	}
}

void initScene()
{
	g_app = std::make_shared<Application>(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
}

void shadowPass()
{
	int activeScene = g_app->m_ui.m_scene;

	glCullFace(GL_FRONT);
	// Bind directional shadowMap Framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, g_app->m_directional_shadowMap_fbo->m_fbo);
	glViewport(0, 0, g_app->m_directional_shadowMap_fbo->m_width, g_app->m_directional_shadowMap_fbo->m_height);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Use directional shadowMap shader
	g_app->m_shader_directional_shadowMap->use();
	g_app->m_shader_directional_shadowMap->set("lightSpaceMatrix", g_app->m_scene[activeScene].m_directional_light->getLightSpaceMatrix(g_app->m_scene[activeScene].m_cam->getNear(), g_app->m_scene[activeScene].m_cam->getFar()));

	// render scene
	for (size_t i = 0; i < g_app->m_scene[activeScene].m_meshes.size(); ++i)
	{
		g_app->m_shader_directional_shadowMap->set("model", g_app->m_scene[activeScene].m_meshes[i]->m_model);
		g_app->m_scene[activeScene].m_meshes[i]->draw(g_app->m_shader_directional_shadowMap);
	}

	// reset
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, g_app->m_width, g_app->m_height);
	glCullFace(GL_BACK);
}

void renderScene()
{
	int activeScene = g_app->m_ui.m_scene;

	// DIRECTIONAL SHADOW PASS
	shadowPass();

	// RENDER TO GEOMETRY FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, g_app->m_g_fbo->m_fbo);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_app->m_shader_GBuffer->use();
	g_app->m_shader_GBuffer->set("proj", g_app->m_scene[activeScene].m_cam->getProjectionMatrix());
	for (size_t i = 0; i < g_app->m_scene[activeScene].m_meshes.size(); ++i)
	{
		glm::mat4 modelView = g_app->m_scene[activeScene].m_cam->getViewMatrix() * g_app->m_scene[activeScene].m_meshes[i]->m_model;
		glm::mat4 normalMat = glm::transpose(glm::inverse(g_app->m_scene[activeScene].m_cam->getViewMatrix() * g_app->m_scene[activeScene].m_meshes[i]->m_model));
		g_app->m_shader_GBuffer->set("modelView", modelView);
		g_app->m_shader_GBuffer->set("normalMat", glm::mat3(normalMat));
		g_app->m_scene[activeScene].m_meshes[i]->draw(g_app->m_shader_GBuffer);
	}

	if (g_app->m_ui.m_ao_mode == 0)
	{
		// AMBIENT OCCLUSION
		ssao_pass();
	}
	else if (g_app->m_ui.m_ao_mode == 1)
	{
		// DIRECTIONAL OCCLUSION
		hbao_pass();
	}

	// DEFERRED RENDERING
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_deferred_shader();
	glBindVertexArray(g_app->m_quad_vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_app->m_quad_index.size()), GL_UNSIGNED_INT, 0);

	// reset
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

void draw_UI()
{
	int activeScene = g_app->m_ui.m_scene;

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(300, 80));
	ImGui::Begin("Scene");
	ImGui::RadioButton("Sponza", &g_app->m_ui.m_scene, 0);
	ImGui::RadioButton("Dragon", &g_app->m_ui.m_scene, 1);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 80));
	ImGui::SetNextWindowSize(ImVec2(300, 55));
	if (g_app->m_ui.m_scene == 0)
	{
		ImGui::Begin("Sponza");
	}
	else if (g_app->m_ui.m_scene == 1)
	{
		ImGui::Begin("Dragon");
	}
	ImGui::RadioButton("shaded", &g_app->m_ui.m_draw_mode, 0);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 135));
	ImGui::SetNextWindowSize(ImVec2(300, 130));
	ImGui::Begin("GBuffer");
	ImGui::RadioButton("show per fragment depth", &g_app->m_ui.m_draw_mode, 1);
	ImGui::RadioButton("show per fragment albedo", &g_app->m_ui.m_draw_mode, 2);
	ImGui::RadioButton("show per fragment (view) position", &g_app->m_ui.m_draw_mode, 3);
	ImGui::RadioButton("show per fragment (view) normal", &g_app->m_ui.m_draw_mode, 4);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 265));
	ImGui::SetNextWindowSize(ImVec2(300, 80));
	ImGui::Begin("Directional shadow map");
	ImGui::RadioButton("Show", &g_app->m_ui.m_draw_mode, 5);
	ImGui::SliderFloat("bias", &g_app->m_ui.shadowMap_bias, 0.0f, 0.005f);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 345));
	ImGui::SetNextWindowSize(ImVec2(300, 110));
	ImGui::Begin("Directional light");
	ImGui::ColorEdit3("color", glm::value_ptr(g_app->m_scene[activeScene].m_directional_light->m_color));
	ImGui::SliderFloat3("direction", glm::value_ptr(g_app->m_scene[activeScene].m_directional_light->m_direction), -5.0f, 5.0f);
	ImGui::SliderFloat("intensity", &g_app->m_scene[activeScene].m_directional_light->m_intensity, 0.0f, 10.0f);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 455));
	ImGui::SetNextWindowSize(ImVec2(500, 100));
	ImGui::Begin("AO mode");
	ImGui::RadioButton("SSAO", &g_app->m_ui.m_ao_mode, 0);
	ImGui::RadioButton("HBAO", &g_app->m_ui.m_ao_mode, 1);
	ImGui::RadioButton("disable", &g_app->m_ui.m_ao_mode, 2);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 555));
	ImGui::SetNextWindowSize(ImVec2(500, 105));
	ImGui::Begin("SSAO");
	ImGui::SliderFloat("hemisphere radius", &g_app->m_ssao.m_hemi_radius, 0.5f, 2.5f);
	ImGui::SliderInt("samples", &g_app->m_ssao.m_sample_count, 16, 64);
	ImGui::RadioButton("Show SSAO texture", &g_app->m_ui.m_draw_mode, 6);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 660));
	ImGui::SetNextWindowSize(ImVec2(500, 150));
	ImGui::Begin("HBAO");
	ImGui::SliderFloat("radius", &g_app->m_hbao.m_R, 0.5f, 2.5f);
	ImGui::SliderInt("direction count", &g_app->m_hbao.m_Nd, 16, 64);
	ImGui::SliderInt("ray marching steps", &g_app->m_hbao.m_Ns, 4, 16);
	ImGui::SliderFloat("angle bias", &g_app->m_hbao.m_angle_bias, glm::radians(10.0f), glm::radians(45.0f));
	ImGui::RadioButton("Show HBAO texture", &g_app->m_ui.m_draw_mode, 7);
	ImGui::End();
}

void ImGui_init(GLFWwindow* win)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(win, true);
	ImGui_ImplOpenGL3_Init("#version 410");
}

void ssao_pass()
{
	int activeScene = g_app->m_ui.m_scene;

	glBindFramebuffer(GL_FRAMEBUFFER, g_app->m_ssao_fbo->m_fbo);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	g_app->m_ssao.m_shader_AO->use();
	
	// send sampling vectors
	g_app->m_ssao.m_shader_AO->set("sample_count", g_app->m_ssao.m_sample_count);
	g_app->m_ssao.m_shader_AO->set("hemi_radius", g_app->m_ssao.m_hemi_radius);
	for (size_t i = 0; i < g_app->m_ssao.m_samples.size(); ++i)
	{
		g_app->m_ssao.m_shader_AO->set("samples[" + std::to_string(i) + "]", g_app->m_ssao.m_samples[i]);
	}

	// send textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_app->m_g_fbo->m_attachments[0].m_texture);
	g_app->m_ssao.m_shader_AO->set("fViewPos", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_app->m_g_fbo->m_attachments[1].m_texture);
	g_app->m_ssao.m_shader_AO->set("fViewNormal", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_app->m_ssao.m_rvec_tex);
	g_app->m_ssao.m_shader_AO->set("rvec_texture", 2);

	// projection matrix
	g_app->m_ssao.m_shader_AO->set("proj", g_app->m_scene[activeScene].m_cam->getProjectionMatrix());

	// camera data
	g_app->m_ssao.m_shader_AO->set("screenWidth", static_cast<float>(g_app->m_width));
	g_app->m_ssao.m_shader_AO->set("screenHeight", static_cast<float>(g_app->m_height));

	// draw quad
	glBindVertexArray(g_app->m_quad_vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_app->m_quad_index.size()), GL_UNSIGNED_INT, 0);

	// apply blur
	glBindFramebuffer(GL_FRAMEBUFFER, g_app->m_ssao_blurred_fbo->m_fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	g_app->m_ssao.m_shader_AO_blur->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_app->m_ssao_fbo->m_attachments[0].m_texture);
	g_app->m_ssao.m_shader_AO_blur->set("fAO", 0);

	glBindVertexArray(g_app->m_quad_vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_app->m_quad_index.size()), GL_UNSIGNED_INT, 0);

	// reset
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

void hbao_pass()
{
	int activeScene = g_app->m_ui.m_scene;

	glBindFramebuffer(GL_FRAMEBUFFER, g_app->m_hbao.m_fbo->m_fbo);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	g_app->m_hbao.m_shader->use();

	// send direction vectors
	for (size_t i = 0; i < g_app->m_hbao.m_directions.size(); ++i)
	{
		g_app->m_hbao.m_shader->set("directions[" + std::to_string(i) + "]", g_app->m_hbao.m_directions[i]);
	}
	g_app->m_hbao.m_shader->set("Nd", g_app->m_hbao.m_Nd);
	g_app->m_hbao.m_shader->set("Ns", g_app->m_hbao.m_Ns);
	g_app->m_hbao.m_shader->set("R", g_app->m_hbao.m_R);
	g_app->m_hbao.m_shader->set("angle_bias", g_app->m_hbao.m_angle_bias);

	// send textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_app->m_hbao.m_rvec_tex);
	g_app->m_hbao.m_shader->set("rvec_texture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_app->m_hbao.m_randStep_texture);
	g_app->m_hbao.m_shader->set("randStep_texture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_app->m_g_fbo->m_attachments[0].m_texture);
	g_app->m_hbao.m_shader->set("fViewPos", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_app->m_g_fbo->m_attachments[1].m_texture);
	g_app->m_hbao.m_shader->set("fViewNormal", 3);

	// projection matrix
	g_app->m_hbao.m_shader->set("proj", g_app->m_scene[activeScene].m_cam->getProjectionMatrix());

	// camera data
	g_app->m_hbao.m_shader->set("screenWidth", static_cast<float>(g_app->m_width));
	g_app->m_hbao.m_shader->set("screenHeight", static_cast<float>(g_app->m_height));

	// draw quad
	glBindVertexArray(g_app->m_quad_vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_app->m_quad_index.size()), GL_UNSIGNED_INT, 0);

	// apply horizontal blur
	glBindFramebuffer(GL_FRAMEBUFFER, g_app->m_hbao.m_blur_fbo->m_fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	g_app->m_hbao.m_shader_blur->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_app->m_hbao.m_fbo->m_attachments[0].m_texture);
	g_app->m_hbao.m_shader_blur->set("fAO", 0);
	g_app->m_hbao.m_shader_blur->set("sigma", 3.0f);
	g_app->m_hbao.m_shader_blur->set("direction", 0);

	glBindVertexArray(g_app->m_quad_vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_app->m_quad_index.size()), GL_UNSIGNED_INT, 0);

	// apply vertical blur
	glBindFramebuffer(GL_FRAMEBUFFER, g_app->m_hbao.m_fbo->m_fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	g_app->m_hbao.m_shader_blur->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_app->m_hbao.m_blur_fbo->m_attachments[0].m_texture);
	g_app->m_hbao.m_shader_blur->set("fAO", 0);
	g_app->m_hbao.m_shader_blur->set("sigma", 3.0f);
	g_app->m_hbao.m_shader_blur->set("direction", 1);

	glBindVertexArray(g_app->m_quad_vao);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_app->m_quad_index.size()), GL_UNSIGNED_INT, 0);

	// reset
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

void set_deferred_shader()
{
	int activeScene = g_app->m_ui.m_scene;

	g_app->m_shader_deferred->use();
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_app->m_g_fbo->m_attachments[0].m_texture);
	g_app->m_shader_deferred->set("fViewPos", 0);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_app->m_g_fbo->m_attachments[1].m_texture);
	g_app->m_shader_deferred->set("fViewNormal", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_app->m_g_fbo->m_attachments[2].m_texture);
	g_app->m_shader_deferred->set("fAlbedo", 2);
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_app->m_g_fbo->m_attachments[3].m_texture);
	g_app->m_shader_deferred->set("fMetallicRoughness", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_app->m_g_fbo->m_attachments[4].m_texture);
	g_app->m_shader_deferred->set("fDepth", 4);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, g_app->m_directional_shadowMap_fbo->m_attachments[0].m_texture);
	g_app->m_shader_deferred->set("shadowMap", 5);
	g_app->m_shader_deferred->set("shadowMap_bias", g_app->m_ui.shadowMap_bias);

	g_app->m_shader_deferred->set("draw_mode", g_app->m_ui.m_draw_mode);
	g_app->m_shader_deferred->set("ao_mode", g_app->m_ui.m_ao_mode);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, g_app->m_ssao_blurred_fbo->m_attachments[0].m_texture);
	g_app->m_shader_deferred->set("fSSAO", 6);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, g_app->m_hbao.m_fbo->m_attachments[0].m_texture);
	g_app->m_shader_deferred->set("fHBAO", 7);

	g_app->m_shader_deferred->set("cam_near", g_app->m_scene[activeScene].m_cam->getNear());
	g_app->m_shader_deferred->set("cam_far", g_app->m_scene[activeScene].m_cam->getFar());
	g_app->m_shader_deferred->set("lightSpaceMatrix", g_app->m_scene[activeScene].m_directional_light->getLightSpaceMatrix(g_app->m_scene[activeScene].m_cam->getNear(), g_app->m_scene[activeScene].m_cam->getFar()));
	g_app->m_shader_deferred->set("view", g_app->m_scene[activeScene].m_cam->getViewMatrix());

	// set lighting uniforms
	glm::vec3 light_direction = glm::vec3(g_app->m_scene[activeScene].m_cam->getViewMatrix() * glm::vec4(g_app->m_scene[activeScene].m_directional_light->m_direction, 0.0f));
	g_app->m_shader_deferred->set("directionalLight.direction", light_direction);
	g_app->m_shader_deferred->set("directionalLight.color", g_app->m_scene[activeScene].m_directional_light->m_color);
	g_app->m_shader_deferred->set("directionalLight.intensity", g_app->m_scene[activeScene].m_directional_light->m_intensity);
}

int main()
{
	// ==================== INIT GLFW ====================
	// ===================================================
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, "Horizon Based Ambient Occlusion", NULL, NULL);
	if(window == NULL)
	{
		std::cerr << "Failed to create GLFW window !" << std::endl;
		glfwTerminate();
		std::exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// ==================== INIT GLAD ====================
	// ===================================================
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD !" << std::endl;
		std::exit(-1);
	}

	// ==================== OpenGL STATES ====================
	// =======================================================
	glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// ==================== MAIN LOOP ====================
	// ===================================================
	ImGui_init(window);
	initScene();

	while(!glfwWindowShouldClose(window))
	{
		processInput(window);

		// ImGui new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		renderScene();

		// draw UI
		draw_UI();
		if (g_app->m_ssao.m_prev_sample_count != g_app->m_ssao.m_sample_count)
		{
			g_app->m_ssao.m_prev_sample_count = g_app->m_ssao.m_sample_count;
			g_app->m_ssao.populate_samples();
		}
		if (g_app->m_hbao.m_prev_Nd != g_app->m_hbao.m_Nd)
		{
			g_app->m_hbao.m_prev_Nd = g_app->m_hbao.m_Nd;
			g_app->m_hbao.recompute_directions();
		}
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// ==================== CLEANUP ====================
	// =================================================
	glfwTerminate();
	return 0;
}