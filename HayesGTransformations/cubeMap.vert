#version 330 core

//similar to shader.vert for the teapot, but with fewer inputs and outputs
//becuase we aren't worried about ligting, normals, et cet - just rendering to screen

layout(location = 0) in vec3 pos;

out vec3 dir;

uniform mat4 view;
uniform mat4 projection;

void main(){

	 dir = pos;

	 //sets the z component to always be 1.0
	 vec4 position = projection * view * vec4(pos.x,pos.y,pos.z,1.0f);
	 gl_Position = vec4(position.x, position.y, position.w, position.w);

}