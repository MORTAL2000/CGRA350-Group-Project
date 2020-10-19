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
    class boids_renderer {
        boids_model model_;
        vec3 target_position_;

        float m_deltaTime = 0.0f;
        float m_lastFrame = 0.0f;

    public:
        vector<boid> boids; // Boids array
        
        boids_renderer();
        ~boids_renderer();
        void update(const mat4& view, const mat4 proj);
        void render_boids(const mat4& view, const mat4 proj);
        void update_boids();
        void apply_boid_rules(boid& b);
    };

}
