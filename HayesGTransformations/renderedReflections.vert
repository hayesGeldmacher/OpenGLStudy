#version 330 core
layout(location = 0) in vec3 pos;


out vec4 worldPos;

out vec3 FragPos;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;


void main(){

	//set MVP for position	
	gl_Position = projection * view * world * vec4(pos.x,pos.y,pos.z,1.0f);
	worldPos = gl_Position;

	//get world fragment position
	FragPos = vec3( world * vec4(pos, 1.0f));


}