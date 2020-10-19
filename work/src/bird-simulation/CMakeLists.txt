
# Source files
set(sources	
	"boids.hpp"
	"boids.cpp"

	"boids_model.hpp"
	"boids_model.cpp"

	"boids_bone.hpp"
	"boids_bone.cpp"

	"boids_renderer.hpp"
	"boids_renderer.cpp"

	"CMakeLists.txt"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
