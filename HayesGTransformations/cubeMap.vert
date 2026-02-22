#version 330 core

//similar to shader.vert for the teapot, but with fewer inputs and outputs
//becuase we aren't worried about ligting, normals, et cet - just rendering to screen

layout(location = 0) in vec3 pos;

out vec3 dir;

uniform mat4 view;
uniform mat4 projection;

void main(){

	 dir = pos;
	 gl_Position =  projection * view * vec4(pos.x,pos.y,pos.z,1.0f);

}