#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCord;

out vec3 vColor;
out vec3 vNormal;
out vec3 FragPos;
out vec3 lightPos;
out vec2 vTex;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPosition;

//matrix for shadow sampling
uniform mat4 matrixShadow;

//output for looking up shadow texture
out vec4 lightView_Position;


//this is the actual texture data that color is being sampled from
uniform sampler2D tex;


void main(){

	gl_Position = projection * view * world * vec4(pos.x,pos.y,pos.z,1.0f);

	FragPos = vec3( world * vec4(pos, 1.0f));

	vNormal = mat3(transpose(inverse(world))) * normal;  

	vTex = texCord;

	lightPos = vec3(vec4(lightPosition, 1.0f));

	//puts position in 0-1 range for texture lookups
	lightView_Position = matrixShadow * vec4(pos, 1.0f);
	
}
