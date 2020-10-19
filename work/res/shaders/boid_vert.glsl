#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

const int MAX_BONES = 4;
uniform mat4 uBones[MAX_BONES];

// mesh data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 aBoneIds;
layout(location = 4) in vec4 aBoneWeights;

// model data (this must match the input of the vertex shader)
out VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
    vec4 boneIds;
    vec4 boneWeights;
} v_out;

void main() {

    mat4 bone_transform = uBones[aBoneIds[0]] * aBoneWeights[0];
    bone_transform += uBones[aBoneIds[1]] * aBoneWeights[1];
    bone_transform += uBones[aBoneIds[2]] * aBoneWeights[2];
    bone_transform += uBones[aBoneIds[3]] * aBoneWeights[3];

    vec4 boned_position = bone_transform * vec4(aPosition, 1.0);

	// transform vertex data to viewspace
	v_out.position = vec3(uModelViewMatrix * boned_position);
	v_out.normal = normalize(vec3(uModelViewMatrix * (bone_transform * vec4(aNormal, 0))));
	v_out.textureCoord = aTexCoord;
    v_out.boneIds = aBoneIds;
    v_out.boneWeights = aBoneWeights;

	// set the screenspace position (needed for converting to fragment data)
	gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aPosition, 1.0);

/*
    v_out.normal = normalize(vec3(uModelViewMatrix * (bone_transform * vec4(aNormal, 0))));
    v_out.position = vec3(uModelViewMatrix * boned_position);
	v_out.textureCoord = aTexCoord;
    v_out.boneIds = aBoneIds;
    v_out.boneWeights = aBoneWeights;

    gl_Position = uModelViewMatrix * boned_position;*/
}