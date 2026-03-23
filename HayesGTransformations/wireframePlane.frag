#version 330 core

layout(location = 0) out vec4 color;

//per-vertex variables
in vec3 vNormal;
in vec3 FragPos;
in vec2 vTex;
in vec4 fragPosLightSpace;

void main(){

	

	color = vec4(1.0f, 0.0f, 1.0f, 1.0f);

	

}