#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 3) in vec2 texCord;
out vec2 vTex;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void main(){

	gl_Position = projection * view * world * vec4(pos.x,pos.y,pos.z,5.0f);
	gl_Position = vec4(pos.x, pos.y, 0.0f, 1.0f);
	vTex = texCord;
}