#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// project
#include "cgra/cgra_mesh.hpp"
#include "opengl.hpp"
#include <iostream>


#include "boids.hpp"
#include "boids_model.hpp"

using namespace glm;
using namespace std;
using namespace cgra;

namespace blukzen
{
    struct flock
    {
        vector<boid> boids;
    };
    
    class boids_renderer {
        boids_model model_;
        vec3 target_position_;

        float m_deltaTime = 0.0f;
        float m_lastFrame = 0.0f;

        int initial_flocks = 6;
        int flock_size = 6;

    public:
        vector<boid> boids; // Boids array
        vector<flock> flocks;

        float alignment_weight = 0.5;
        float cohesion_weight = 0.7;
        float separate_weight = 1;
        float target_height_weight = 1;
        float desired_height = 100;
        float max_speed = 0.6;
        float max_force = 0.1;
        
        boids_renderer();
        ~boids_renderer();
        void update(const mat4& view, const mat4 proj);
        void render_boids(const mat4& view, const mat4 proj);
        void update_boids();
    };

}
