#define BILLION 1000000000
#define LOWRANGE -2
#define HIGHRANGE 10

#include "core/NoiseGenerator.hpp"

#include <glm/gtc/noise.hpp>

#include <iostream>
#include <random>
using namespace std;

NoiseGenerator::NoiseGenerator(int height, int width) : m_height(height), m_width(width)
{
	std::random_device r;
	std::default_random_engine engine(r());
	std::uniform_int_distribution<int> uniform_dist(1, BILLION);

	m_seeds.resize(m_width * m_height);
	for (int i = 0; i < m_width * m_height; i++)	m_seeds[i] = uniform_dist(engine);
}

NoiseGenerator::NoiseGenerator()
{
}

std::vector<std::vector<float>> NoiseGenerator::GenerateNoiseMap(int octaves, float amplitude)
{
	m_noiseMap.resize(m_width, std::vector<float>(m_height, -1));

	float oldMax = -1;
	float oldMin = BILLION;

	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			float noiseHeight = 0.0f;
			float fscale = 27.6f;
			float scaleAcc = 0.0f;

			float famplitude = amplitude;
			float frequency = 1.0f;

			float newValue = -1;

			for (int i = 0; i < octaves; i++)
			{
				int nPitch = m_width >> i;
				if (nPitch == 0) nPitch = 1;

				int nSampleX1 = ((x / nPitch) * nPitch);
				int nSampleY1 = ((y / nPitch) * nPitch);

				int nSampleX2 = (nSampleX1 + nPitch) % m_width;
				int nSampleY2 = (nSampleY1 + nPitch) % m_width;

				float blendX = ((float)(x - nSampleX1) / (float)nPitch) * frequency;
				float blendY = ((float)(y - nSampleY1) / (float)nPitch) * frequency;

				float sampleT = (1.0f - blendX) * m_seeds[nSampleY1 * m_width + nSampleX1] + blendX * m_seeds[nSampleY1 * m_width + nSampleX2];
				float sampleB = (1.0f - blendX) * m_seeds[nSampleY2 * m_width + nSampleX1] + blendX * m_seeds[nSampleY2 * m_width + nSampleX2];

				scaleAcc += fscale;
				noiseHeight += (blendY * (sampleB - sampleT) + sampleT) * fscale;

				fscale /= 2.0f;

				float value = (noiseHeight / scaleAcc);

				if (oldMax < value) oldMax = value;
				if (oldMin > value) oldMin = value;

				float oldRange = oldMax - oldMin;
				float newRange = (HIGHRANGE - LOWRANGE);
				newValue = (((value - oldMin) * newRange) / oldRange) + LOWRANGE;

				newValue *= famplitude;
			}


			m_noiseMap.at(x).at(y) = newValue;
		}
	}

	return m_noiseMap;
}

float NoiseGenerator::getHeight(int x, int y)
{
	return m_noiseMap.at(x).at(y);
}

void NoiseGenerator::regenerateSeeds()
{
	std::random_device r;
	std::default_random_engine engine(r());
	std::uniform_int_distribution<int> uniform_dist(1, BILLION);

	m_seeds.resize(m_width * m_height);
	for (int i = 0; i < m_width * m_height; i++)	m_seeds[i] = uniform_dist(engine);
}
