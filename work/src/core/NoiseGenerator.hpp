#pragma once

// glm
#include <glm/glm.hpp>

// std
#include <vector>

class NoiseGenerator
{
private:
	std::vector<std::vector<float>> m_noiseMap;
	std::vector<float> m_seeds;

	int m_width = -1, m_height = -1;

public:
	NoiseGenerator(int height, int width); // constructor with arguments
	NoiseGenerator(); // default constructor
	~NoiseGenerator();

	std::vector<std::vector<float>> GenerateNoiseMap(int octaves, float amplitude, float scale, float persistance);


	void regenerateSeeds();
};