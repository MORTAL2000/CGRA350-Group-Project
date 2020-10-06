#define BILLION 1000000000

#include "core/NoiseGenerator.hpp"

#include <glm/gtc/noise.hpp>

#include <iostream>
#include <random>
using namespace std;

NoiseGenerator::NoiseGenerator(float amplitude) : m_amplitude(amplitude)
{
	// Set seed as a random number between 1 and 1,000,000,000

}

std::vector<std::vector<float>> NoiseGenerator::GenerateNoiseMap(int width, int height, int octaves, float scale, float persistance, float lacunarity)
{
	std::random_device r;
	std::default_random_engine engine(r());
	std::uniform_int_distribution<int> uniform_dist(1, BILLION);
	
	m_seeds.resize(width * height);
	for(int i = 0; i < width * height; i++)	m_seeds[i] = uniform_dist(engine);

	m_noiseMap.resize(width, std::vector<float>(height, -1));

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			float noiseHeight = 0.0f;
			float scale = 100.0f;
			float scaleAcc = 0.0f;

			for (int i = 0; i < octaves; i++)
			{
				int nPitch = width >> i;
				if (nPitch == 0) nPitch = 1;
				int nSampleX1 = (x / nPitch) * nPitch;
				int nSampleY1 = (y / nPitch) * nPitch;

				int nSampleX2 = (nSampleX1 + nPitch) % width;
				int nSampleY2 = (nSampleY1 + nPitch) % width; 

				float blendX = (float)(x - nSampleX1) / (float)nPitch;
				float blendY = (float)(y - nSampleY1) / (float)nPitch;

				float sampleT = (1.0f - blendX) * m_seeds[nSampleY1 * width + nSampleX1] + blendX * m_seeds[nSampleY1 * width + nSampleX2];
				float sampleB = (1.0f - blendX) * m_seeds[nSampleY2 * width + nSampleX1] + blendX * m_seeds[nSampleY2 * width + nSampleX2];

				scaleAcc += scale;
				noiseHeight += (blendY * (sampleB - sampleT) + sampleT) * scale;
				scale /= 2.0f;
			}
			//cout << noiseHeight / scaleAcc / 10000000 << endl;
			m_noiseMap.at(x).at(y) = noiseHeight / scaleAcc / 10000000;

		}
	}

	return m_noiseMap;
}

float NoiseGenerator::getHeight(int x, int y)
{
	return m_noiseMap.at(x).at(y);
}
