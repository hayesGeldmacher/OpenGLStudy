#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCord;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

//pass along fields
out vec3 tcsNormal;
out vec2 tcsTex;


void main(){

	vec3 newPos = vec3(pos.x, pos.y + 0.1f, pos.z);
	gl_Position = vec4(newPos, 1.0f);

	tcsNormal = normal;
	tcsTex = texCord;
}
