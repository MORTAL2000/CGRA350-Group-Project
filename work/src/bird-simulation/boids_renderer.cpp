#define GLM_ENABLE_EXPERIMENTAL

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

// opengl
#include "opengl.hpp"

// project
#include "boids_renderer.hpp"

#include "glm/gtx/rotate_vector.hpp"


using namespace glm;
using namespace cgra;

blukzen::boids_renderer::boids_renderer()
{
    model_.load_model(CGRA_SRCDIR + std::string("//res//assets//bird.dae"));

    boids.resize(10);
    for (boid &boid : boids)
    {
        boid.position = vec3(rand() % 10 + 100, rand() % 40 + 25, rand() % 10 + 100);
        cout << "Spawned boid at " << boid.position.x << ", " << boid.position.y << ", " << boid.position.z << endl;
    }

    target_position_ = vec3(rand() % 100, rand() % 40 + 10, rand() % 100);
}

blukzen::boids_renderer::~boids_renderer()
{
}

void blukzen::boids_renderer::update(const mat4& view, const mat4 proj)
{
    update_boids();
    render_boids(view, proj);
}

void blukzen::boids_renderer::render_boids(const mat4& view, const mat4 proj)
{
    mat4 transform(1);
    
    for (boid &boid : boids)
    {
        transform = translate(transform, boid.position);
        model_.draw(view * transform, proj);
        
        transform = mat4(1);
    }
}

void blukzen::boids_renderer::update_boids()
{
    vec3 flock_center(0);
    for (boid &boid : boids)
    {
        flock_center += boid.position;
    }
    
    for (boid &boid : boids)
    {
        boid.flock_center = flock_center;
        boid.flock_size = boids.size();
        boid.update(boids);
        boid.move();
    }
}

void blukzen::boids_renderer::apply_boid_rules(boid& b)
{
}
