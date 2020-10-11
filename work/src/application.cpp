#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/gtx/string_cast.hpp> 

#define BILLION 1000000000
#define LOWRANGE -2
#define HIGHRANGE 10

// std
#include <iostream>
#include <string>
#include <chrono>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "application.hpp"
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"

#include "core/camera.hpp"
#include "core/NoiseGenerator.hpp"

using namespace std;
using namespace cgra;
using namespace glm;


void basic_model::draw(const glm::mat4& view, const glm::mat4 proj)
{
	mat4 modelview = view * modelTransform;

	////glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	//glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));
	mesh.draw(); // draw
}


Application::Application(GLFWwindow* window) : m_window(window)
{
	shader_builder sb;
	shader_builder depth;
	depth.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//shadow_vert.glsl"));
	depth.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//shadow_frag.glsl"));
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert2.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag2.glsl"));
	GLuint depthShader = depth.build();
	GLuint shader = sb.build();

	DepthShader = depth.build();
	Shader = sb.build();

	m_model.shader = shader;
	m_model.depthshader = depthShader;
	m_model.color = vec3(1, 0, 0);

	// generate terrain with the default attributes

	m_ng1 = NoiseGenerator(m_height, m_width);
	m_ng2 = NoiseGenerator(m_height, m_width);
	m_ng3 = NoiseGenerator(m_height, m_width);
	noiseMap.resize(m_width, std::vector<float>(m_height, -1)); // set the noiseMap vector to correct size but every value -1

	updateTerrain();


	// set up buffers for shadows

	glGenFramebuffers(1, &FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, map_size, map_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderCol[] = { 1.0f,1.0f,1.0f,1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderCol);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0);
	glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Something went wrong with the buffer" << std::endl;
	}

	depthMatrixID = glGetUniformLocation(DepthShader, "depthMVP");
	MatrixID = glGetUniformLocation(Shader, "MVP");
	DepthBiasID = glGetUniformLocation(Shader, "DepthBiasMVP");
	ShadowMapID = glGetUniformLocation(Shader, "shadowMap");
}




void Application::render()
{

	// retrieve the window hieght
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	m_windowsize = vec2(width, height); // update window size
	//glViewport(0, 0, width, height); // set the viewport to draw to the entire window

	// clear the back-buffer
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// enable flags for normal/forward rendering
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// projection matrix
	mat4 proj = perspective(1.f, float(width) / height, 0.1f, 1000.f);

	// view matrix
	mat4 view = m_camera.Update();

	// helpful draw options
	if (m_show_grid) drawGrid(view, proj);
	if (m_show_axis) drawAxis(view, proj);
	glPolygonMode(GL_FRONT_AND_BACK, (m_showWireframe) ? GL_LINE : GL_FILL);


	updateTerrain();

	// rotate model and bob up and down
	m_model.modelTransform = rotate(m_model.modelTransform, radians(0.1f), vec3(0, 1, 0));

	if (count >= 0.3) { shouldGoDown = true; }
	else if (count <= -0.3) { shouldGoDown = false; }

	if (shouldGoDown) { m_model.modelTransform = translate(m_model.modelTransform, vec3(0, count -= 0.001, 0)); }
	else { m_model.modelTransform = translate(m_model.modelTransform, vec3(0, count += 0.001, 0)); }


	// shadow mapping
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);
	glViewport(0, 0, map_size, map_size);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(DepthShader);

	vec3 lightInvDir = vec3(80.f,20.f,100.f);
	float s = 15;
	float near_plane = -0.0f * s, far_plane = 30.0f * s;
	mat4 depthProjectionMatrix = ortho(-10.0f * s, 10.0f * s, -10.0f * s, 10.0f * s, near_plane, far_plane);

	mat4 depthViewMatrix = lookAt(lightInvDir, vec3(0, 0, 0), vec3(0, 1, 0));
	mat4 depthModelMatrix = mat4(1.0);
	//mat4 depthModelMatrix = m_model.modelTransform;
	mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

	glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVP[0][0]);

	m_model.draw(depthViewMatrix, depthProjectionMatrix); // render to buffer

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(Shader);
	//mat4 modelview = view * modelTransform;
	mat4 MVP = proj * view * m_model.modelTransform;

	mat4 biasMatrix(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
	mat4 depthBiasMVP = biasMatrix * depthMVP;

	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(DepthBiasID, 1, GL_FALSE, &depthBiasMVP[0][0]);
	glUniform3fv(glGetUniformLocation(Shader, "lightdirection"), 1, value_ptr(lightInvDir));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glUniform1i(ShadowMapID, 0);
	//

	// draw the model
	m_model.draw(view, proj);
}


void Application::renderGUI()
{

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);
	ImGui::Text("Application %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);
	ImGui::Text("WASD - Move \nQ - Capture/Release Mouse");

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Camera Settings"))
	{
		ImGui::SliderFloat("Sensitivity", &m_camera.sensitivity, 0, 100, "%.2f");
		ImGui::SliderFloat("Speed", &m_camera.camera_speed, 0, 100, "%.2f");
	}

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Terrain Generation Inputs"))
	{
		if (ImGui::SliderInt("octaves", &m_octaves, 2, 9, "%1.0f"))		m_needsUpdating = true;
		if (ImGui::SliderFloat("amplitude", &m_amplitude, 1, 5, "%0.5f"))	m_needsUpdating = true;
		if (ImGui::SliderFloat("scale", &m_scale, 1, 3, "%0.5f"))			m_needsUpdating = true;
		if (ImGui::SliderFloat("persistance", &m_persistance, 0.5, 1.5, "%0.5f"))	m_needsUpdating = true;
		if (ImGui::SliderFloat("m_exponent", &m_exponent, 0.1, 2, "%0.5f"))			m_needsUpdating = true;
		if (ImGui::SliderFloat("bias1", &bias1, 0, 2, "%0.1f"))			m_needsUpdating = true;
		if (ImGui::SliderFloat("bias2", &bias2, 0, 2, "%0.1f"))			m_needsUpdating = true;
		if (ImGui::SliderFloat("bias3", &bias3, 0, 2, "%0.1f"))			m_needsUpdating = true;

		if (ImGui::Button("Update Terrain")) updateTerrain();
		ImGui::SameLine();
		if (ImGui::Button("New Seed"))
		{
			m_ng1.regenerateSeeds();
			m_ng2.regenerateSeeds();
			m_ng3.regenerateSeeds();
			m_needsUpdating = true;
		}
	}
	// finish creating window
	ImGui::End();
}


void Application::cursorPosCallback(double xpos, double ypos)
{
	if (m_captureMouse)
	{
		m_camera.Rotate(xpos, ypos, m_windowsize);
		glfwSetCursorPos(m_window, m_windowsize.x / 2, m_windowsize.y / 2);
	}
}


void Application::mouseButtonCallback(int button, int action, int mods)
{
	(void)mods; // currently un-used

	// capture is left-mouse down
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
}


void Application::scrollCallback(double xoffset, double yoffset)
{
	(void)xoffset, (void)yoffset; // currently un-used
}


void Application::keyCallback(int key, int scancode, int action, int mods)
{
	(void)scancode, (void)mods; // currently un-used

	// W Key
	if (key == 87 && action > 0)
	{
		m_camera.Move(CameraMove::forward);
	}
	// S Key
	else if (key == 83 && action > 0)
	{
		m_camera.Move(CameraMove::backward);
	}
	// A Key
	else if (key == 65 && action > 0)
	{
		m_camera.Move(CameraMove::left);
	}
	// D Key
	else if (key == 68 && action > 0)
	{
		m_camera.Move(CameraMove::right);
	}

	// Mouse capture Q
	if (key == 81 && action == 1)
	{
		if (!m_captureMouse)
		{
			//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			m_captureMouse = true;
		}
		else
		{
			//glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			m_captureMouse = false;
		}
	}
}


void Application::charCallback(unsigned int c)
{
	(void)c; // currently un-used
}

void Application::updateTerrain()
{
	if (m_needsUpdating)
	{
		std::vector<std::vector<float>> noiseMap1 = m_ng1.GenerateNoiseMap(m_octaves, m_amplitude, m_scale, m_persistance);
		std::vector<std::vector<float>> noiseMap2 = m_ng2.GenerateNoiseMap(m_octaves, m_amplitude, m_scale, m_persistance);
		std::vector<std::vector<float>> noiseMap3 = m_ng3.GenerateNoiseMap(m_octaves, m_amplitude, m_scale, m_persistance);

		for (int j = 0; j < m_height; j++)
		{
			for (int i = 0; i < m_width; i++)
			{

				int J = floor(j / m_scale);
				int I = floor(i / m_scale);

				float value = (noiseMap1.at(J).at(I) * bias1) + (noiseMap2.at(J).at(I) * bias2) + (noiseMap3.at(J).at(I) * bias3);
				value = pow(value, m_exponent);

				value = (value - (-1.f)) / 2.f;
				//if(i % 200 == 0)
				//cout << i<<'/'<<j<<'\t'<<value << endl;
				noiseMap.at(j).at(i) = value;
			}
		}

		//noiseMap = m_ng.GenerateNoiseMap(m_octaves, m_amplitude, m_scale, m_persistance);
		vector<vec3> points;

		for (int j = 0; j < m_height; j++)
		{
			for (int i = 0; i < m_width; i++)
			{
				float value = noiseMap.at(j).at(i);

				float s = (i / (float)(m_width - 1));
				float t = (j / (float)(m_height - 1));

				float x = (s * m_width) - (m_width / 2);
				float y = value;
				float z = (t * m_height) - (m_height / 2);

				vec3 vertex(x, y, z);
				points.push_back(vertex);
			}
		}

		vector<int> indexBuffer(((m_width - 1) * (m_height - 1) * 2) * 3);;
		unsigned int index = 0;
		for (int j = 0; j < m_height - 1; j++)
		{
			for (int i = 0; i < m_width - 1; i++)
			{
				int vertexIndex = (j * m_width) + i;

				// TOP
				indexBuffer[index++] = vertexIndex;
				indexBuffer[index++] = vertexIndex + m_width + 1;
				indexBuffer[index++] = vertexIndex + 1;

				// BOTTOM
				indexBuffer[index++] = vertexIndex;
				indexBuffer[index++] = vertexIndex + m_width;
				indexBuffer[index++] = vertexIndex + m_width + 1;
			}
		}

		vector<vec3> normalBuffer(m_width * m_height);
		for (int i = 0; i < indexBuffer.size(); i += 3)
		{
			vec3 v0 = points[indexBuffer[i + 0]];
			vec3 v1 = points[indexBuffer[i + 1]];
			vec3 v2 = points[indexBuffer[i + 2]];

			vec3 normal = normalize(cross(v1 - v0, v2 - v0));

			normalBuffer[indexBuffer[i + 0]] += normal;
			normalBuffer[indexBuffer[i + 1]] += normal;
			normalBuffer[indexBuffer[i + 2]] += normal;
		}

		mesh_builder mb;
		unsigned int count = 0;
		for (vec3 point : points)
		{
			mesh_vertex v;
			v.pos = point;
			v.norm = normalBuffer[count];
			mb.push_vertex(v);
			count++;
		}

		for (int i = 0; i < indexBuffer.size() - 1; i++)
		{
			mb.push_index(indexBuffer.at(i));
		}

		m_model.mesh = mb.build();

		m_needsUpdating = false;
	}
}