
# Source files
set(sources	
	"camera.hpp"
	"camera.cpp"
	"NoiseGenerator.cpp"
	"NoiseGenerator.hpp"

	"NoiseGenerator.hpp"
	"NoiseGenerator.cpp"

	"CMakeLists.txt"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
