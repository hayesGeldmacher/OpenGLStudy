#version 330 core
layout(location = 0) out vec4 color;

in vec2 vTex;

uniform sampler2D textureColorBuffer;

void main(){
	color = texture(textureColorBuffer, vTex);
	//color = vec4(0.5f, 1.0f, 0.4f, 1.0f);
}