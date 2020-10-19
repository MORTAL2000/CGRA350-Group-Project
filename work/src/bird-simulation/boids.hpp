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
struct boid {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration = glm::vec3(0);
    
    float max_speed = 0.2;
    float max_force = 0.05;

    glm::vec3 direction;
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 vel = glm::vec3(0);
    glm::vec3 flock_center;
    int flock_size = 0;
    float target_y;
    float max_turn_speed = 3;
    float max_vertical_angle = 20;
    int pitch = 180;
    int yaw = 0;
    
    const float alignment_weight = 1.0;
    const float cohesion_weight = 1.0;
    const float separate_weight = 1.5;
    
    boid();
    void move();
    void update(std::vector<boid>& boids);
    glm::vec3 steer(glm::vec3 vec);
    glm::vec3 seperate(std::vector<boid>& boids);
    glm::vec3 align(std::vector<boid>& boids);
    glm::vec3 cohesion(std::vector<boid>& boids);
    glm::vec3 seek(glm::vec3 target);
};
