#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCord;

out vec3 vColor;
out vec3 vNormal;
out vec3 fragPos;
out vec2 vTex;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

//matrix for shadow sampling
uniform mat4 lightMat;

//output for looking up shadow texture
out vec4 fragPosLightSpace;

void main(){

	fragPos = vec3(world * vec4(pos, 1.0f));
	vNormal = transpose(inverse(mat3(world))) * normal;
	vTex = texCord;
	fragPosLightSpace = lightMat * vec4(fragPos, 1.0f);
	gl_Position = projection * view * vec4(fragPos, 1.0f);
}