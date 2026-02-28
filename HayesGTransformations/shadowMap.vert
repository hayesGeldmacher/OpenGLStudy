#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 mvp;

//matrix for shadow sampling
uniform mat4 matrixShadow;

//output for looking up shadow texture
out vec4 lightView_Position;



void main(){

	gl_Position = mvp * vec4(pos, 1.0f);
	//puts position in 0-1 range for texture lookups
	lightView_Position = matrixShadow * vec4(pos, 1.0f);

}