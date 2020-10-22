#version 330 core

// uniform data
// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
	vec3 position;
	vec3 normal;
	vec2 textureCoord;
} f_in;
in vec3 vertexColor;
in vec4 shadowCoord;


// framebuffer output
layout(location = 0) out vec4 colour;

uniform sampler2D shadowMap;
uniform vec3 lightdirection;
uniform mat4 MVP;

const vec3 lightCol = vec3(1.0,0.9,1.0);
const float lightPower = 0.2;
const vec3 ambient = vec3(0.2,0,0.5);

void main() {

	vec3 lightdir = normalize(lightdirection);
	float bias = max(0.01 * (1 - dot(normalize(f_in.normal), lightdir)), 0.01); // for shadow acne

	vec3 lightCol = vec3(1,0.9,1);
	vec3 matCol = vertexColor;
	float visibility = 1.0;


	// naive soft shadowing
	int softness = 5;
	vec2 texelSize = 1.0 / textureSize(shadowMap, (softness - 1));
	for(float x = -1; x <= 1; x += 1.0f/(softness/2+0.5)){
		for(float y = -1; y <= 1; y += 1.0f/(softness/2+0.5)){
			float pcfDepth = texture(shadowMap, shadowCoord.xy + vec2(x,y) * texelSize).r;
			if(shadowCoord.z - bias > pcfDepth){
				visibility -= 1.0f/ 9.0f / (softness*1.2);
			}
		}
	}

	if( texture( shadowMap, shadowCoord.xy ).r <  shadowCoord.z - bias){
		//visibility = 0.3;
	}
	if(shadowCoord.z > 1){ // double check this
		//visibility = 1;
	}

	//colour = vec4(visibility * matCol * lightCol, 1); // output just the shadows


	// cook torrence shading
	float diffuse = 0.9;
	float diffRatio = 0.1;
	float roughness = 0.9;
	vec3 eye = normalize(-f_in.position); // view??
	vec3 halfway = normalize(eye - normalize(lightdirection));

	float p = (2 / (pow(roughness,2)) - 2);

	float F = 0.04 + (1-0.04) * pow((1 - dot(eye,halfway)), 5);
	float D = 1 / (3.14159265358979 * roughness * roughness) * pow(dot(halfway , f_in.normal),p);

	float g1 = (2 * dot(halfway, f_in.normal) * dot(f_in.normal, eye)) / dot(eye, halfway);
	float g2 = (2 * dot(halfway, f_in.normal) * dot(f_in.normal, lightdir)) / dot(eye, halfway);
	float G = min (g1,g2);
	G = min(1, G);

	float spec = D * G * F;
	//spec /= (4 * dot(f_in.normal, lightdir) * dot(f_in.normal, eye));
	float div = (4 * dot(f_in.normal, lightdir) * dot(f_in.normal, eye));
	if(div < 0.001 && div >= 0 ){
		div = 0.001;
	}
	else if(div > -0.01 && div <= 0){
		div = -0.001;
	}
	spec = spec / div;

	float power = clamp(dot(f_in.normal, lightdir) * ((1 - diffRatio) * diffuse + diffRatio * spec),-10,10);
	vec3 r = matCol + lightCol * power * lightPower;
	r = vec3(r.x * visibility, r.y * visibility, r.z* ((1 - visibility)/2 + visibility) ); // keeps shadows bluer

	// output to the frambuffer
	colour = vec4(r, 1);
}





























//#version 330 core
//
//// uniform data
//uniform mat4 uProjectionMatrix;
//uniform mat4 uModelViewMatrix;
//uniform vec3 uColor;
//
//// viewspace data (this must match the output of the fragment shader)
//in VertexData {
//	vec3 position;
//	vec3 normal;
//	vec2 textureCoord;
//} f_in;
//
//// framebuffer output
//out vec4 fb_color;
//
//void main() {
//	// calculate lighting (hack)
//	vec3 eye = normalize(-f_in.position);
//	float light = abs(dot(normalize(f_in.normal), eye));
//	vec3 color = mix(uColor / 4, uColor, light);
//
//	// output to the frambuffer
//	fb_color = vec4(color, 1);
//}