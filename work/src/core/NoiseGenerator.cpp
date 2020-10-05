#define BILLION 1000000000

#include "core/NoiseGenerator.hpp"

#include <glm/gtc/noise.hpp>

#include <iostream>
#include <random>
using namespace std;

NoiseGenerator::NoiseGenerator(float amplitude) : m_amplitude(amplitude)
{
	// Set seed as a random number between 1 and 1,000,000,000
	std::random_device r;
	std::default_random_engine engine(r());
	std::uniform_int_distribution<int> uniform_dist(1, BILLION);
	m_seed = uniform_dist(engine); // set the seed
}

std::vector<std::vector<float>> NoiseGenerator::GenerateNoiseMap(int width, int height, int octaves, float scale, float persistance, float lacunarity)
{

	m_noiseMap.resize(width, std::vector<float>(height, -1));

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			float amplitude = 1.f;
			float frequency = 1.f;
			float noiseHeight = 0.f;
			for (int i = 0; i < octaves; i++)
			{
				float sampleX = x / scale * frequency;
				float sampleY = y / scale * frequency;

				float value = glm::perlin(glm::vec2(x, y)) * 2 - 1;
				noiseHeight += value * amplitude;

				amplitude *= persistance;
				frequency *= lacunarity;
			}
			m_noiseMap.at(x).at(y) = noiseHeight;

		}
	}

	return m_noiseMap;
}

float NoiseGenerator::getHeight(int x, int y)
{
	return m_noiseMap.at(x).at(y);
}
