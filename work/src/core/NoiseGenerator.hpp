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
	float m_amplitude = -1;

	std::vector<std::vector<float>> m_noiseMap;
	std::vector<float> m_seeds;

public:
	NoiseGenerator(float amplitude);

	std::vector<std::vector<float>> GenerateNoiseMap(int width, int height, int octaves, float scale, float persistance, float lacunarity);

	float getHeight(int x, int y);
};