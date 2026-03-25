#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCord;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;



void main(){

	gl_Position = vec4(pos, 1.0f);
}