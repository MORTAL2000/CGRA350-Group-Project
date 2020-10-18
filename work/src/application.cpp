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


void basic_model::draw(const glm::mat4 &view, const glm::mat4 proj) {
	mat4 modelview = view * modelTransform;
	
	glUseProgram(shader); // load shader and variables
	glUniformMatrix4fv(glGetUniformLocation(shader, "uProjectionMatrix"), 1, false, value_ptr(proj));
	glUniformMatrix4fv(glGetUniformLocation(shader, "uModelViewMatrix"), 1, false, value_ptr(modelview));
	glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, value_ptr(color));

	mesh.draw(); // draw
}


Application::Application(GLFWwindow *window) : m_window(window) {

	shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
	sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
	GLuint shader = sb.build();

	m_model.shader = shader;
	m_model.color = vec3(1, 0, 0);

	// generate terrain with the default attributes
	updateTerrain();
}



void Application::render() {
	
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


	// draw the model
	m_model.draw(view, proj);
}


void Application::renderGUI() {

	// setup window
	ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiSetCond_Once);
	ImGui::Begin("Options", 0);

	// helpful drawing options
	ImGui::Checkbox("Show axis", &m_show_axis);
	ImGui::SameLine();
	ImGui::Checkbox("Show grid", &m_show_grid);
	ImGui::Checkbox("Wireframe", &m_showWireframe);
	ImGui::SameLine();
	if (ImGui::Button("Screenshot")) rgba_image::screenshot(true);
	ImGui::Text("WASD - Move \nSpacebar/Shift - Ascend/Descend \nQ - Capture/Release Mouse");

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Camera Settings")) {
		ImGui::SliderFloat("Sensitivity", &m_camera.sensitivity, 0, 100, "%.2f");
		ImGui::SliderFloat("Speed", &m_camera.camera_speed, 0, 100, "%.2f");
	}

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Terrain Generation Inputs"))
	{
		ImGui::SliderInt("octaves", &m_octaves, 2, 10, "%1.0f");

		if (ImGui::Button("Update Terrain")) updateTerrain();
	}
	// finish creating window
	ImGui::End();
}


void Application::cursorPosCallback(double xpos, double ypos) {
	if (m_captureMouse) {
		m_camera.Rotate(xpos, ypos, m_windowsize);
		glfwSetCursorPos(m_window, m_windowsize.x / 2, m_windowsize.y / 2);
	}
}


void Application::mouseButtonCallback(int button, int action, int mods) {
	(void)mods; // currently un-used

	// capture is left-mouse down
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		m_leftMouseDown = (action == GLFW_PRESS); // only other option is GLFW_RELEASE
}


void Application::scrollCallback(double xoffset, double yoffset) {
	(void)xoffset, (void)yoffset; // currently un-used
}


void Application::keyCallback(int key, int scancode, int action, int mods) {
	(void)scancode, (void)mods; // currently un-used

	// W Key
	if (key == 87 && action > 0) {
		m_camera.Move(CameraMove::forward);
	}
	// S Key
	else if (key == 83 && action > 0) {
		m_camera.Move(CameraMove::backward);
	}
	// A Key
	else if (key == 65 && action > 0) {
		m_camera.Move(CameraMove::left);
	}
	// D Key
	else if (key == 68 && action > 0) {
		m_camera.Move(CameraMove::right);
	}
	// Shift Key
	else if (key == GLFW_KEY_LEFT_SHIFT && action > 0) {
		m_camera.Move(CameraMove::descend);
	}
	// Space Key
	else if (key == GLFW_KEY_SPACE && action > 0) {
		m_camera.Move(CameraMove::ascend);
	}

	// Mouse capture Q
	if (key == 81 && action == 1) {
		if (!m_captureMouse) {
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


void Application::charCallback(unsigned int c) {
	(void)c; // currently un-used
}

void Application::updateTerrain()
{
	NoiseGenerator ng(70.f);
	int height = 256, width = 256;
	noiseMap = ng.GenerateNoiseMap(width, height, m_octaves, 27.6, 0.5, 2);

	vector<vec3> points;

	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			float value = ng.getHeight(i, j);

			float s = (i / (float)(width - 1));
			float t = (j / (float)(height - 1));

			float x = (s * width) - (width / 2);
			float y = value;
			float z = (t * height) - (height / 2);

			vec3 vertex(x, y, z);
			points.push_back(vertex);
		}
	}

	vector<int> indexBuffer(((width - 1) * (height - 1) * 2) * 3);;
	unsigned int index = 0;
	for (int j = 0; j < height - 1; j++)
	{
		for (int i = 0; i < width - 1; i++)
		{
			int vertexIndex = (j * width) + i;

			// TOP
			indexBuffer[index++] = vertexIndex;
			indexBuffer[index++] = vertexIndex + width + 1;
			indexBuffer[index++] = vertexIndex + 1;

			// BOTTOM
			indexBuffer[index++] = vertexIndex;
			indexBuffer[index++] = vertexIndex + width;
			indexBuffer[index++] = vertexIndex + width + 1;
		}
	}

	vector<vec3> normalBuffer(width * height);
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

	m_model.mesh = mb.build();}
