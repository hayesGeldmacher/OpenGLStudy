#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 2) in vec3 normal;
layout(location = 1) in vec2 texCord;

out vec2 vTex;
out vec4 reflectionPosition;
out vec4 worldPos;

out vec3 vNormal;
out vec3 FragPos;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 reflectionMat;

void main(){

	
	mat4 reflectProjectWorld;

	vNormal = mat3(transpose(inverse(world))) * normal;  
	FragPos = vec3( world * vec4(pos, 1.0f));
	gl_Position = projection * view * world * vec4(pos.x,pos.y,pos.z,1.0f);
	worldPos = gl_Position;
	vTex = texCord;
	
	//create reflection world mat
	reflectProjectWorld = reflectionMat * projection;
	reflectProjectWorld = world * reflectProjectWorld;

	//calculate input position
	reflectionPosition = vec4(pos, 1.0f) * reflectProjectWorld;
	
}