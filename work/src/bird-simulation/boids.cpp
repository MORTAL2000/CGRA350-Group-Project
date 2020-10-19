#define GLM_ENABLE_EXPERIMENTAL

// project
#include "opengl.hpp"
#include "boids.hpp"

// glm
#include <iostream>
#include <ostream>
#include <glm/glm.hpp>
#include <glm/detail/func_common.hpp>
#include <glm/ext.hpp>

using namespace glm;

boid::boid()
{
}

void boid::move()
{   
    // Update velocity
    velocity += acceleration;
    velocity = clamp(velocity, -max_speed, max_speed);

    // Update position
    position += velocity;

    // Check horizontal bounds
    /*if (position.x < -horizontal_bounds)
        position.x = horizontal_bounds - boid_size;
    else if (position.x > horizontal_bounds)
        position.x = -horizontal_bounds + boid_size;
    
    if (position.z < -horizontal_bounds)
        position.z = horizontal_bounds - boid_size;
    else if (position.z > horizontal_bounds)
        position.z = -horizontal_bounds + boid_size;*/
    
    // Reset acceleration
    acceleration *= 0;
}

void boid::update(std::vector<boid>& boids, float delta)
{
    vec3 s = separate(boids);
    vec3 a = align(boids);
    vec3 c = cohesion(boids);
    vec3 w = avoid_walls();
    vec3 h = target_height();

    s *= separate_weight;
    a *= alignment_weight;
    c *= cohesion_weight;
    w *= wall_avoidance_weight;
    h *= target_height_weight;

    acceleration += s * delta;
    acceleration += a * delta;
    acceleration += c * delta;
    acceleration += w * delta;
    acceleration += h * delta;
}

vec3 boid::steer(glm::vec3 vec)
{
    vec3 force = normalize(vec) * max_speed;
    return clamp(force, -max_speed, max_speed);
}

vec3 boid::separate(std::vector<boid>& boids)
{
    float seperation_dist = 25.0f;
    vec3 steer = vec3(0);

    int count = 0;
    
    for (boid &other : boids)
    {
        float d = distance(position, other.position);
        if (d > 0 && d < seperation_dist)
        {
            vec3 diff = position - other.position;
            diff = normalize(diff);
            diff /= d;
            steer += diff;
            count++;
        }
    }

    if (count > 0)
    {
        steer /= count;
    }

    if (steer.x + steer.y + steer.z > 0)
    {
        steer = normalize(steer);
        steer *= max_speed;
        steer -= velocity;
        steer = clamp(steer, -max_force, max_force);
    }

    return steer;
}

vec3 boid::align(std::vector<boid>& boids)
{
    float neighbour_dist = 50;
    vec3 sum = vec3(0);
    int count = 0;

    for (boid &other : boids)
    {
        float d = distance(position, other.position);
        if (d > 0 && d < neighbour_dist)
        {
            sum += other.velocity;
            count++;
        }
    }

    if (count > 0)
    {
        sum /= count;
        sum = normalize(sum);
        sum *= max_speed;

        vec3 steer = sum - velocity;
        steer = clamp(steer, -max_force, max_force);
        return steer;
    }
    
    return vec3(0);
}

vec3 boid::cohesion(std::vector<boid>& boids)
{
    float neighbour_dist = 60;
    vec3 sum = vec3(0);
    int count = 0;
    for (boid other : boids)
    {
        float d = distance(position, other.position);
        if (d > 0 && d < neighbour_dist)
        {
            sum += other.position;
            count++;
        }
    }

    if (count > 0)
    {
        sum /= count;
        return seek(sum);
    }

    return vec3(0);
}

vec3 boid::avoid_walls()
{
    vec3 steer = vec3(0);
    int count = 0;

    // TODO: Remove duplicate code
    if (position.x < -horizontal_bounds)
    {
        vec3 norm = vec3(1, 0, 0);
        norm *= max_speed;
        steer += clamp((norm - velocity), -max_force, max_force);
        count++;
    }

    if (position.x > horizontal_bounds)
    {
        vec3 norm = vec3(-1, 0, 0);
        norm *= max_speed;
        steer += clamp((norm - velocity), -max_force, max_force);
        count++;
    }

    if (position.z < -horizontal_bounds)
    {
        vec3 norm = vec3(0, 0, 1);
        norm*=max_speed;
        steer += clamp((norm - velocity), -max_force, max_force);
        count++;
    }

    if (position.z > horizontal_bounds)
    {
        vec3 norm = vec3(0, 0, -1);
        norm*=max_speed;
        steer += clamp((norm - velocity), -max_force, max_force);
        count++;
    }

    if (count > 0)
    {
        steer /= count;
        return steer;
    }

    return vec3(0);
}

vec3 boid::target_height()
{
    float max_dist = 30.0f;
    
    if (position.y < desired_height - max_dist || position.y > desired_height + max_dist)
    {
        vec3 norm = vec3(0, -normalize(position.y - desired_height), 0);
        norm *= max_speed;
        vec3 steer = norm - velocity;
        steer = clamp(steer, -max_force, max_force);
        return steer;
    }

    return vec3(0);
}

vec3 boid::seek(glm::vec3 target)
{
    vec3 dir = target - position;
    dir = normalize(dir);
    dir *= max_speed;
    vec3 steer = dir - velocity;
    steer = clamp(steer, -max_force, max_force);
    return steer;
}
