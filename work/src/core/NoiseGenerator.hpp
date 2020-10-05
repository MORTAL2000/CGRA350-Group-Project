#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// project
#include "opengl.hpp"

// std
#include <vector>

class NoiseGenerator
{
private:
	int m_seed = -1;
	float m_amplitude = -1;

	std::vector<std::vector<float>> m_noiseMap;

public:
	NoiseGenerator(float amplitude);

	std::vector<std::vector<float>> GenerateNoiseMap(int width, int height);

	float getHeight(int x, int y);
};