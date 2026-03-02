#version 330 core

layout (location = 0) in vec3 pos;

//matrix for object-world space
uniform mat4 world;

//matrix for light space
uniform mat4 lightMat;


void main(){

	gl_Position = lightMat * world * vec4(pos.x,pos.y,pos.z,1.0f);

}