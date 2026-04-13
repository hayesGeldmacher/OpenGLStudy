#version 330 core
//layout (location = 1) out vec3 outNormal;

out vec4 color;

in vec2 vTex;

uniform sampler2D AO;


void main(){

	//get the normal postion from gBuffer texture
	float occlusion = texture(AO, vTex).r;




	color = vec4(occlusion, occlusion, occlusion, 1.0f);


}