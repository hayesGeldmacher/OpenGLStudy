#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 2) in vec3 normal;

out vec3 vNormal;
out vec3 FragPos;
out vec3 lightPos;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPosition;

void main(){

	vNormal = mat3(transpose(inverse(world))) * normal;  
	FragPos = vec3( world * vec4(pos, 1.0f));
	gl_Position = projection * view * world * vec4(pos.x,pos.y,pos.z,1.0f);
	lightPos = vec3(vec4(lightPosition, 1.0f));
	
}