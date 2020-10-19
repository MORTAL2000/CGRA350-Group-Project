#define GLM_ENABLE_EXPERIMENTAL

// project
#include "opengl.hpp"
#include "boids.hpp"

// glm
#include <iostream>
#include <ostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/detail/func_common.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

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

    // Reset acceleration
    acceleration *= 0;
}

void boid::update(std::vector<boid>& boids)
{
    vec3 s = seperate(boids);
    vec3 a = align(boids);
    vec3 c = cohesion(boids);

    s *= separate_weight;
    a *= alignment_weight;
    c *= cohesion_weight;

    acceleration += s;
    acceleration += a;
    acceleration += c;
}

vec3 boid::steer(glm::vec3 vec)
{
    vec3 force = normalize(vec) * max_speed;
    return clamp(force, -max_speed, max_speed);
}

vec3 boid::seperate(std::vector<boid>& boids)
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
    float neighbour_dist = 50;
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

vec3 boid::seek(glm::vec3 target)
{
    vec3 dir = target - position;
    dir = normalize(dir);
    dir *= max_speed;
    vec3 steer = dir - velocity;
    steer = clamp(steer, -max_force, max_force);
    return steer;
}
