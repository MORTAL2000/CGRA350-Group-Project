#define BILLION 1000000000

#include "core/NoiseGenerator.hpp"

#include <iostream>
#include <random>

NoiseGenerator::NoiseGenerator(float amplitude) : m_amplitude(amplitude)
{
	// Set seed as a random number between 1 and 1,000,000,000
	std::random_device r;
	std::default_random_engine engine(r());
	std::uniform_int_distribution<int> uniform_dist(1, BILLION);
	m_seed = uniform_dist(engine); // set the seed
}

std::vector<std::vector<float>> NoiseGenerator::GenerateNoiseMap(int width, int height)
{

	m_noiseMap.resize(width, std::vector<float>(height, -1));

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			float perlinValue = 1 * x * y;
			m_noiseMap.at(x).at(y) = perlinValue;
		}
	}

	return m_noiseMap;
}

float NoiseGenerator::getHeight(int x, int y)
{
	return m_noiseMap.at(x).at(y);
}
