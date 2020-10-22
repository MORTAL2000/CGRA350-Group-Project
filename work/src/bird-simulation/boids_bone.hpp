#pragma once

// assimp
#include <assimp/scene.h>

// std
#include <string>

using namespace std;

namespace blukzen
{
    class bone
    {
    public:
        std::string name;
        unsigned int id;
        aiNode* node;
        aiNodeAnim* anim_node;
        bone* parent;
        aiMatrix4x4 offset_matrix;

        bone(): node(nullptr), anim_node(nullptr), parent(nullptr)
        {
            name = "";
            id = -1;
        }

        bone(unsigned int in_id, string in_name, aiMatrix4x4 in_offset);

        aiMatrix4x4 get_parent_transforms();
    };
}
