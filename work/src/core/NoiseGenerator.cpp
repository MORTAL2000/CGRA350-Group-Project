#include "NoiseGenerator.hpp"

// std
#include <random>
#include <iostream>

using namespace std;

NoiseGenerator::NoiseGenerator(int height, int width) : m_height(height), m_width(width)
{
	regenerateSeeds();
	cout << "created objected! " << endl;

	m_noiseMap.resize(m_width, std::vector<float>(m_height, -1));
}

NoiseGenerator::NoiseGenerator()
{
	this->~NoiseGenerator();
}

NoiseGenerator::~NoiseGenerator()
{
	cout << "destroyed object! " << this << endl;
}

std::vector<std::vector<float>> NoiseGenerator::GenerateNoiseMap(int octaves, float amplitude, float scale, float persistance)
{

	float oldMax = 1.0e+09;
	float oldMin = 1.0e+09;
	amplitude *= persistance;

	float HIGHRANGE = 10 * amplitude;
	float LOWRANGE = -1 * amplitude;

	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			float noiseHeight = 0.0f;
			float fscale = 1;

			float frequency = 1.0f;

			float value = -1;

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

				noiseHeight += (blendY * (sampleB - sampleT) + sampleT) * fscale;

				if (oldMax < noiseHeight)
				{
					oldMax = noiseHeight;
				}
				if (oldMin > noiseHeight)
				{
					oldMin = noiseHeight;
				}

				float oldRange = oldMax - oldMin;
				float newRange = (HIGHRANGE - LOWRANGE);

				fscale /= 2.0f;
				value *= amplitude;
				amplitude *= persistance;

				value = ((noiseHeight - oldMin) / (oldMax - oldMin)) * (HIGHRANGE - LOWRANGE) + LOWRANGE;

			}

			m_noiseMap.at(x).at(y) = value;
		}
	}

	return m_noiseMap;
}

void NoiseGenerator::regenerateSeeds()
{
	std::random_device r;
	std::default_random_engine engine(r());
	std::uniform_int_distribution<int> uniform_dist(1, 1.0e+09);

	m_seeds.resize(m_width * m_height);
	for (int i = 0; i < m_width * m_height; i++)	m_seeds[i] = uniform_dist(engine);
}
