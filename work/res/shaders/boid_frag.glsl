#version 330 core

// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

const int MAX_BONES = 4;
uniform mat4 uBones[MAX_BONES];

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
    vec4 boneIds;
    vec4 boneWeights;
} f_in;

// framebuffer output
out vec4 fb_color;

void main() {
	// calculate lighting (hack)
	vec3 eye = normalize(-f_in.position);
	float light = abs(dot(normalize(f_in.normal), eye));
	vec3 color = mix(uColor / 4, uColor, light);

	// output to the frambuffer
	fb_color = vec4(f_in.boneWeights.xyz, 1);
}