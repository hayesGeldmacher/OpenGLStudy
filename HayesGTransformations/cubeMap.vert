#version 330 core

//similar to shader.vert for the teapot, but with fewer inputs and outputs
//becuase we aren't worried about ligting, normals, et cet - just rendering to screen

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 direction;

out vec3 dir;

uniform mat4 world;

void main(){

//for testing dont even worry about projection, just do standard rendering
	 gl_Position =  world * vec4(pos.x,pos.y,pos.z,1.0f);
	 dir = direction;

}