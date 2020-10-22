#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/detail/type_vec.hpp>

// project
#include <vector>

#include "opengl.hpp"

/**
*
*
*/
struct bounds
{
    glm::vec3 max;
    glm::vec3 min;
};

struct boid {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration = glm::vec3(0);

    float max_y = 0.2;
    float max_speed = 0.6;
    float max_force = 0.1;

    float desired_height = 100;
    float ceiling_height = 150;
    float floor_height = 40;
    float horizontal_bounds = 180;
    float boid_size = 2;

    bounds bounds{
        glm::vec3(horizontal_bounds, ceiling_height, horizontal_bounds),
        glm::vec3(-horizontal_bounds, floor_height, -horizontal_bounds)
    };
    
    float alignment_weight = 0.5;
    float cohesion_weight = 0.7;
    float separate_weight = 1;
    float wall_avoidance_weight = 1;
    float target_height_weight = 1;

    float m_deltaTime = 0.0f;
    float m_lastFrame = 0.0f;
    
    boid();
    void move();
    void update(std::vector<boid>& boids, float delta);
    glm::vec3 steer(glm::vec3 vec);
    glm::vec3 separate(std::vector<boid>& boids);
    glm::vec3 align(std::vector<boid>& boids);
    glm::vec3 cohesion(std::vector<boid>& boids);
    glm::vec3 avoid_walls();
    glm::vec3 target_height();
    glm::vec3 seek(glm::vec3 target);
};
