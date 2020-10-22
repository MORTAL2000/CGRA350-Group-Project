#version 330 core

// uniform data
// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
//uniform vec3 uColor;

// mesh data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// model data (this must match the input of the vertex shader)
out VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} v_out;
out vec3 vertexColor;

uniform mat4 MVP; // proj * view * model
uniform mat4 DepthBiasMVP;

out vec4 shadowCoord;

void main() {
	v_out.position = (MVP * vec4(aPosition, 1)).xyz;
	v_out.normal = normalize((MVP * vec4(aNormal, 0)).xyz);
	v_out.textureCoord = aTexCoord;

	gl_Position = MVP * vec4(aPosition, 1);
	shadowCoord = DepthBiasMVP * vec4(aPosition, 1);


	if (aPosition.y >= 50) //snow
	{
		vertexColor = vec3(1, 1, 1);
	}
	if (aPosition.y < 50) //land
	{
		vertexColor = vec3(90.f / 255, 69.f / 255, 69.f / 255);
	}
	if (aPosition.y < 40) //grass
	{
		vertexColor = vec3(86.f / 255, 152.f / 255, 23.f / 255);
	}
	if (aPosition.y < 30) // sand
	{
		vertexColor = vec3(178.f / 255, 193.f / 255, 85.f / 255);
	}
	if (aPosition.y <= 20) //water
	{
		vertexColor = vec3(67.f / 255, 115.f / 255, 208.f / 255);
	}
}

