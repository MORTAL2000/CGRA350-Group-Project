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
	basic_model m_terrain;
	basic_model m_water;

	// Terrain Properties
	std::vector<std::vector<float>> noiseMap;
	std::vector<std::vector<float>> waterMap;
	std::vector<int> indexBuffer;

	int m_octaves = 4;
	float m_amplitude = 3.f;
	float m_scale = 1.f;
	float m_persistance = 1;
	float m_exponent = 1.f;
	// Weight that each heightmap should carry
	float bias1 = 1.5f, bias2 = 1.0f, bias3 = 0.15f;

	// Height and Width of map - Should be the same number (should make a square)
	int m_height = 256, m_width = 256;

	// Actual Noise Generator objects that creates the noise height maps
	NoiseGenerator m_terrainMap1, m_terrainMap2, m_waterMap;

	// Rendering
	bool m_needsUpdating = true; // if the model should be updated (changing values on the heightmap) - changing actual position of the mesh should not enable this
	bool shouldGoDown = true; // for bobbing the model mesh up and down
	float bobbingCount = 0; // keeps track of when to switch between up and down when bobbing

	// Time
	// only update the mesh once every 0.1 seconds (can be changed)
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

	// update mesh
	void updateMesh(basic_model &model, std::vector<std::vector<float>> map);
};