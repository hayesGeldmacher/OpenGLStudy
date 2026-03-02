#version 330 core
layout(location=0) out vec4 color;

in vec4 lightView_Position;

uniform sampler2D shadow;

void main(){
	
	color = vec4(0.0f, 1.0f, 1.0f, 1.0f);
}