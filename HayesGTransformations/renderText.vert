#version 330 core

//similar to shader.vert for the teapot, but with fewer inputs and outputs
//becuase we aren't worried about ligting, normals, et cet - just rendering to screen

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCord;
layout(location = 2) in vec2 normal;

out vec2 vTex;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void main(){

//for testing dont even worry about projection, just do standard rendering
	 gl_Position =  projection * view * world * vec4(pos.x,pos.y,pos.z,1.0f);
	 vTex = texCord;

}
