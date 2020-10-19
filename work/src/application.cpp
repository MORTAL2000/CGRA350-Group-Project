#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  

#include "application.hpp"

// std
#include <iostream>
#include <string>

// glm
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// project
#include "cgra/cgra_geometry.hpp"
#include "cgra/cgra_gui.hpp"
#include "cgra/cgra_image.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_wavefront.hpp"

using namespace std;
using namespace cgra;
using namespace glm;


void basic_model::draw(const glm::mat4& view, const glm::mat4 proj)
{
	mat4 modelview = view * modelTransform;

	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	mesh.draw(); // draw
}

Application::Application(GLFWwindow* window) : m_window(window)
{
	shader_builder sb;
	sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
	GLuint shader = sb.build();

	// both meshes have same shader
	m_terrain.shader = shader;
	m_water.shader = shader; // i do some checks later to give it a really cool box effect tho

	// generate terrain with the default attributes

	m_terrainMap1 = NoiseGenerator(m_height, m_width);
	m_terrainMap2 = NoiseGenerator(m_height, m_width);
	m_waterMap = NoiseGenerator(m_height, m_width);
	waterMap.resize(m_width, std::vector<float>(m_height, 20));
	noiseMap.resize(m_width, std::vector<float>(m_height, -1)); // set the noiseMap vector to correct size but initialse every value -1

	indexBuffer.resize(((m_width - 1) * (m_height - 1) * 2) * 3);
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

	updateTerrain();
	updateMesh(m_water, waterMap);
}

void Application::render()
{
	// retrieve the window hieght
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);

	m_windowsize = vec2(width, height); // update window size
	glViewport(0, 0, width, height); // set the viewport to draw to the entire window

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

	// check time
	chrono::high_resolution_clock::time_point timeCurrent = chrono::high_resolution_clock::now();
	chrono::duration<double, std::milli> timer = timeCurrent - timeStart;

	// only update the terrain if 0.1 seconds has passed since last update
	if (timer.count() >= 100)
	{
		timeStart = chrono::high_resolution_clock::now();
		updateTerrain();
	}

	// rotate model and bob up and down
	m_terrain.modelTransform = rotate(m_terrain.modelTransform, radians(0.1f), vec3(0, 1, 0));
	m_water.modelTransform = rotate(m_terrain.modelTransform, radians(0.1f), vec3(0, 1, 0));


	if (bobbingCount >= 0.3) { shouldGoDown = true; }
	else if (bobbingCount <= -0.3) { shouldGoDown = false; }

	if (shouldGoDown) {
		m_terrain.modelTransform = translate(m_terrain.modelTransform, vec3(0, bobbingCount -= 0.001, 0));
		m_water.modelTransform = translate(m_water.modelTransform, vec3(0, bobbingCount -= 0.001, 0));
	}
	else {
		m_terrain.modelTransform = translate(m_terrain.modelTransform, vec3(0, bobbingCount += 0.001, 0));
		m_water.modelTransform = translate(m_water.modelTransform, vec3(0, bobbingCount += 0.001, 0));
	}

	// draw the model
	m_terrain.draw(view, proj);
	m_water.draw(view, proj);
}


void Application::renderGUI()
{

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

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
		if (ImGui::SliderFloat("m_exponent", &m_exponent, 0.5, 1.5, "%0.5f"))			m_needsUpdating = true;
		if (ImGui::SliderFloat("Base Bias", &bias1, 0, 2, "%0.5f"))			m_needsUpdating = true;
		if (ImGui::SliderFloat("Secondary Bias", &bias2, 0, 2, "%0.5f"))			m_needsUpdating = true;
		//if (ImGui::SliderFloat("Water", &bias3, 0, 2, "%0.5f"))			m_needsUpdating = true;

		if (ImGui::Button("New Seed"))
		{
			m_terrainMap1.regenerateSeeds();
			m_terrainMap2.regenerateSeeds();

			m_needsUpdating = true;
		}
	}
	// finish creating window
	ImGui::End();
}

void Application::updateTerrain()
{
	if (m_needsUpdating)
	{
		std::vector<std::vector<float>> noiseMap1 = m_terrainMap1.GenerateNoiseMap(m_octaves, m_amplitude, m_scale, m_persistance);
		std::vector<std::vector<float>> noiseMap2 = m_terrainMap2.GenerateNoiseMap(m_octaves, m_amplitude, m_scale, m_persistance);


		for (int j = 0; j < m_height; j++)
		{
			for (int i = 0; i < m_width; i++)
			{
				int J = floor(j / m_scale);
				int I = floor(i / m_scale);

				float value = (noiseMap1.at(J).at(I) * bias1) + (noiseMap2.at(J).at(I) * bias2);
				value = pow(value, m_exponent);

				noiseMap.at(j).at(i) = value;
			}
		}

		for (int j = 0; j < m_height - 1; j++)
		{
			for (int i = 0; i < m_width - 1; i++)
			{
				if (j == 0 || i == 0) {
					waterMap[j][i] = 20;
				}
				else {
					float compare = noiseMap[j][i];
					if (compare >= 20) {
						waterMap[j][i] = compare - 0.1;
					}
					else {
						waterMap[j][i] = 20;
					}
				}
			}
		}

		updateMesh(m_terrain, noiseMap);
		updateMesh(m_water, waterMap);


		m_needsUpdating = false;
	}
}


void Application::updateMesh(basic_model& model, std::vector<std::vector<float>> map)
{
	vector<vec3> points;

	for (int j = 0; j < m_height; j++)
	{
		for (int i = 0; i < m_width; i++)
		{
			float value = map.at(j).at(i);

			float s = (i / (float)(m_width - 1));
			float t = (j / (float)(m_height - 1));

			float x = (s * m_width) - (m_width / 2);
			float y = value;
			float z = (t * m_height) - (m_height / 2);

			vec3 vertex(x, y, z);
			points.push_back(vertex);
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

	model.mesh = mb.build();
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
