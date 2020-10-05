
# Source files
set(sources	
	"boids.hpp"
	"boids.cpp"

	"CMakeLists.txt"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
