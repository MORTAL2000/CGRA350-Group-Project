
#pragma once
// std
#include <vector>
#include <chrono>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"
#include "cgra/cgra_mesh.hpp"
#include "skeleton_model.hpp"
#include "core/camera.hpp"
#include "core/NoiseGenerator.hpp"

// Basic model that holds the shader, mesh and transform for drawing.
// Can be copied and modified for adding in extra information for drawing
// including textures for texture mapping etc.
struct basic_model
{
	GLuint shader = 0;
	cgra::gl_mesh mesh;
	glm::vec3 color{ 0.7 };
	glm::mat4 modelTransform{ 1.0 };
	GLuint texture;

	void draw(const glm::mat4& view, const glm::mat4 proj);
};


// Main application class
//
class Application
{
private:
	// window
	glm::vec2 m_windowsize;
	GLFWwindow* m_window;

	Camera m_camera;
	bool m_captureMouse;

	// last input
	bool m_leftMouseDown = false;
	glm::vec2 m_mousePosition;

	// drawing flags
	bool m_show_axis = false;
	bool m_show_grid = false;
	bool m_showWireframe = false;

	// geometry
	basic_model m_model;

	// terrain generation
	std::vector<std::vector<float>> noiseMap;
	int m_octaves = 4;
	float m_amplitude = 2.f;
	float m_scale = 1.f;
	float m_persistance = 1;
	float m_exponent = 1.0f;
	float bias1 = 1.5f, bias2 = 1.0f, bias3 = 0.15f;

	int m_height = 256, m_width = 256;
	NoiseGenerator m_ng1, m_ng2, m_ng3;
	float count = 0;

	//rendering
	bool shouldGoDown = true;
	bool m_needsUpdating = true;
	std::chrono::high_resolution_clock::time_point timeStart = std::chrono::high_resolution_clock::now();

public:
	// setup
	Application(GLFWwindow*);

	// disable copy constructors (for safety)
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	// rendering callbacks (every frame)
	void render();
	void renderGUI();

	// input callbacks
	void cursorPosCallback(double xpos, double ypos);
	void mouseButtonCallback(int button, int action, int mods);
	void scrollCallback(double xoffset, double yoffset);
	void keyCallback(int key, int scancode, int action, int mods);
	void charCallback(unsigned int c);

	// terrain generation
	void updateTerrain();
};