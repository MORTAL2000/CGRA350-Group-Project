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

    target_position_ = vec3(rand() % 10 + 20, rand() % 40 + 10, rand() % 10 + 20);

    flocks.resize(initial_flocks);
    for (flock &flock : flocks)
    {
        flock.boids.resize(rand() % 3 + 5);

        for (boid &boid : flock.boids)
        {
            boid.position = vec3(0, rand() % 10 + 90, 0);
            boid.velocity = vec3(rand() % 2 - 1 * (0.1), 0, rand() % 2 - 1 * (0.1));
        }
    }
}

blukzen::boids_renderer::~boids_renderer()
{
}

void blukzen::boids_renderer::update(const mat4& view, const mat4 proj)
{
    // Update current frame
    float currentFrame = glfwGetTime();
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;
    
    update_boids();
    render_boids(view, proj);
}

void blukzen::boids_renderer::render_boids(const mat4& view, const mat4 proj)
{
    mat4 transform(1);

    for (flock &flock : flocks)
    {
        for (boid &boid : flock.boids)
        {
            transform = translate(transform, boid.position) * orientation(normalize(boid.velocity), vec3(0, 0, 1)) * scale(mat4(1), vec3(0.5, 0.5, 0.5));
            model_.draw(view * transform, proj);
        
            transform = mat4(1);
        }
    }
}

void blukzen::boids_renderer::update_boids()
{
    for (flock &flock : flocks)
    {
        for (boid &boid : flock.boids)
        {
            // TODO: Move parameter updates to an event.
            boid.alignment_weight = alignment_weight;
            boid.cohesion_weight = cohesion_weight;
            boid.separate_weight = separate_weight;
            boid.target_height_weight = target_height_weight;
            boid.max_speed = max_speed;
            boid.max_force = max_force;
            boid.desired_height = desired_height;
            boid.update(flock.boids, m_deltaTime);
            boid.move();
        }
    }
}
