#version 330 core
layout(location = 0) out vec4 color;

in vec3 dir;

uniform samplerCube env;

void main(){
	 color = texture(env, dir);
	//cube has red color

	//always render something in red first to make sure it works!
	//color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}