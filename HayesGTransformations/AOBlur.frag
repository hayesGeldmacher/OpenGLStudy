#version 330 core
layout (location = 0) out float fragColor;

in vec2 vTex;
uniform sampler2D AO;

void main(){

	//get size of texel based on stored AO texture size
	vec2 texelSize = 1.0 / vec2(textureSize(AO, 0));
	float result = 0.0;
	for(int x = -2; x < 2; x++){
		
		for(int y = -2; y < 2; y++){
			
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(AO, vTex + offset).r;
		}
	}
	
	fragColor = result / (4.0 * 4.0);
}