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

    boids.resize(5);
    for (boid &boid : boids)
    {
        boid.position = vec3(rand() % 10 + 20, rand() % 100 + 25, rand() % 10 + 20);
        boid.velocity = vec3(rand() % 2 - 1 * (boid.max_speed/2), 0, rand() % 2 - 1 * (boid.max_speed/2));
        cout << "Spawned boid at " << boid.position.x << ", " << boid.position.y << ", " << boid.position.z << endl;
    }

    target_position_ = vec3(rand() % 100, rand() % 40 + 10, rand() % 100);
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
    
    for (boid &boid : boids)
    {
        transform = translate(transform, boid.position) * orientation(normalize(boid.velocity), vec3(0, 0, 1));
        model_.draw(view * transform, proj);
        
        transform = mat4(1);
    }
}

void blukzen::boids_renderer::update_boids()
{
    for (boid &boid : boids)
    {
        boid.update(boids, m_deltaTime);
        boid.move();
    }
}

void blukzen::boids_renderer::apply_boid_rules(boid& b)
{
}
